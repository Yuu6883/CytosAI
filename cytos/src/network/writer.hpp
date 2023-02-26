#pragma once

#include <bit>
#include <thread>
#include <memory.h>
#include <string_view>

#ifdef WIN32
#include <intrin.h>
#else
#include <byteswap.h>
#endif

using std::string_view;
using std::u16string_view;

inline thread_local std::unique_ptr<char[]> pool(nullptr);

class Writer {
    char* ptr;
public:
    Writer() {
        reset();
    };

    void reset() {
        if (!pool.get()) pool.reset(new char[10 * 1024 * 1024]);
        ptr = pool.get();
    }

    template<typename I>
    I& ref(I init = 0) {
        I& r = * ((I*) ptr);
        r = 0;
        ptr += sizeof(I);
        return r;
    }

    template<typename I, typename O>
    inline void write(O&& input) {
        *((I*)ptr) = (I)input;
        ptr += sizeof(I);
    }

    inline void writeBE(uint16_t input) {
#ifdef WIN32
        *((uint16_t*) ptr) = _byteswap_ushort(input);
#else
        *((uint16_t*) ptr) = bswap_16(input);
#endif
        ptr += sizeof(uint16_t);
    }

    inline void writeBE(int16_t input) {
        uint16_t value = *reinterpret_cast<uint16_t*>(&input);
#ifdef WIN32
        * ((uint16_t*)ptr) = _byteswap_ushort(value);
#else
        * ((uint16_t*)ptr) = bswap_16(value);
#endif
        ptr += sizeof(uint16_t);
    }

    /*template<typename I>
    inline void writeBE(I input) {
        char buf[sizeof(I)];
        *((I*)buf) = input;
        for (int i = 0; i < sizeof(I); i++) {
            ptr[i] = buf[sizeof(I) - i - 1];
        }
        ptr += sizeof(I);
    }*/

    inline void write(string_view buffer, bool padZero = true, bool asUTF16 = false) {
        if (asUTF16) {
            auto p = buffer.data();
            for (int i = 0; i < buffer.size(); i++)
                write<char16_t>(p[i]);
            if (padZero) write<char16_t>(0);
        } else {
            memcpy(ptr, buffer.data(), buffer.size());
            ptr += buffer.size();
            if (padZero) write<uint8_t>(0);
        }
    }

    inline void write(u16string_view buffer, bool padZero = true) {
        memcpy(ptr, buffer.data(), buffer.size() * sizeof(char16_t));
        ptr += buffer.size() * sizeof(char16_t);
        if (padZero) write<char16_t>(0);
    }

    inline void fill(uint8_t v, size_t size) {
        memset(ptr, v, size);
        ptr += size;
    }

    inline size_t offset() {
        return ptr - pool.get();
    }

    string_view finalize() {
        size_t s = ptr - pool.get();
        char* out = static_cast<char*>(malloc(s));
        memcpy(out, pool.get(), s);
        ptr = pool.get();
        return string_view(out, s);
    }

    string_view buffer() {
        return string_view(pool.get(), ptr - pool.get());
    }
};