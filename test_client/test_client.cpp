#include <iostream>
#include "curl_wrapper.h"
#include <utililty.h> // ich
#include <algorithm>
#include <compression.h>


void test_GET() {

    CURL_handle curl;
    if (curl.initialised()) {

        curl.set_read_headers(true);

        curl.set_option(CURLOPT_URL, "https://localhost:12344/index_big.html");
        const char *headers[] = {
            "Content-Type: application/json",
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
            "charset: UTF-8",
            "Accept-Encoding: gzip, deflate",
            "Connection: keep-alive",
            nullptr
        };
        curl_slist_handle slist(headers);
        curl.set_option(CURLOPT_HTTPHEADER, slist.get());
        curl.set_option(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl.set_option(CURLOPT_TIMEOUT, 10L);
        curl.set_option(CURLOPT_HTTP_VERSION, int(CURL_HTTP_VERSION_1_1));
        curl.set_option(CURLOPT_USERAGENT, "the_test_client/0.0.98");

        curl.perform();
//        std::cout << __FUNCTION__ << "\n";
//        std::cout << curl.get_data_buffer() << "\n";
    }
}

void test_POST() {

    const char* json = ESCAPE_LITERAL({thing:42, list:[1, 3, 4]});

    CURL_handle curl;
    if (curl.initialised()) {

        curl.set_read_headers(true);

        const char *headers[] = {
            "Content-Type: application/json",
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
            "charset: UTF-8",
            "Accept-Encoding: gzip, deflate",
            "Connection: keep-alive",
            nullptr
        };
        curl_slist_handle slist(headers);

        curl.set_option(CURLOPT_HTTPHEADER, slist.get());
        curl.set_option(CURLOPT_URL, "https://localhost:12344");
        curl.set_option(CURLOPT_POSTFIELDS, json);
        curl.set_option(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl.set_option(CURLOPT_TIMEOUT, 10L);
        curl.set_option(CURLOPT_HTTP_VERSION, int(CURL_HTTP_VERSION_1_1));
        curl.set_option(CURLOPT_USERAGENT, "the_test_client/0.0.98");

        curl.perform();
//        std::cout << __FUNCTION__ << "\n";
//        std::cout << curl.get_data_buffer() << "\n";
    }

}
void quit() {
    const char* json = "QUIT";

    CURL_handle curl;
    if (curl.initialised()) {

        curl.set_read_headers(true);

        const char *headers[] = {
            "Content-Type: text/plain",
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
            "charset: UTF-8",
            nullptr
        };
        curl_slist_handle slist(headers);

        curl.set_option(CURLOPT_HTTPHEADER, slist.get());
        curl.set_option(CURLOPT_URL, "https://localhost:12344");
        curl.set_option(CURLOPT_POSTFIELDS, json);
        curl.set_option(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl.set_option(CURLOPT_TIMEOUT, 10L);
        curl.set_option(CURLOPT_HTTP_VERSION, int(CURL_HTTP_VERSION_1_1));
        curl.set_option(CURLOPT_USERAGENT, "the_test_client/0.0.98");

        curl.perform();
        std::cout << __FUNCTION__ << curl.get_data_buffer() << "\n";

    }

}
int main() {
    const curl_initialise curl_initialise;

    for (int i(0); i < 1000; ++i) {
        test_GET();
        test_POST();
    }
//    quit();
}