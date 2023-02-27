#pragma once

#include "../entity/handle.hpp"

// TODO: implement
struct Agent : Handle {
    struct Action {
        uint16_t splits;
        uint16_t ejects;
        bool lock_cursor;
        float cursor_x;
        float cursor_y;
    };

    Agent(Server* server, uint16_t id)
        : Handle(server, "agent#" + std::to_string(id)) {}

    string_view observe();
};