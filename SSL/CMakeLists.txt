add_library(SSL)
add_library(SHS::SSL ALIAS SSL)

target_sources(SSL
        PRIVATE
        ssl_context.cpp
        ssl_server.cpp
        )
target_include_directories(SSL
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(SSL
        PUBLIC
        OpenSSL::SSL
        util::util
        )