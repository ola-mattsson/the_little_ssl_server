#pragma once
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <mutex>
#include <iostream>
#include "../socket_descriptor.h"

class SSL_context {
    using ssl_context_ptr = std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)>;
    ssl_context_ptr ctx;
public:

    using ssl_ptr = std::unique_ptr<SSL, void (*)(SSL *)>;

    SSL_context() : ctx(SSL_CTX_new(TLS_server_method()), SSL_CTX_free) {

        init_openssl();

        if (ctx == nullptr) {
//            auto * thing = SSL_CTX_new(TLS_server_method());
//            std::cout << (thing ? "got thing" : "no thing\n");
            perror("SSL_CTX_new MEH!");
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    }

    [[nodiscard]] [[maybe_unused]]
    const ssl_context_ptr &get() const { return ctx; }

    ssl_ptr get_SSL(const socket_fd &fd) {
        ssl_ptr ssl(SSL_new(ctx.get()), SSL_free);

        SSL_set_fd(ssl.get(), fd.get());

        return ssl;
    }

    ~SSL_context() {
        cleanup_openssl();
    }

    bool configure(const char *cert, const char *key) {
        SSL_CTX_set_ecdh_auto(ctx, 1);

        /* Set the key and cert */
        if (SSL_CTX_use_certificate_file(ctx.get(), cert, SSL_FILETYPE_PEM) <= 0) {
            std::cout << "---------in configure error------------\n";
            ERR_print_errors_fp(stderr);
            return false;
        }

        if (SSL_CTX_use_PrivateKey_file(ctx.get(), key, SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
            return false;
        }

        return true;
    }
private:
    static std::once_flag init_openssl_once;
    static void init_openssl();

    static void cleanup_openssl() {
        EVP_cleanup();
    }
};

