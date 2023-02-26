#include <fstream>

#include "../entity/handle.hpp"
#include "server.hpp"
#include "writer.hpp"

bool Server::open(ServerConfig config) {
    if (isOpen()) return false;

    std::mutex m;
    std::condition_variable cv;

    for (uint16_t i = 0; i < config.cytos.threads; i++) {
        auto th = new thread([&, i] {
            auto app =
                uWS::App()
                    .get("/",
                         [](auto res, uWS::HttpRequest* req) {
                             res->end("Hello Cytos");
                         })
                    .listen(
                        "0.0.0.0", config.cytos.port,
                        [this, config, i, &m, &cv](us_listen_socket_t* sock) {
                            if (sock) {
                                std::scoped_lock<std::mutex> lk(m);
                                sockets.push_back(sock);
                            } else {
                                logger::error(
                                    "Server (Cytos) failed to open on port "
                                    "%i\n",
                                    config.cytos.port);
                                std::exit(1);
                            }

                            cv.notify_one();
                        });

            app.run();
        });
        threads.push_back(th);
    }

    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&] {
            return sockets.size() >= config.cytos.threads;
        });  // >= ???
    }

    logger::debug("Game config: cytos(:%i, %ut)\n", config.cytos.port,
                  config.cytos.threads);

    return true;
};