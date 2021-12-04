#pragma once
/**
 * a socket wrapper, of sorts
 */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/cdefs.h>
#include <netinet/in.h>


struct socket_fd {
    using socket_type = int;
private:
    socket_type fd;
public:
    explicit socket_fd() : fd(-1) {}

    explicit socket_fd(socket_type fd_) : fd(fd_) {}

    explicit socket_fd(const socket_fd &) = delete;

    socket_fd(socket_fd && other) noexcept {
        fd = other.fd;
        other.fd = -1;
    }

    socket_fd &operator=(socket_fd &&other) noexcept {
        *this = other;
        other.fd = -1;
        return *this;
    }

    socket_fd &operator=(const socket_fd &other) {
        if (fd != other.fd && valid()) {
            close_socket();
        }
        fd = other.fd;
        return *this;
    }

    ~socket_fd() { if (valid()) { close_socket(); }}

    void close_socket() const {
        close(fd);
    }

    [[nodiscard]]
    bool valid() const { return fd != -1; }

    explicit operator socket_type() const { return fd; }

    [[nodiscard]]
    socket_type get() const { return fd; }

    static socket_fd create_socket(int32_t port) {

        struct sockaddr_in addr = {};

        int sock(socket(AF_INET, SOCK_STREAM, 0));

        if (sock == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            perror("bind");
            return socket_fd{};
        }

        if (listen(sock, SOMAXCONN) == -1) {
            perror("listen");
            return socket_fd{};
        }

        return socket_fd(sock);
    }

    void wait_request(const socket_fd &socket) {
        struct sockaddr_in addr = {};
        socklen_t addrlen = sizeof(addr);
        fd = accept(socket.get(), (struct sockaddr *) &addr, (socklen_t *) &addrlen);
        if (fd < 0) {
            perror("accept");
        }

    }
};
