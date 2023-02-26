#pragma once

#include <stdio.h>

#include <csignal>
#include <string_view>

#define NOMINMAX
#include <uv.h>

#include "../network/server.hpp"
#include "../physics/engine.hpp"
#include "logger.hpp"

typedef Server server_t;

static inline std::unique_ptr<char[]> std_pool(new char[65536]);

class readline {
    static inline bool closing;
    static inline uv_tty_t tty;
    static inline uv_pipe_t stdin_pipe;
    static inline uv_signal_t signal_handle;
    static inline bool isTTY;

    static void on_sigint(uv_signal_t* handle, int signal) {
        if (closing) return;
        closing = true;
        logger::debug("SIGINT received\n");
        if (isTTY) {
            uv_read_stop((uv_stream_t*)&tty);
            logger::debug("CLI reader closed\n");
        } else {
            uv_read_stop((uv_stream_t*)&stdin_pipe);
            logger::debug("STDIN reader closed\n");
        }
        static_cast<server_t*>(handle->data)->close();
        uv_signal_stop(handle);
    }

    static void alloc_buffer(uv_handle_t* handle, size_t suggested_size,
                             uv_buf_t* buf) {
        *buf = uv_buf_init(std_pool.get(), suggested_size);
    }

    static void read_stdin(uv_stream_t* stream, ssize_t nread,
                           const uv_buf_t* buf) {
        if (nread < 0) {
            if (nread == UV_EOF) {
                // end of file
                uv_read_stop(stream);
                logger::info("EOF received\n");
                static_cast<server_t*>(stream->data)->close();
                uv_signal_stop(&signal_handle);
                logger::verbose("CLI reader closed\n");
            }
        } else if (nread > 0) {
            string_view view =
                string_view(buf->base, nread).substr(0, nread - 1);

            if (view.ends_with("\r")) view.remove_suffix(1);

            if (view == "exit") {
                uv_read_stop(stream);
                logger::info("Exit command received\n");
                static_cast<server_t*>(stream->data)->close();
                uv_signal_stop(&signal_handle);
                logger::verbose("CLI reader closed\n");
            } else {
                // implement engine switch in cli?
                auto server = static_cast<server_t*>(stream->data);
                server->onCommand(server->engines.begin()->second, view);
            }
        }
    }

   public:
    void static cli(bool autoStart = true) {
        closing = false;

        const char* g = std::getenv("GAME_THREADS");
        uint16_t gt = g ? std::atoi(g) : 1;

        const char* mode = std::getenv("MODE");

        server_t s(gt, mode ? string(mode) : "ffa");

        ServerConfig config;

        const char* port_str = std::getenv("CYTOS_PORT");
        config.cytos.port = port_str ? std::atoi(port_str) : 3000;

        const char* th_str = std::getenv("CYTOS_THREADS");
        config.cytos.threads = th_str ? std::atoi(th_str) : 1;

        const char* gw_str = std::getenv("GATEWAY_PORT");

        int gw_port = gw_str ? std::atoi(gw_str) : 4000;

        if (s.open(config)) {
            for (auto [_, e] : s.engines) e->start();
        }

        uv_loop_t* loop = uv_default_loop();

        uv_signal_init(loop, &signal_handle);
        signal_handle.data = &s;
        uv_signal_start(&signal_handle, on_sigint, SIGINT);

        isTTY = uv_guess_handle(0) == UV_TTY;

        if (isTTY) {
            uv_tty_init(loop, &tty, 0, 1);
            uv_tty_set_mode(&tty, UV_TTY_MODE_NORMAL);
            tty.data = &s;

            logger::debug("CLI reader open\n");
            uv_read_start((uv_stream_t*)&tty, alloc_buffer, read_stdin);
        } else {
            uv_pipe_init(loop, &stdin_pipe, 0);
            uv_pipe_open(&stdin_pipe, 0);
            stdin_pipe.data = &s;

            logger::debug("STDIN reader open\n");
            uv_read_start((uv_stream_t*)&stdin_pipe, alloc_buffer, read_stdin);
        }

        if (autoStart) {
            uv_run(loop, UV_RUN_DEFAULT);
        }

        uv_walk(
            loop,
            [](uv_handle_t* handle, void* arg) { uv_close(handle, nullptr); },
            nullptr);
        uv_run(loop, UV_RUN_DEFAULT);
        uv_loop_close(loop);
    }
};