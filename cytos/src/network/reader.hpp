#pragma once

#include <vector>
#include <string_view>

#ifdef WIN32
#include <intrin.h>
#else
#include <byteswap.h>
#endif

#include "../misc/logger.hpp"

using std::vector;
using std::string_view;
using std::u16string_view;

constexpr string_view EMPTY_BUF("");
constexpr u16string_view EMPTY_BUF_U16(u"");

class Reader {
    const char* origin;
    char* ptr;
    size_t size;
    bool& error;

    bool check(size_t limit) {
        if (offset() + limit > size) {
            error = true;
            return false;
        }
        return true;
    }

public:
    Reader(string_view view, bool& e) : origin(view.data()), 
        ptr((char*) view.data()), size(view.size()),
        error(e) {
            error = false;
        };

    inline size_t offset() { return ptr - origin; }
    inline size_t length() { return size; }

    inline bool eof() { return offset() == length(); }

    inline void skip(size_t b) {
        if (check(b)) ptr += b;
    } 

    template<typename T>
    inline T read() {
        if (check(sizeof(T))) {
            T t = *((T*) (ptr));
            ptr += sizeof(T);
            return t;
        }
        return 0;
    }

    template<typename I>
    inline void read(I& output) {
        if (check(sizeof(I))) {
            output = *((I*) (ptr));
            ptr += sizeof(I);
        }
    }

    inline uint16_t readBE() {
        if (check(sizeof(uint16_t))) {
            uint16_t t = *((uint16_t*) ptr);
            ptr += sizeof(uint16_t);
#ifdef WIN32
            return _byteswap_ushort(t);
#else
            return bswap_16(t);
#endif
        }
        return 0;
    }

    inline string_view utf8() {
        char* start = ptr;
        char c = 'a';
        while (c != 0) {
            read(c);
            if (error) return EMPTY_BUF;
        }
        return string_view(start, ptr - start - 1);
    }

    inline u16string_view ucs2() {
        auto start = (char16_t*) ptr;
        char16_t c = 'a';
        while (c != 0) {
            read(c);
            if (error) return EMPTY_BUF_U16;
        }
        return u16string_view(start, (char16_t* ) ptr - start - 1);
    }

    inline string_view rest() {
        auto left = size - offset();
        return string_view(ptr, left);
    }
};
