
add_executable(test_client)

target_sources(test_client
        PRIVATE
        test_client.cpp
        )

target_link_libraries(test_client
        PRIVATE
        util::util
        CURL::libcurl
        OpenSSL::SSL
        ZLIB::ZLIB
        OpenSSL::Crypto
        Boost::boost
        )

add_executable(test_compression)

target_sources(test_compression
        PRIVATE
        test_compression.cpp
        )

target_link_libraries(test_compression
        PRIVATE
        util::util
        CURL::libcurl
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
        Boost::boost
        )

