#include "manager.hpp"

#include "../entity/control.hpp"
#include "../network/server.hpp"
#include "../physics/engine.hpp"

Agent::~Agent() {
    // logger::debug("dealloc %s: 0x%p\n", __gid.c_str(), this);
}

AgentManager::~AgentManager() {
    for (auto& agent : agents) {
        auto buf = (void*)agent->state.data();
        if (buf) {
            // logger::debug("dealloc agent state buffer: 0x%p\n", buf);
            free(buf);
        }
        delete agent;
    }
    agents.clear();
}

void AgentManager::init(uint16_t num_agents) {
    if (!server->engines.size()) {
        logger::warn("No engine found\n");
        return;
    }

    auto engine = server->engines.begin()->second;

    for (auto& agent : agents) {
        agent->remove();
        engine->freeHandle(agent);
    }
    agents.resize(num_agents);

    for (uint16_t i = 0; i < num_agents; i++) {
        auto agent = agents[i] = new Agent(server, i);
        agent->setEngine(engine);
        agent->join();

        // pair up agent as team
        if (i & 1) {
            agent->dual = agents[i - 1];
            agents[i - 1]->dual = agent;
        }
    }
}

bool AgentManager::act(vector<Agent::Action>& actions, uint8_t steps) {
    if (actions.size() != agents.size()) return false;

    // Set actions
    for (auto i = 0; i < actions.size(); i++) {
        auto& action = actions[i];
        auto& ctrl = agents[i]->control;

        if (!ctrl->alive) {
            // logger::info("spawning %s\n", agents[i]->__gid.c_str());
            ctrl->requestSpawn();
            continue;
        }

        ctrl->splits = action.splits;
        ctrl->ejects = action.ejects;

        if (!action.lock_cursor) {
            ctrl->__mouseX = action.cursor_x;
            ctrl->__mouseY = action.cursor_y;
        }
    }

    uint64_t t0 = hrtime(), t1, t2;

    // Timesteps
    for (auto _ = 0; _ < steps; _++) server->tick();
    auto m1 = time_func(t0, t1);

    // Calculate reward
    for (auto& agent : agents) {
        server->threadPool.enqueue([&] {
            auto buf = agent->observe();

            if (agent->state.data()) {
                free((void*)agent->state.data());
            }

            auto copy = (char*)malloc(buf.size());
            memcpy(copy, buf.data(), buf.size());
            agent->state = string_view(copy, buf.size());
            // logger::debug("agent %s output: <buf %i>\n",
            // agent->gatewayID().data(), buf.size());
        });
    }
    server->threadPool.sync();
    auto m2 = time_func(t1, t2);

    // logger::debug("tick time:    %.2fms\n", m1);
    // logger::debug("observe time: %.2fms\n", m2);

    return true;
}