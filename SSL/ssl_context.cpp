#include "ssl_context.h"

std::once_flag SSL_context::init_openssl_once;

void SSL_context::init_openssl() {
    std::call_once(init_openssl_once, [](){
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    });
}

