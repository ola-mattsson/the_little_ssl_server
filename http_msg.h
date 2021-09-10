#pragma once

#include <iostream>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>

struct http_msg {
    enum METHOD {
        GET,
        POST,
        PUT,
        DELETE,
        UNKNOWN
    };

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

    std::string_view headers;
    std::string_view body;
    std::vector<std::string> head_vector;

    http_msg(const http_msg &) = delete;

    explicit http_msg(char *m) : headers(m) {
        auto head_end = headers.find("\r\n\r\n");
        if (head_end != std::string_view::npos) {
            headers = std::string_view(&headers[0], head_end);
            body = std::string_view(&headers[head_end + 4]);
        }
        boost::split(head_vector, headers, [](auto &&c) { return c == '\r'; }, boost::token_compress_off);
        std::for_each(head_vector.begin(), head_vector.end(), [](auto &&header) {
            if (header[0] == '\n') {
                header.erase(0, 1);
            }
        });
    }

    std::string requested_file() {
        if (head_vector.empty()) { return ""; }
        auto &method = head_vector.front();

        tokenizer tok(method, boost::char_separator<char>(" "));
        tokenizer::iterator iter = tok.begin();
        ++iter;
        const std::string &name = *iter;
        if (!name.empty() && name[0] == '/') {
            return &name[1];
        }
        return "";
    }

    std::string get_header(std::string_view name) {
        auto &method = head_vector.front();
        auto header = std::find_if(head_vector.begin(), head_vector.end(), [&name](auto &&header) {
            return header.find(name) == 0;
        });
        if (header != head_vector.end()) {
            return *header;
        }

        return "";
    }

    METHOD method() {
        if (head_vector.empty()) { return UNKNOWN; }

        auto method = head_vector.begin();

        if (method->find("POST ") == 0) {
            return POST;
        }
        if (method->find("GET ") == 0) {
            return GET;
        }
        if (method->find("PUT ") == 0) {
            return PUT;
        }
        if (method->find("DELETE ") == 0) {
            return DELETE;
        }
        return UNKNOWN;
    }

    friend std::ostream &operator<<(std::ostream &os, const http_msg &msg) {
        os << "headers: " << msg.headers << "\nbody: " << msg.body;
        return os;
    }

};
