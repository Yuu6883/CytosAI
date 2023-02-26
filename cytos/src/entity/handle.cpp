#define _USE_MATH_DEFINES
#include "handle.hpp"

#include <math.h>

#include <cmath>
#include <mutex>

#include "../physics/engine.hpp"
#include "control.hpp"
using std::unique_lock;

bool Handle::isAlive() { return control && control->alive; };
cell_cord_prec Handle::getScore() { return control ? control->score : 0.f; };
uint32_t Handle::getKills() { return control ? control->kills : 0; };

void Handle::join() {
    if (engine) engine->addHandle(this);
}

void Handle::remove() {
    if (engine) engine->removeHandle(this);
}

void Handle::setEngine(Engine* engine) {
    if (this->engine) remove();
    this->engine = engine;
}

void Handle::onTick() {
    viewArea = control ? control->viewport.toAABB().getArea() : 0.f;
}

Point Handle::position() {
    if (control)
        return {control->viewport.x, control->viewport.y};
    else
        return {0, 0};
}

bool Handle::canEatPerk() {
    if (isBot()) return false;
    if (!engine->hidePerks) return true;
    if (!engine || !wasAlive) return false;
    return getScore() > engine->minPerkSize();
}