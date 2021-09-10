#pragma once
#include <boost/tokenizer.hpp>

#define ESCAPE_LITERAL(...) #__VA_ARGS__

template<typename T>
struct TypeParseTraits;

#define REGISTER_PARSE_TYPE(X) template <> struct TypeParseTraits<X> \
    { static const char* name; } ; const char* TypeParseTraits<X>::name = #X




