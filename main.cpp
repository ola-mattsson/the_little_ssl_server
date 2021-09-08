
#include "socket_descriptor.h"
#include <cstdlib>
#include <iostream>
#include <future>
#include <array>
#include <ssl_server.h>

#define CRAP_THREADS 0
int main(int argc, char **argv) {

    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " port cert key\n";
        exit(EXIT_FAILURE);
    }

    SSL_server server(argv[2], argv[3]);

    int port = static_cast<int>(std::strtol(argv[1], nullptr, 10));
    if (port < 3000) {
        std::cout << "first argument is port, needs to be an integer and >= 3000\n";
        exit(EXIT_FAILURE);
    }

    auto server_socket = socket_fd::create_socket(port);

#if CRAP_THREADS
    auto thread_func = [&server](const socket_fd& s){
        return server.handle_request(s);
    };
    const int max_threads = 100;
    std::array<std::thread, max_threads> threads;
    int current_thread = 0;
#endif
    for (uint64_t i(0); i < std::numeric_limits<uint64_t>::max(); ++i) {

        socket_fd client_socket;
        client_socket.wait_request(server_socket);

#if CRAP_THREADS
        // this sux but unless the files are a bit larger, it makes no difference
        // so still, TODO thread this shit properly
        threads[current_thread++] = std::thread(thread_func, std::move(client_socket));
        if (current_thread == max_threads) {
            for (auto && t : threads) {
                t.join();
            }
            current_thread = 0;
        }
#else
        int rc = server.handle_request(client_socket);
        if (rc < 0) {
            break;
        } else if (rc > 0) {
            continue;
        }
#endif
    }

    return 0;
}
