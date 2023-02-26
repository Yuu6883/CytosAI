#pragma once

#include "handle.hpp"
#include "control.hpp"

struct DualHandle : public Handle {
    DualHandle(Handle* dual) : Handle(dual->server) {
        this->dual = dual;
    }

    bool isAlive() { return dual ? dual->isAlive() : Handle::isAlive(); };
    bool spectatable() { return false; }
    cell_cord_prec getScore() { 
        return dual ? ((control ? control->score : 0) + (dual->control ? dual->control->score : 0)) : 
            Handle::getScore(); }
    bool cleanMe() override { return !dual; }
};