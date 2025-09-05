#pragma once

#include "util/debug_macro.hpp"
#include <type_traits>
#include <cassert>

template <size_t MAX_TS, typename CurrentT, typename ...Ts>
struct Indices {
    static constexpr size_t idx = Indices<MAX_TS, Ts...>::idx - 1;
};

template<size_t MAX_TS, typename T>
struct Indices<MAX_TS, T> {
    static constexpr size_t idx = MAX_TS - 1;
};

template <size_t MAX_TS, typename Compare, typename T, typename ...Ts>
struct GetIndexOf {
private:
    static constexpr size_t GetIndex() {
        if constexpr (std::is_same_v<Compare, T>) {
            return Indices<MAX_TS, T, Ts...>::idx;
        } else if constexpr (sizeof...(Ts) == 0) {
            static_assert(false, "Unreachable: Compare not in Ts...");
        } else {
            return GetIndexOf<MAX_TS, Compare, Ts...>::idx;
        }
    }
public:
    static constexpr size_t idx = GetIndex();
};

template<size_t Idx, typename T, typename ...Ts>
struct GetTypeOf {
private:
    static constexpr auto *GetTypePointer() {
        if constexpr (Idx == 0) {
            return static_cast<T *>(nullptr);
        } else {
            return static_cast<typename GetTypeOf<Idx - 1, Ts...>::type *>(nullptr);
        }
    }
public:
    using type = std::remove_pointer_t<decltype(GetTypePointer())>;
};

class CombinedPointer {
public:
    CombinedPointer(uintptr_t ptr, uint8_t tag, bool is_const) : ptr(ptr), tag(tag), is_const(is_const) {}
    uintptr_t getPtr() const { return ptr; }
    uint8_t getTag() const { return tag; }
    bool isConst() const { return is_const; }
    void setPtr(uintptr_t ptr) { this->ptr = ptr; }
    void setTag(uint8_t tag) { this->tag = tag; }
    void setConst(bool is_const) { this->is_const = is_const; }
private:
    uintptr_t ptr;
    bool is_const;
    uint8_t tag;
};

class TaggegPointer {
public:
    TaggegPointer(uintptr_t ptr, uint8_t tag, bool is_const) {
        DEBUG_LINE(assert(ptr | const_tag_mask == 0))
        bits = (uintptr_t(is_const) << const_shift) | (uintptr_t(tag) << tag_shift) | ptr;
    }
    uintptr_t getPtr() const { return bits & ptr_mask; }
    uint8_t getTag() const { return (bits & tag_mask) >> tag_shift; };
    bool isConst() const { return bits >> const_shift; }
    void setPtr(uintptr_t ptr) { bits = (bits & const_tag_mask) | ptr; }
    void setTag(uint8_t tag) { bits = (bits & ptr_mask) | ((uintptr_t(tag) << tag_shift) & tag_mask); }
    void setConst(bool is_const) { bits = (bits & tag_ptr_mask) | (uintptr_t(is_const) << const_shift); }
private:
    uintptr_t bits;
    static constexpr uintptr_t const_shift = sizeof(uintptr_t) * uintptr_t(8) - uintptr_t(1);
    static constexpr uintptr_t const_mask = uintptr_t(1) << const_shift;
    static constexpr uintptr_t tag_bit_num = 6;
    static constexpr uintptr_t tag_shift = const_shift - tag_bit_num;
    static constexpr uintptr_t tag_mask = ((uintptr_t(1) << tag_bit_num) - uintptr_t(1)) << tag_shift;
    static constexpr uintptr_t const_tag_mask = tag_mask | const_mask;
    static constexpr uintptr_t ptr_mask = ~const_tag_mask;
    static constexpr uintptr_t tag_ptr_mask = ~const_mask;
};

template <typename ...Ts>
class GeneralizedPtr {
private:
    static constexpr size_t MAX_TS = sizeof...(Ts);
    TaggegPointer pointer;
public:
    template <typename T>
    GeneralizedPtr(T *ptr) : pointer(
            reinterpret_cast<uintptr_t>(ptr),
            GetIndexOf<MAX_TS, std::decay_t<T>, Ts...>::idx,
            std::is_const_v<T>
        ) {}

    GeneralizedPtr() : pointer(
            0,
            MAX_TS,
            true
        ) {}

    bool isValid() const { return pointer.getPtr() != 0; }

    uintptr_t ptr() const { return pointer.getPtr(); }
private:
    template <size_t Idx>
    typename GetTypeOf<Idx, Ts...>::type *cast() {
        // Idx must equal to Ts_idx
        return reinterpret_cast<typename GetTypeOf<Idx, Ts...>::type *>(pointer.getPtr());
    }

    template <size_t Idx>
    const auto *constCast() const {
        return reinterpret_cast<const typename GetTypeOf<Idx, Ts...>::type *>(pointer.getPtr());
    }
protected:

    #define B(max_ts) else if constexpr (MAX_TS == max_ts) { switch(pointer.getTag()) {
    #define E() } }
    #define BODY() \
        B(1) C(0) E() \
        B(2) C(0) C(1)  E() \
        B(3) C(0) C(1) C(2) E() \
        B(4) C(0) C(1) C(2) C(3)  E() \
        B(5) C(0) C(1) C(2) C(3) C(4) E() \
        B(6) C(0) C(1) C(2) C(3) C(4) C(5) E() \
        B(7) C(0) C(1) C(2) C(3) C(4) C(5) C(6) E() \
        B(8) C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7) E() \
        B(9) C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7) C(8) E() \
        B(10) C(0) C(1) C(2) C(3) C(4) C(5) C(6) C(7) C(8) C(9) E() \

    template <typename Func>
    auto Dispatch(Func &&func) {
        if constexpr (MAX_TS == 0) {
        }
        #define C(idx) case idx: if (!pointer.isConst()) return func(cast<idx>()); else { assert(false); return func(cast<idx>()); }
        BODY()
        #undef C
        else {
            static_assert(false, "Unreachable: Please Add Code");
        }
        // Unreachable, Just For Skiping Warning
        return func(reinterpret_cast<decltype(cast<0>())>(0));
    }

    template <typename Func>
    auto DispatchConst(Func &&func) const {
        if constexpr (MAX_TS == 0) {
        }
        #define C(idx) case idx: return func(constCast<idx>());
        BODY()
        #undef C
        else {
            static_assert(false, "Unreachable: Please Add Code");
        }
        // Unreachable, Just For Skiping Warning
        return func(reinterpret_cast<decltype(constCast<0>())>(0));
    }

    #undef BODY
    #undef E
    #undef B
};

#define DISPATCH(func_name, ...) Dispatch([&](auto *ptr) { return ptr->func_name(__VA_ARGS__); })
#define DISPATCH_CONST(func_name, ...) DispatchConst([&](const auto *ptr) { return ptr->func_name(__VA_ARGS__); })
