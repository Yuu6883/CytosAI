#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::atomic;
using std::list;
using std::mutex;
using std::thread;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using namespace std::chrono;

#include "../misc/pool.hpp"

using std::string;
using std::string_view;
using std::vector;

struct Engine;
struct VSEngine;

struct us_listen_socket_t;

struct PortConfig {
    int port;
    uint16_t threads;
};

struct ServerConfig {
    PortConfig cytos;
};

struct Handle;
struct Engine;
class readline;

class Server {
    friend Engine;
    friend readline;

    vector<thread*> threads;
    vector<us_listen_socket_t*> sockets;
    atomic<uint32_t> connections;

    mutex m;
    string mode;

    int64_t timestamp =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();

   public:
    unordered_map<uint16_t, Engine*> engines;

    ThreadPool threadPool;

    Server(uint16_t t, string mode);
    ~Server();

    bool isOpen() { return !sockets.empty(); }
    bool open(ServerConfig config);
    bool close();

    void tick();

    void onCommand(Engine* engine, string_view cmd);
    void gc();
    void log(Engine* engine);

    void postTick();

    uint32_t getConnections() { return connections.load(); };
    int64_t getTimestamp() { return timestamp; }

    void addEngine(Engine* engine);

    template <typename SyncCallback>
    void sync(const SyncCallback& cb) {
        bool l = m.try_lock();
        cb();
        if (l) m.unlock();
    }
};
