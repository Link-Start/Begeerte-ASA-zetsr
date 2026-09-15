#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <string>
#include <string_view>

// 把 __TIME__ ("HH:MM:SS") 转换成从零点开始的秒数
constexpr unsigned int CompileTimeSeconds() {
    return static_cast<unsigned int>(__TIME__[0] - '0') * 36000u
        + static_cast<unsigned int>(__TIME__[1] - '0') * 3600u
        + static_cast<unsigned int>(__TIME__[3] - '0') * 600u
        + static_cast<unsigned int>(__TIME__[4] - '0') * 60u
        + static_cast<unsigned int>(__TIME__[6] - '0') * 10u
        + static_cast<unsigned int>(__TIME__[7] - '0');
}

// 由 __TIME__ 导出的基础密钥（加 1 以避免 xorshift 的 0 不动点）
constexpr unsigned int TimeBaseKey() {
    return CompileTimeSeconds() + 1u;
}

// 编译期状态推进（xorshift32）
constexpr unsigned int AdvanceState(unsigned int s) {
    if (s == 0u) s = 1u;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return s;
}

// 窄字符单字节密钥，范围 [1, 255]
constexpr char GenerateByteKey(unsigned int seed, size_t index) {
    unsigned int state = seed + static_cast<unsigned int>(index);
    state = AdvanceState(state);
    return static_cast<char>((state % 255u) + 1u);
}

// 宽字符单字节密钥，范围 [1, 0xFFFF]
constexpr wchar_t GenerateWideKey(unsigned int seed, size_t index) {
    unsigned int state = seed + static_cast<unsigned int>(index);
    state = AdvanceState(state);
    return static_cast<wchar_t>((state % 0xFFFEu) + 1u);
}

// 根据计数器与编译期时间生成每个字符串的种子
constexpr unsigned int StringSeed(unsigned int counter) {
    unsigned int state = TimeBaseKey() + counter;
    state = AdvanceState(state);
    return state;
}

#define GENERATE_SEED(counter) (StringSeed(static_cast<unsigned int>(counter)))

// 栈上解密缓冲区（窄）
// 
// XorBuffer<N>：持有栈上缓冲区 data[N]（窄字符明文）和 wdata[N]（宽字符明文）。
// 它由 XorString::crypt() / c_str() 按值返回，因此：
//   - 返回的对象本身是临时的，生命周期只到当前完整表达式结束；
//   - 若需要在多条语句之间使用，请先用 auto 接收，例如：
//         auto buf = _XOR_("hello").crypt();
//         const char* p = buf.c_str();                  // 安全
//         std::wstring_view wsv = buf.wstring_view();   // 安全
//   - 不要保存 XorBuffer 内部的指针/视图到临时对象之外。
template <size_t N>
struct XorBuffer {
    char data[N];
    mutable wchar_t wdata[N];
    mutable bool wdata_ready = false;

    XorBuffer() : data{}, wdata{}, wdata_ready(false) {}

    __forceinline const char* c_str() const { return data; }
    __forceinline const char8_t* c_str_u8() const {
        return reinterpret_cast<const char8_t*>(data);
    }

    __forceinline operator const char* () const { return data; }
    __forceinline operator const char8_t* () const {
        return reinterpret_cast<const char8_t*>(data);
    }
    __forceinline operator const void* () const {
        return static_cast<const void*>(data);
    }

    __forceinline std::string str() const {
        return std::string(data, N - 1);
    }
    __forceinline operator std::string() const { return str(); }

    __forceinline std::string_view string_view() const {
        return std::string_view(data, N - 1);
    }
    __forceinline operator std::string_view() const { return string_view(); }

    __forceinline std::u8string u8str() const {
        return std::u8string(reinterpret_cast<const char8_t*>(data), N - 1);
    }
    __forceinline operator std::u8string() const { return u8str(); }

    __forceinline std::u8string_view u8string_view() const {
        return std::u8string_view(reinterpret_cast<const char8_t*>(data), N - 1);
    }
    __forceinline operator std::u8string_view() const { return u8string_view(); }

    __forceinline std::wstring_view wstring_view() const {
        if (!wdata_ready) {
            for (size_t i = 0; i < N; ++i) {
                wdata[i] = static_cast<wchar_t>(static_cast<unsigned char>(data[i]));
            }
            wdata_ready = true;
        }
        return std::wstring_view(wdata, N - 1);
    }

    __forceinline std::wstring wstr() const {
        return std::wstring(wstring_view());
    }
    __forceinline operator std::wstring() const { return wstr(); }
};

// 栈上解密缓冲区（宽）
template <size_t N>
struct XorWBuffer {
    wchar_t data[N];

    XorWBuffer() : data{} {}

    __forceinline const wchar_t* c_str() const { return data; }
    __forceinline operator const wchar_t* () const { return data; }

    __forceinline std::wstring wstr() const {
        return std::wstring(data, N - 1);
    }
    __forceinline operator std::wstring() const { return wstr(); }

    __forceinline std::wstring_view wstring_view() const {
        return std::wstring_view(data, N - 1);
    }
    __forceinline operator std::wstring_view() const { return wstring_view(); }
};

// XorString
template <size_t N, unsigned int Seed>
class XorString {
private:
    // 内部只保存密文，且永不修改。对象生命周期内不会出现明文。
    const char data[N];

    template <size_t... Is>
    constexpr XorString(const char(&str)[N], std::index_sequence<Is...>)
        : data{ static_cast<char>(str[Is] ^ GenerateByteKey(Seed, Is))... } {
    }

    template <size_t... Is>
    constexpr XorString(const char8_t(&str)[N], std::index_sequence<Is...>)
        : data{ static_cast<char>(static_cast<char>(str[Is]) ^ GenerateByteKey(Seed, Is))... } {
    }

public:
    constexpr XorString(const char(&str)[N])
        : XorString(str, std::make_index_sequence<N>{}) {
    }

    constexpr XorString(const char8_t(&str)[N])
        : XorString(str, std::make_index_sequence<N>{}) {
    }

    // 解密到栈上临时缓冲区，按值返回。
    __forceinline XorBuffer<N> crypt() const {
        XorBuffer<N> buf;
        for (size_t i = 0; i < N; ++i) {
            buf.data[i] = static_cast<char>(data[i] ^ GenerateByteKey(Seed, i));
        }
        return buf;
    }

    __forceinline XorBuffer<N> crypt_u8() const { return crypt(); }
    __forceinline XorBuffer<N> c_str() const { return crypt(); }

    __forceinline std::string str() const { return crypt().str(); }
    __forceinline operator std::string() const { return str(); }

    __forceinline std::string_view string_view() const {
        return crypt().string_view();
    }

    __forceinline std::u8string u8str() const { return crypt().u8str(); }
    __forceinline operator std::u8string() const { return u8str(); }

    __forceinline std::u8string_view u8string_view() const {
        return crypt().u8string_view();
    }

    __forceinline std::wstring_view wstring_view() const {
        return crypt().wstring_view();
    }

    __forceinline std::wstring wstr() const { return crypt().wstr(); }
    __forceinline operator std::wstring() const { return wstr(); }

    __forceinline operator XorBuffer<N>() const { return crypt(); }
};

// ---------------- XorStringW（宽字符串） ----------------

template <size_t N, unsigned int Seed>
class XorStringW {
private:
    // 内部只保存密文，且永不修改。对象生命周期内不会出现明文。
    const wchar_t data[N];

    template <size_t... Is>
    constexpr XorStringW(const wchar_t(&str)[N], std::index_sequence<Is...>)
        : data{ static_cast<wchar_t>(str[Is] ^ GenerateWideKey(Seed, Is))... } {
    }

public:
    constexpr XorStringW(const wchar_t(&str)[N])
        : XorStringW(str, std::make_index_sequence<N>{}) {
    }

    // 解密到栈上临时缓冲区，按值返回。
    __forceinline XorWBuffer<N> crypt() const {
        XorWBuffer<N> buf;
        for (size_t i = 0; i < N; ++i) {
            buf.data[i] = static_cast<wchar_t>(data[i] ^ GenerateWideKey(Seed, i));
        }
        return buf;
    }

    __forceinline XorWBuffer<N> c_str() const { return crypt(); }

    __forceinline std::wstring wstr() const { return crypt().wstr(); }
    __forceinline operator std::wstring() const { return wstr(); }

    __forceinline std::wstring_view wstring_view() const {
        return crypt().wstring_view();
    }

    __forceinline operator XorWBuffer<N>() const { return crypt(); }
};

// 空字符串特化
template <unsigned int Seed>
class XorString<1, Seed> {
public:
    constexpr XorString(const char(&)[1]) {}
    constexpr XorString(const char8_t(&)[1]) {}

    __forceinline XorBuffer<1> crypt() const {
        XorBuffer<1> buf{};
        return buf;
    }

    __forceinline XorBuffer<1> crypt_u8() const { return crypt(); }
    __forceinline XorBuffer<1> c_str() const { return crypt(); }

    __forceinline std::string str() const { return std::string(); }
    __forceinline operator std::string() const { return str(); }

    __forceinline std::string_view string_view() const { return std::string_view(); }

    __forceinline std::u8string u8str() const { return std::u8string(); }
    __forceinline operator std::u8string() const { return u8str(); }

    __forceinline std::u8string_view u8string_view() const {
        return std::u8string_view();
    }

    __forceinline std::wstring_view wstring_view() const {
        return std::wstring_view();
    }

    __forceinline std::wstring wstr() const { return std::wstring(); }
    __forceinline operator std::wstring() const { return wstr(); }

    __forceinline operator XorBuffer<1>() const { return crypt(); }
};

template <unsigned int Seed>
class XorStringW<1, Seed> {
public:
    constexpr XorStringW(const wchar_t(&)[1]) {}

    __forceinline XorWBuffer<1> crypt() const {
        XorWBuffer<1> buf{};
        return buf;
    }

    __forceinline XorWBuffer<1> c_str() const { return crypt(); }

    __forceinline std::wstring wstr() const { return std::wstring(); }
    __forceinline operator std::wstring() const { return wstr(); }

    __forceinline std::wstring_view wstring_view() const {
        return std::wstring_view();
    }

    __forceinline operator XorWBuffer<1>() const { return crypt(); }
};

// 工厂函数
//
// 工厂以 Seed 为显式模板参数，N 由数组引用自动推导，
// 这样对 L"..." 也能得到正确的元素个数（而不是字节数）。

template <unsigned int Seed, size_t N>
__forceinline XorString<N, Seed> MakeXor(const char(&str)[N]) {
    return XorString<N, Seed>(str);
}

template <unsigned int Seed, size_t N>
__forceinline XorString<N, Seed> MakeXor(const char8_t(&str)[N]) {
    return XorString<N, Seed>(str);
}

template <unsigned int Seed, size_t N>
__forceinline XorStringW<N, Seed> MakeXor(const wchar_t(&str)[N]) {
    return XorStringW<N, Seed>(str);
}

#define _XOR_(str) (MakeXor<GENERATE_SEED(__COUNTER__)>(str))