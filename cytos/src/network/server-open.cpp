#include <fstream>

#include "../agent/manager.hpp"
#include "../entity/handle.hpp"
#include "server.hpp"
#include "writer.hpp"

struct read_ctx {
    // ~read_ctx() { logger::debug("read_ctx::~read_ctx\n"); }
    bool aborted = false;
    uint64_t start;
    std::string data;
};

bool Server::open(ServerConfig config) {
    if (isOpen()) return false;

    std::mutex m;
    std::condition_variable cv;

    for (uint16_t i = 0; i < config.cytos.threads; i++) {
        auto th = new thread([&, i] {
            AgentManager manager(this);

            auto app =
                uWS::App()
                    .get("/",
                         [](auto res, uWS::HttpRequest* req) {
                             res->end("Hello Cytos");
                         })
                    .post("/init",
                          [&](auto res, uWS::HttpRequest* req) {
                              auto agents_sv = req->getQuery("agents");
                              auto agents_str =
                                  string(agents_sv.data(), agents_sv.size());
                              int agents = 0;

                              if (!sscanf(agents_str.c_str(), "%d", &agents)) {
                                  logger::debug("Invalid agent param: %s\n",
                                                agents_str.c_str());
                                  res->writeStatus("400 Bad Request");
                                  res->end();
                                  return;
                              }

                              if (agents <= 0 || agents > 16 || agents & 1) {
                                  logger::debug("Invalid number of %i agents\n",
                                                agents);
                                  res->writeStatus("400 Bad Request");
                                  res->end();
                                  return;
                              }

                              logger::debug("Initializing %i agents\n", agents);
                              manager.init(agents);
                              res->end();
                          })
                    .post(
                        "/act",
                        [&](auto res, uWS::HttpRequest* req) {
                            auto ctx = std::make_shared<read_ctx>();
                            ctx->start = uv_hrtime();

                            res->onData([&, res, ctx](std::string_view chunk,
                                                      bool isFin) mutable {
                                if (chunk.length()) {
                                    ctx->data.append(chunk.begin(),
                                                     chunk.end());

                                    logger::debug("Received %lu bytes\n",
                                                  chunk.size());

                                    uint64_t end;
                                    auto ms = time_func(ctx->start, end);
                                    logger::debug("data receive: %.2fms\n", ms);
                                }

                                if (!isFin || ctx->aborted) return;

                                bool success =
                                    AgentManager::parse(&manager, ctx->data);

                                {
                                    uint64_t end;
                                    auto ms = time_func(ctx->start, end);
                                    logger::debug("total time: %.2fms\n", ms);
                                }

                                if (success) {
                                    res->end();
                                } else {
                                    logger::debug("Parse failed: %s\n",
                                                  ctx->data.c_str());
                                    res->writeStatus("400 Bad Request")->end();
                                }
                            });

                            res->onAborted([ctx]() { ctx->aborted = true; });
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