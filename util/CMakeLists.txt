add_library(util INTERFACE)
add_library(util::util ALIAS util)
target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(util
        INTERFACE
        Boost::boost
        )
