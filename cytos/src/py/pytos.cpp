#include <Python.h>
#include <numpy/arrayobject.h>

#include "../agent/manager.hpp"
#include "../network/server.hpp"

#define CAP_NAME "pytos"

static PyObject* pytos_create(PyObject* self, PyObject* args) {
    const char* mode;

    int agents = -1;
    int t = -1;

    if (!PyArg_ParseTuple(args, "sii", &mode, &agents, &t)) {
        return NULL;
    }

    if (agents <= 0 || agents > 16) {
        PyErr_SetString(PyExc_AssertionError,
                        "Agent number range [1, 16] not satisfied");
        return NULL;
    }

    uint16_t threads =
        std::min(t > 0 ? uint32_t(t) : std::thread::hardware_concurrency(), 8u);

    auto pytos = new Server(threads, string(mode));
    auto manager = new AgentManager(pytos);
    manager->init(agents);

    auto capsule = PyCapsule_New(manager, CAP_NAME, [](PyObject* obj) {
        auto m =
            static_cast<AgentManager*>(PyCapsule_GetPointer(obj, CAP_NAME));
        auto server = m->server;
        // delete manager first -> delete agents (which has access to engine)
        delete m;
        // then delete server -> delete engines
        delete server;
    });

    if (!capsule) {
        delete pytos;
        return NULL;
    }

    return capsule;
}

static PyObject* pytos_act(PyObject* self, PyObject* args) {
    PyObject* capsule;
    PyObject* actions_obj;
    int steps = -1;

    if (!PyArg_ParseTuple(args, "OOi", &capsule, &actions_obj, &steps)) {
        return NULL;
    }

    if (steps < 0 || steps > 25) {
        PyErr_SetString(PyExc_AssertionError,
                        "Steps range [1, 25] not satisfied");
        return NULL;
    }

    auto manager =
        static_cast<AgentManager*>(PyCapsule_GetPointer(capsule, CAP_NAME));
    if (!manager) Py_RETURN_FALSE;

    if (!PySequence_Check(actions_obj)) {
        PyErr_SetString(PyExc_TypeError, "Expected a sequence object");
        return NULL;
    }

    PyObject* action_arr =
        PySequence_Fast(actions_obj, "Expected a sequence object");
    if (!action_arr) return NULL;

    Py_ssize_t array_length = PySequence_Fast_GET_SIZE(action_arr);

    if (!array_length) {
        PyErr_SetString(PyExc_AssertionError,
                        "Action array length must not be 0");
        return NULL;
    }

    if (array_length != manager->agents.size()) {
        PyErr_SetString(PyExc_AssertionError,
                        "Action array length must match internal agent number");
        return NULL;
    }

    vector<Agent::Action> actions(array_length);

    for (Py_ssize_t i = 0; i < array_length; ++i) {
        PyObject* item = PySequence_Fast_GET_ITEM(action_arr, i);

        if (!PyDict_Check(item)) {
            PyErr_SetString(PyExc_TypeError,
                            "Expected a dictionary object in action array");
            break;
        }

        // do something with the dictionary object
        auto split_obj = PyDict_GetItemString(item, "splits");
        auto eject_obj = PyDict_GetItemString(item, "ejects");
        auto lockc_obj = PyDict_GetItemString(item, "lock_cursor");
        auto cursor_x_obj = PyDict_GetItemString(item, "cursor_x");
        auto cursor_y_obj = PyDict_GetItemString(item, "cursor_y");

        if (!split_obj || !eject_obj || !lockc_obj || !cursor_x_obj ||
            !cursor_y_obj || !PyLong_Check(split_obj) ||
            !PyLong_Check(eject_obj) || !PyBool_Check(lockc_obj) ||
            !PyNumber_Check(cursor_x_obj) || !PyNumber_Check(cursor_y_obj)) {
            PyErr_SetString(PyExc_AssertionError, "Unexpected action object");
            break;
        }

        auto& action = actions[i];
        action.splits = PyLong_AsLong(split_obj);
        action.ejects = PyLong_AsLong(eject_obj);
        action.lock_cursor = PyObject_IsTrue(lockc_obj);
        action.cursor_x = PyFloat_AsDouble(cursor_x_obj);
        action.cursor_y = PyFloat_AsDouble(cursor_y_obj);

        action.splits = std::min(action.splits, uint16_t(5));
        action.ejects = std::min(action.ejects, uint16_t(5));
    }

    bool success = false;
    if (!PyErr_Occurred()) {
        success = manager->act(actions, steps);
    }

    if (!success) Py_RETURN_NONE;

    auto list = PyList_New(array_length);

    for (Py_ssize_t i = 0; i < array_length; ++i) {
        auto agent = manager->agents[i];

        if (agent->state.size() != Agent::DIM * Agent::DIM * 4) {
            PyErr_SetString(PyExc_AssertionError,
                            "Unexpected state buffer size");
            break;
        }

        auto info = PyDict_New();

        PyDict_SetItemString(info, "reward", PyFloat_FromDouble(0.0));

        auto data = (void*)agent->state.data();
        agent->state = string_view(nullptr, 0);

        npy_intp dims[] = {Agent::DIM, Agent::DIM, 3};
        npy_intp strides[] = {4 * Agent::DIM, 4, 1};

        auto pyArray = PyArray_NewFromDescr(&PyArray_Type,
                                            PyArray_DescrFromType(NPY_UINT8), 3,
                                            dims, strides, data, 0, NULL);

        PyDict_SetItemString(info, "state", pyArray);

        PyObject* capsule =
            PyCapsule_New(data, "state_buffer", [](PyObject* obj) {
                void* data = reinterpret_cast<int*>(
                    PyCapsule_GetPointer(obj, "state_buffer"));
                free(data);
            });

        PyArray_SetBaseObject((PyArrayObject*)pyArray, capsule);

        PyList_SetItem(list, i, info);

        Py_DECREF(pyArray);

        // logger::debug("dict refcount: %i\n", PyArray_REFCOUNT(info));
        // logger::debug("arr  refcount: %i\n", PyArray_REFCOUNT(pyArray));
        PyObject_GC_Track(info);
    }

    if (PyErr_Occurred()) return NULL;
    return list;
}

static PyMethodDef module_methods[] = {
    {"create", pytos_create, METH_VARARGS, "Create a Pytos object"},
    {"act", pytos_act, METH_VARARGS, "Pass actions to a Pytos object"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "pytos",
                                        "Example module", -1, module_methods};

PyMODINIT_FUNC PyInit_pytos(void) {
    import_array();
    return PyModule_Create(&module_def);
}