#pragma once

#include <vector>

#include "agent.hpp"

using std::vector;

class Server;

class AgentManager {
    vector<Agent*> agents;

   public:
    Server* server;
    AgentManager(Server* server) : server(server){};

    void init(uint16_t num_agents);
    bool act(vector<Agent::Action>& actions, uint8_t steps);
    size_t agent_num() { return agents.size(); };
    // protocol parse
    static bool parse(AgentManager* manager, string_view buf);
};
