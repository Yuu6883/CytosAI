
#include "server.hpp"

#include <fstream>

#include "../entity/handle.hpp"
#include "../misc/logger.hpp"
#include "../modes/options.hpp"
#include "writer.hpp"

// Headers
#include "../extensions/rockslide/rock-engine.hpp"
#include "../physics/engine.hpp"

// Implementations headers
#include "../extensions/rockslide/rock-engine-impl.hpp"
#include "../physics/engine-impl.hpp"

constexpr OPT make_rock_opt() {
    OPT temp = instant_opt;
    temp.EJECT_MAX_AGE = 1000;
    return temp;
};

constexpr OPT rock_opt = make_rock_opt();

typedef TemplateEngine<ffa_opt> FFAEngine;
typedef TemplateEngine<instant_opt> InstantEngine;
typedef TemplateEngine<mega_opt> MegaEngine;
typedef TemplateEngine<omega_opt> OmegaEngine;
typedef TemplateEngine<omega_fat_opt> OmegaFatEngine;
typedef TemplateEngine<sf_opt> SfEngine;
typedef TemplateEngine<ultra_opt> UltraEngine;
typedef RockEngine<rock_opt> RockslideEngine;

Server::Server(uint16_t t, string mode)
    : connections(0), threadPool(t), mode(mode) {
    constexpr auto engineCount = std::max(1, ENGINES);

    for (int i = 0; i < engineCount; i++) {
        if (mode == "ffa") {
            addEngine(new FFAEngine(this));
        } else if (mode == "insta") {
            addEngine(new InstantEngine(this));
        } else if (mode == "mega") {
            addEngine(new MegaEngine(this));
        } else if (mode == "omega") {
            addEngine(new OmegaEngine(this));
        } else if (mode == "sf") {
            addEngine(new SfEngine(this));
        } else if (mode == "ultra") {
            addEngine(new UltraEngine(this));
        } else if (mode == "rock") {
            addEngine(new RockslideEngine(this));
        } else if (mode == "omega-fat") {
            addEngine(new OmegaFatEngine(this));
        } else if (mode == "custom") {
            addEngine(new Engine(this));
        } else {
            mode = "none";
            logger::warn("Game mode: default to none\n");
            addEngine(new Engine(this));
        }
    }

    logger::info("Initialized %u \"%s\" engine(s) with %u threads\n",
                 engineCount, mode.c_str(), t);
}

Server::~Server() {
    close();
    for (auto [_, e] : engines) delete e;
    engines.clear();
}

constexpr uint32_t PHYSICS_TPS = 25;
constexpr uint32_t TICK_MS = 1000 / PHYSICS_TPS;

void Server::tick() {
    for (auto [_, engine] : engines) {
        engine->__now += TICK_MS * MS_TO_NANO;
        // MILLISECONDS
        engine->tick(TICK_MS);
    }

    postTick();
}

void Server::postTick() {}

void Server::log(Engine* engine) {}

void Server::gc() {
    for (auto [_, engine] : engines) engine->gc();
}

void Server::onCommand(Engine* engine, string_view cmd) {
    if (!engine) return;

    if (cmd == "dt")
        log(engine);
    else if (cmd == "gc")
        gc();
    else
        engine->onExtCommand(cmd);
}

void Server::addEngine(Engine* engine) {
    uint16_t eid = 0;
    while (engines.find(eid) != engines.cend()) eid++;
    engine->id = eid;
    engines.insert({eid, engine});
}
