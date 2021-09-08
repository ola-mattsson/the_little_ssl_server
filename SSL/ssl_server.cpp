//
// Created by Ola Mattsson on 2021-09-19.
//
#include <iostream>
#include <fstream>
#include <limits>
#include "ssl_server.h"
#include <utililty.h>
#include <compression.h>
#include "http_msg.h"
#include "actions.h"

std::string create_response_html(std::string_view file_name, std::string_view accept_encoding) {
    std::cout << "Requested file: " << file_name << '\n';

    std::stringstream file_content, response;

    std::ifstream f(file_name);
    if (f.is_open()) {
        file_content << f.rdbuf();

        response
            << "HTTP/1.1 200 OK\r\n";

        if (accept_encoding.find("gzip") != std::string::npos) {
            auto zipped = gzip(file_content.str());

            response
                << "Content-Encoding: gzip\r\n"
                << "Content-Length: " << zipped.size() << "\r\n"
                << "\r\n";
            assert(zipped.size() < std::numeric_limits<std::streamsize>::max());
            auto size = static_cast<std::streamsize>(zipped.size());
            response.write(zipped.c_str(), size);

        } else if (accept_encoding.find("deflate") != std::string::npos) {
            auto deflated = deflate(file_content.str());
            response
                << "Content-Encoding: deflate\r\n"
                << "Content-Length: " << deflated.size() << "\r\n"
                << "\r\n" << deflated;
        } else {
            std::string raw = file_content.str();
            response
                << "Content-Length: " << raw.size() << "\r\n"
                << "\r\n" << raw;
        }

    } else {
        response
            << "HTTP/1.1 404 Not Found\r\n\r\n"
            << ESCAPE_LITERAL(
        <!DOCTYPE html><html lang="en"><head><meta charset="UTF-8">
                                                           <title>Title</title></head><body>No such file here</body></html>);
    }

    return response.str();
}

bool SSL_server::serve(const SSL_context::ssl_ptr &ssl) {
    char buf[1024] = {};

//    int nread =
    SSL_read(ssl.get(), buf, (sizeof buf) - 1);
    http_msg msg(buf);
    std::cout << msg;

    switch (msg.method()) {
        case http_msg::GET: {
            auto html = create_response_html(msg.requested_file(),
                                             msg.get_header("Accept-Encoding"));
            SSL_write(ssl.get(), html.c_str(), static_cast<int>(html.size()));
            break;
        }
        case http_msg::POST:
            if (msg.body.empty() || msg.body == "QUIT") {
                std::string_view response = "HTTP/1.1 200 OK\r\n"
                                            "Content-Type: text/plain\r\n"
                                            "Content-Length: 3\r\n"
                                            "\r\n"
                                            "Bye";

                SSL_write(ssl.get(), &response[0], static_cast<int>(response.size()));
                return false;
            }
            actions::create_something(ssl.get(), buf);
            break;
        case http_msg::PUT:
            actions::update_something(ssl.get(), buf);
            break;
        case http_msg::DELETE:
            actions::delete_something(ssl.get(), buf);
        default:
            std::string_view response = "HTTP/1.1 404 Not Found\r\n\r\n";
            SSL_write(ssl.get(), &response[0], static_cast<int>(response.size()));
    }

    return true;
}
