#include <nlohmann/json.hpp>

#include "../agent/manager.hpp"
#include "reader.hpp"

using json = nlohmann::json;

bool AgentManager::parse(AgentManager* manager, string_view buf) {
    uint64_t start = uv_hrtime();
    auto obj = json::parse(buf, nullptr, false);
    if (obj.is_discarded() || !obj.is_object()) return false;

    auto steps = obj["steps"];
    auto action_arr = obj["actions"];
    if (!action_arr.is_array() || !steps.is_number_unsigned()) return false;
    if (steps > 16) return false;

    if (action_arr.size() != manager->agents.size()) {
        return false;
    }

    vector<Agent::Action> actions;

    for (auto i = 0; i < action_arr.size(); i++) {
        auto action_obj = action_arr[i];
        if (!action_obj.is_object()) return false;

        auto split_obj = action_obj["splits"];
        auto eject_obj = action_obj["ejects"];
        auto lockc_obj = action_obj["lock_cursor"];
        auto cursor_x_obj = action_obj["cursor_x"];
        auto cursor_y_obj = action_obj["cursor_y"];

        if (!split_obj.is_number_unsigned() ||
            !eject_obj.is_number_unsigned() || !lockc_obj.is_boolean() ||
            !cursor_x_obj.is_number() || !cursor_y_obj.is_number())
            return false;

        Agent::Action action;
        action.splits = split_obj;
        action.ejects = eject_obj;
        action.lock_cursor = lockc_obj;
        action.cursor_x = cursor_x_obj;
        action.cursor_y = cursor_y_obj;

        if (std::isnan(action.cursor_x) || std::isnan(action.cursor_y))
            return false;

        action.splits = std::min(action.splits, uint16_t(5));
        action.ejects = std::min(action.ejects, uint16_t(5));

        actions.push_back(action);
    }

    uint64_t end;
    auto ms = time_func(start, end);
    logger::debug("parse time:   %.2fms\n", ms);

    manager->act(actions, uint8_t(steps));
    return true;
}