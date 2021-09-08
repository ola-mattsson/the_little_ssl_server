#pragma once
#include <zlib.h>
#include <zconf.h>
#include <string>
#include <vector>
#include <array>

/*
 * easy to use compression algorithms.
 *
 * https://stackoverflow.com/questions/67732232/in-zlib-programming-will-the-chunk-size-affect-the-compressed-file-size
 */

std::string deflate(std::string_view str) {
    std::vector<unsigned char> out(str.size());

    z_stream def_stream = {};
    def_stream.avail_in = str.size() + 1;
    // this is just dirty, does the c cast "(Bytef *)" byt hey, that's what zlib needs
    def_stream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(&str[0]));

    def_stream.avail_out = out.size();
    def_stream.next_out = &out[0];

    deflateInit(&def_stream, Z_BEST_COMPRESSION);
    deflate(&def_stream, Z_FINISH);
    deflateEnd(&def_stream);

    return {(char *) &out[0], def_stream.total_out - 1};
}

std::string gzip(std::string_view str) {
    std::vector<unsigned char> out(str.size()+10);

    z_stream def_stream = {};
    def_stream.avail_in = str.size() + 1;
    def_stream.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(&str[0]));

    def_stream.avail_out = out.size();
    def_stream.next_out = &out[0];

    deflateInit2(&def_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    deflate(&def_stream, Z_FINISH);
    deflateEnd(&def_stream);

    return {(char*)&out[0], def_stream.total_out - 1};
}

std::string inflate(std::string_view str, size_t out_size) {
    std::vector<char> out(out_size);

    z_stream params = {};

    params.avail_in = str.size();
    params.next_in = (Bytef *)&str[0];
    params.avail_out = out_size;
    params.next_out = (Bytef *)&out[0];

    inflateInit(&params);
    inflate(&params, Z_NO_FLUSH);
    inflateEnd(&params);

    return {&out[0], params.total_out - 1};
}

std::string unzip(std::string_view str, size_t out_size) {
    std::vector<char> out(out_size);

    z_stream params = {};

    params.avail_in = str.size();
    params.next_in = (Bytef *)&str[0];
    params.avail_out = out_size;
    params.next_out = (Bytef *)&out[0];

    inflateInit2(&params, 16 + MAX_WBITS);
    int inf_result = inflate(&params, Z_NO_FLUSH);
    inf_result = inflateEnd(&params);
    if (params.total_out > 0) {
        return {&out[0], params.total_out - 1};
    } else {
        return {};
    }
}

// play with this some day
int def(FILE *source, FILE *dest, int level, int memLevel)
{
    static size_t CHUNK = 16384;

    int ret, flush;
    unsigned have;
    z_stream strm = {};
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    ret = deflateInit2(&strm, level, Z_DEFLATED, MAX_WBITS + 16, memLevel, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return ret;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

// play with this some day
//#define CHUNK 16384ULL
int def(std::string_view source, std::vector<char> &dest)
{
    static const size_t CHUNK = 10;
    z_stream strm = {};

    std::array<unsigned char, CHUNK> out = {};

    int32_t ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                               MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK) {
        return ret;
    }
    int32_t flush = 0;
    auto read = 0UL, left = source.size(), current = 0UL;
    do {
        auto delta = std::min(CHUNK, source.size() - read);
        left -= delta;
        read += delta;
        current = read - delta;


        strm.avail_in = delta;
        flush = left < CHUNK ? Z_FINISH : Z_NO_FLUSH;
        // erk
        strm.next_in = reinterpret_cast<Bytef *>(const_cast<char*>(&source[current]));

        do {
            strm.avail_out = delta;
            strm.next_out = &out[current];
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            const auto have = CHUNK - strm.avail_out;

            dest.insert(dest.end(), out.begin(), out.end());

        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}
