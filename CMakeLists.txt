cmake_minimum_required(VERSION 3.20)
project(ssl_http_server)

set(CMAKE_CXX_STANDARD 17)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

# put the html next to the binary for easy reading
configure_file(index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
configure_file(index_big.html ${CMAKE_BINARY_DIR}/index_big.html COPYONLY)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)

add_library(util INTERFACE)
add_library(util::util ALIAS util)
target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(ssl_http_server)
target_sources(ssl_http_server
        PRIVATE
        main.cpp
        ssl_server.cpp
        ssl_context.cpp
        )
target_link_libraries(ssl_http_server
        PRIVATE
        Boost::boost
        OpenSSL::SSL
        ZLIB::ZLIB
        )

add_subdirectory(test_client)
