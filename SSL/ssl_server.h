#pragma once
#include "ssl_context.h"
#include "../socket_descriptor.h"

std::string create_response_html(std::string_view file_name, std::string_view accept_encoding);

class SSL_server {
    SSL_context context;
public:
    SSL_server(std::string_view cert, std::string_view key) {
        if (!context.configure(&cert[0], &key[0])) {
            exit(EXIT_FAILURE);
        }
    }

    static bool serve(const SSL_context::ssl_ptr& ssl);

    int handle_request(const socket_fd &socket) {
        auto ssl = context.get_SSL(socket);

        if (SSL_accept(ssl.get()) <= 0) {
            ERR_print_errors_fp(stderr);
            return 1;
        }

        if (!serve(ssl)) {
            // quit
            return -1;
        }

        return 0;
    }
};
