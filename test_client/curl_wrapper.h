#pragma once

#ifdef __VMS
#ifndef __USE_STD_IOSTREAM
#define __USE_STD_IOSTREAM
#endif
#include <curl.h>
#else

#include <curl/curl.h>

#endif

#include <boost/utility/string_view.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <vector>
#include <compression.h>
#include <utililty.h>

// instantiate this in main to avoid
// repeated calls, not a huge deal but seems unnecessary
struct curl_initialise {
    curl_initialise() {
        curl_global_init(CURL_GLOBAL_ALL);
    }


    ~curl_initialise() {
        curl_global_cleanup();
    }
};

class receive_buffer {
    std::vector<char> buffer;
public:
    using iter = std::vector<char>::iterator;
    using citer = std::vector<char>::const_iterator;

    void append(const void *prt, size_t bytes) {
        auto *data = reinterpret_cast<const char *>(prt);
        buffer.insert(buffer.end(), data, data + bytes);
    }

    static size_t read_response(void *ptr, size_t size, size_t nmemb, void *stream) {
        auto bytes = size * nmemb;

        auto *buffer = reinterpret_cast<receive_buffer *>(stream);
        buffer->append(ptr, bytes);

        return bytes;
    }

    iter begin() noexcept { return buffer.begin(); }

    citer begin() const noexcept { return buffer.cbegin(); }

    iter end() noexcept { return buffer.end(); }

    citer end() const noexcept { return buffer.cend(); }

    size_t size() const noexcept { return buffer.size(); }

    void print_buffer() {
        std::string zipped(buffer.begin(), buffer.end());
        std::string inflated = unzip(zipped, buffer.size());
        std::cout << inflated << "\n";
    }
};

class CURL_handle {
    bool read_headers{false};
    bool verbose{false};
    receive_buffer body;
    receive_buffer headers;
    CURL *curl;
public:
    explicit CURL_handle(bool verbose_ = false) : curl(curl_easy_init()), verbose(verbose_) {
        if (verbose && curl) {
            set_option(CURLOPT_VERBOSE, 1L);
        }
    }

    ~CURL_handle() {
        curl_easy_cleanup(curl);
    }

    bool initialised() noexcept { return curl != nullptr; }

    template<typename OPT, typename VAL>
    void set_option(OPT opt, VAL val) noexcept {
        auto res = curl_easy_setopt(curl, opt, val);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_setopt\n";
        }
    }

    void curl_version() {
        // TODO make this less pointless
        CURLversion curl_ver = CURLVERSION_NOW;
        curl_version_info_data *ver = curl_version_info(curl_ver);
        std::cout << ver->ssl_version << "\n";
    }

    long get_info() noexcept {

        long http_code = 0;
        int err_num = curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &http_code
        );
        if (err_num != CURLE_OK) {
            std::cout << "Get info: " << err_num << '\n';
        }
        return http_code;
    }

    enum ENCODING {
        PLAIN,
        GZIP,
        DEFLATE
    };

    ENCODING content_encoding() {
        std::string upper = {headers.begin().base(), headers.size()};
        boost::to_upper(upper);
        auto pos = upper.find("CONTENT-ENCODING");
        auto end = upper.find("\r\n", pos);
        if (upper.find("GZIP", pos) < end) {
            return GZIP;
        } else if (upper.find("DEFLATE", pos) < end) {
            return DEFLATE;
        }
        return PLAIN;
    }

    std::string get_data_buffer() {
        if (read_headers) {
            std::string_view body_string{body.begin().base(), body.size()};
            switch (content_encoding()) {
                case GZIP:
                    return unzip(body_string, 1024 * 1024);
                case DEFLATE:;
                    return inflate(body.begin().base(), 1024 * 1024);
                case PLAIN:
                    return {body_string.begin(), body_string.size()};
            }
        }

        return {body.begin().base(), body.size()};
    }

//    const receive_buffer &get_header_buffer() {
//        return headers;
//    }

    void set_read_headers(bool doit) noexcept {
        if ((read_headers = doit)) {
            set_option(CURLOPT_HEADERFUNCTION, receive_buffer::read_response);
            set_option(CURLOPT_HEADERDATA, reinterpret_cast<void *>(&headers));
        }
    }

    CURLcode perform() noexcept {
        // get a better error message on failure
        char error_buf[CURL_ERROR_SIZE] = {};
        set_option(CURLOPT_ERRORBUFFER, error_buf);
        set_option(CURLOPT_WRITEFUNCTION, receive_buffer::read_response);
        set_option(CURLOPT_WRITEDATA, reinterpret_cast<void *>(&body));

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            boost::string_view error(reinterpret_cast<const char *>(error_buf));
            std::cerr << "\nlibcurl: " << res << ' ';
            if (!error.empty()) {
                std::cerr << error << '\n';
            }
            std::cerr << curl_easy_strerror(res) << '\n';
        }
        return res;
    }
};


/**
 * an array of const char* creating a curl_slist
 * end with an empty string or nullptr/NULL
 */
class curl_slist_handle {
    struct curl_slist *slist;
public:
    explicit curl_slist_handle(const char *strings[]) : slist(nullptr) {
        append_strings(strings);
    }

    curl_slist_handle() : slist(nullptr) {}

    void append_strings(const char *strings[]) {
        for (int i(0); nullptr != strings[i] && '\0' != *strings[i]; ++i) {
            slist = curl_slist_append(slist, strings[i]);
        }
    }

    ~curl_slist_handle() {
        curl_slist_free_all(slist);
    }

    curl_slist *get() { return slist; }
};
