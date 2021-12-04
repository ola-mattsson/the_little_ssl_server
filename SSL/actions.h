#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <openssl/ssl.h>
#include <cstring>

namespace actions {

    static void create_something(SSL *ssl, char *inp __attribute__((unused))) {

        boost::uuids::random_generator generator;
        std::string uuid_str = boost::uuids::to_string(generator());

        std::stringstream ss;
        ss << R"({"id":")" << uuid_str << R"("})";
        std::string json = ss.str();

        std::stringstream ss1;
        ss1 <<
            "HTTP/1.1 201 Created\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " << json.size() << "\r\n"
            "\r\n" << json << "\r\n";

        std::string response = ss1.str();
        SSL_write(ssl, response.c_str(), static_cast<int>(response.size()));
    }

    static void update_something(SSL *ssl, char *inp __attribute__((unused))) {
        static char response[1024];

        snprintf(
            response,
            1024,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "\r\n"
            "{}"
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
    }

    static void delete_something(SSL *ssl, char *inp __attribute__((unused))) {
        static char response[1024];

        snprintf(
            response,
            1024,
            "HTTP/1.1 204 No Content\r\n"
            "\r\n"
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
    }
}
