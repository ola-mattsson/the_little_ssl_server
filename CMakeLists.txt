cmake_minimum_required(VERSION 3.20)
project(ssl_http_server)

set(CMAKE_CXX_STANDARD 17)


if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
        fmt/7.1.3
        boost/1.69.0
        libpq/11.11
        openssl/1.1.1k
        CMAKE_TARGETS
        BUILD missing
        GENERATORS
        cmake_find_package
        OPTIONS
        boost:zlib=False
        boost:header_only=True
        )

find_package(OpenSSL REQUIRED)

add_executable(ssl_http_server main.cpp)
target_link_libraries(ssl_http_server
        PRIVATE
        OpenSSL::SSL
        )