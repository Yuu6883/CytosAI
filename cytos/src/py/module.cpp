#include <Python.h>
#include <numpy/arrayobject.h>

static PyObject* my_module_create_array(PyObject* self, PyObject* args) {
    npy_intp dims[2] = {2, 3};
    PyArrayObject* array =
        (PyArrayObject*)PyArray_EMPTY(2, dims, NPY_DOUBLE, 0);
    double* data = (double*)PyArray_DATA(array);
    data[0] = 1.0;
    data[1] = 2.0;
    data[2] = 3.0;
    data[3] = 4.0;
    data[4] = 5.0;
    data[5] = 6.0;

    return (PyObject*)array;
}

static PyMethodDef module_methods[] = {
    {"create_array", my_module_create_array, METH_NOARGS,
     "Create a 2x3 NumPy array"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "my_module",
                                        "Example module", -1, module_methods};

PyMODINIT_FUNC PyInit_my_module(void) {
    import_array();
    return PyModule_Create(&module_def);
}