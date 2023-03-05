#pragma once

#include <vector>

#include "agent.hpp"

using std::vector;

class Server;

struct AgentManager {
    vector<Agent*> agents;
    Server* server;
    AgentManager(Server* server) : server(server){};
    ~AgentManager();

    void init(uint16_t num_agents);
    bool act(vector<Agent::Action>& actions, uint8_t steps);
    // protocol parse
    static bool parse(AgentManager* manager, string_view buf);
};
