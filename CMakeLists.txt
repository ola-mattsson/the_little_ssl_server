cmake_minimum_required(VERSION 3.20)
project(ssl_http_server)

set(CMAKE_CXX_STANDARD 17)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

# put the html next to the binary for easy reading
configure_file(files/index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
configure_file(files/index_big.html ${CMAKE_BINARY_DIR}/index_big.html COPYONLY)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

add_executable(ssl_http_server)
target_sources(ssl_http_server
        PRIVATE
        main.cpp
        )

target_link_libraries(ssl_http_server
        PRIVATE
        Boost::boost
        ZLIB::ZLIB
        SHS::SSL
        Threads::Threads
        )

add_subdirectory(test_client)
add_subdirectory(SSL)
add_subdirectory(util)
