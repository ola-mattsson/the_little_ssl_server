#pragma once

#include <gethostuuid.h>
#include <openssl/ssl.h>
#include <uuid/uuid.h>
#include <cstring>

namespace actions {
    static void create_something(SSL *ssl, char *inp __attribute__((unused))) {
        char uuid_str[37];
        uuid_t uuid;
        static char response[1024];
        static char json[1024];

        uuid_generate(uuid);
        uuid_unparse(uuid, uuid_str);

        snprintf(json, 1024, R"({"id":"%s"})", uuid_str);

        snprintf(
            response,
            1024,
            "HTTP/1.1 201 Created\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %ld\r\n"
            "\r\n%s",
            strlen(json),
            json
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
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
};
