
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <csignal>
//#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string_view>
#include <iostream>

namespace {

    int create_socket(int32_t port) {
        int s;
        struct sockaddr_in addr = {};

        s = socket(AF_INET, SOCK_STREAM, 0);

        if (s == -1) {
            perror("socket");
            return -1;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            perror("bind");
            close(s);
            return -1;
        }

        if (listen(s, SOMAXCONN) == -1) {
            perror("listen");
            close(s);
            return -1;
        }

        return s;
    }

    void init_openssl() {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    }

    void cleanup_openssl() {
        EVP_cleanup();
    }

//    class SSL_context {
//        SSL_CTX* ctx;
//        SSL_context() : ctx(nullptr) {
//            const SSL_METHOD* method = SSLv23_server_method();
//            ctx = SSL_CTX_new(method);
//
//            if (ctx == nullptr) {
//                perror("SSL_CTX_new");
//                ERR_print_errors_fp(stderr);
//            }
//        }
//    };
    SSL_CTX *create_context() {

        const SSL_METHOD* method = SSLv23_server_method();
        SSL_CTX *ctx = SSL_CTX_new(method);

        if (ctx == nullptr) {
            perror("SSL_CTX_new");
            ERR_print_errors_fp(stderr);
            return nullptr;
        }

        return ctx;
    }

    int configure_context(
        SSL_CTX *ctx,
        const char *cert,
        const char *key
    ) {
        SSL_CTX_set_ecdh_auto(ctx, 1);

        /* Set the key and cert */
        if (SSL_CTX_use_certificate_file(
            ctx,
            cert,
            SSL_FILETYPE_PEM
        ) <= 0) {
            ERR_print_errors_fp(stderr);
            return -1;
        }

        if (SSL_CTX_use_PrivateKey_file(
            ctx,
            key,
            SSL_FILETYPE_PEM
        ) <= 0) {
            ERR_print_errors_fp(stderr);
            return -1;
        }

        return 0;
    }

    int init = 0;
    int sfd = -1;
    SSL_CTX *ctx = nullptr;

    void sig_handler(int signo) {
        if (signo == SIGINT) {
            if (sfd != -1) {
                close(sfd);
            }

            if (ctx != nullptr) {
                SSL_CTX_free(ctx);
            }

            if (init != 0) {
                cleanup_openssl();
            }

            fputc('\n', stdout);
            exit(EXIT_SUCCESS);
        }
    }

    void create_user(SSL *ssl, char *inp __attribute__((unused))) {
        char uuid_str[37];
        uuid_t uuid;
        static char response[1024];
        static char json[1024];

        uuid_generate(uuid);
        uuid_unparse(uuid, uuid_str);

        snprintf(
            json,
            1024,
            R"({"id":"%s"})",
            uuid_str
        );

        snprintf(
            response,
            1024,
            "HTTP/1.1 201 Created\r\n"
            "Content-Type: application/scim+json\r\n"
            "\r\n"
            "%s",
            json
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
    }

    void update_user(SSL *ssl, char *inp __attribute__((unused))) {
        static char response[1024];

        snprintf(
            response,
            1024,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/scim+json\r\n"
            "\r\n"
            "{}"
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
    }

    void delete_user(SSL *ssl, char *inp __attribute__((unused))) {
        static char response[1024];

        snprintf(
            response,
            1024,
            "HTTP/1.1 204 No Content\r\n"
            "\r\n"
        );

        SSL_write(ssl, response, static_cast<int>(strlen(response)));
    }

    bool test_server(SSL *ssl) {
        char buf[1024] = {};

        const char *response = "HTTP/1.1 404 Not Found\r\n\r\n";

        int nread = SSL_read(ssl, buf, sizeof(buf) - 1);

        std::string_view request(buf, nread);
        std::cout << request << '\n';
        if (request.compare(0, 4, "POST") == 0) {
            create_user(ssl, buf);
        } else if (request.compare(0, 4, "PUT") == 0) {
            update_user(ssl, buf);
        } else if (request.compare(0, 4, "DELETE") == 0) {
            delete_user(ssl, buf);
        } else if (request.empty() || request == "QUIT") {
            return false;
        } else {
            SSL_write(ssl, response, static_cast<int>(strlen(response)));
        }
        return true;
    }
}

int main(int argc, char *argv[]) {
    int err;

    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " port cert key\n";
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    init_openssl();
    init = 1;
    ctx = create_context();

    if (ctx == nullptr) {
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    err = configure_context(ctx, argv[2], argv[3]);

    if (err == -1) {
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }
    int port = static_cast<int>(std::strtol(argv[1], nullptr, 10));
    sfd = create_socket(port);

    if (sfd == -1) {
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    while (true) {
        struct sockaddr_in addr = {};
        socklen_t addrlen = sizeof(addr);
        SSL *ssl;
        int cfd;

        cfd = accept(sfd, (struct sockaddr *) &addr, &addrlen);

        if (cfd == -1) {
            perror("accept");
            continue;
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cfd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(cfd);
            continue;
        }

        if (!test_server(ssl)) {
            break;
        }
        SSL_free(ssl);
        close(cfd);
    }

    close(sfd);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    return 0;
}
