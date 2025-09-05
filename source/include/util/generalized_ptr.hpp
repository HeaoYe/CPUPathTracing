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

namespace internal {
    template<typename F, typename T0>
    auto Dispatch(F &&func, void *ptr, uint8_t tag) {
        return func(static_cast<T0 *>(ptr));
    }

    template<typename F, typename T0, typename T1>
    auto Dispatch(F &&func, void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<T0 *>(ptr));
        default:
            return func(static_cast<T1 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2>
    auto Dispatch(F &&func, void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<T0 *>(ptr));
        case 1:
            return func(static_cast<T1 *>(ptr));
        default:
            return func(static_cast<T2 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2, typename T3>
    auto Dispatch(F &&func, void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<T0 *>(ptr));
        case 1:
            return func(static_cast<T1 *>(ptr));
        case 2:
            return func(static_cast<T2 *>(ptr));
        default:
            return func(static_cast<T3 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2, typename T3, typename T4>
    auto Dispatch(F &&func, void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<T0 *>(ptr));
        case 1:
            return func(static_cast<T1 *>(ptr));
        case 2:
            return func(static_cast<T2 *>(ptr));
        case 3:
            return func(static_cast<T3 *>(ptr));
        default:
            return func(static_cast<T4 *>(ptr));
        }
    }

    template<typename F, typename T0>
    auto DispatchConst(F &&func, const void *ptr, uint8_t tag) {
        return func(static_cast<const T0 *>(ptr));
    }

    template<typename F, typename T0, typename T1>
    auto DispatchConst(F &&func, const void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<const T0 *>(ptr));
        default:
            return func(static_cast<const T1 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2>
    auto DispatchConst(F &&func, const void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<const T0 *>(ptr));
        case 1:
            return func(static_cast<const T1 *>(ptr));
        default:
            return func(static_cast<const T2 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2, typename T3>
    auto DispatchConst(F &&func, const void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<const T0 *>(ptr));
        case 1:
            return func(static_cast<const T1 *>(ptr));
        case 2:
            return func(static_cast<const T2 *>(ptr));
        default:
            return func(static_cast<const T3 *>(ptr));
        }
    }

    template<typename F, typename T0, typename T1, typename T2, typename T3, typename T4>
    auto DispatchConst(F &&func, const void *ptr, uint8_t tag) {
        switch (tag) {
        case 0:
            return func(static_cast<const T0 *>(ptr));
        case 1:
            return func(static_cast<const T1 *>(ptr));
        case 2:
            return func(static_cast<const T2 *>(ptr));
        case 3:
            return func(static_cast<const T3 *>(ptr));
        default:
            return func(static_cast<const T4 *>(ptr));
        }
    }
}

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
    template <typename Func>
    auto Dispatch(Func &&func) {
        DEBUG_LINE(assert(!pointer.isConst()))
        return internal::Dispatch<Func, Ts...>(std::move(func), reinterpret_cast<void *>(pointer.getPtr()), pointer.getTag());
    }

    template <typename Func>
    auto DispatchConst(Func &&func) const {
        return internal::DispatchConst<Func, Ts...>(std::move(func), reinterpret_cast<const void *>(pointer.getPtr()), pointer.getTag());
    }
};

#define DISPATCH(func_name, ...) Dispatch([&](auto *ptr) { return ptr->func_name(__VA_ARGS__); })
#define DISPATCH_CONST(func_name, ...) DispatchConst([&](const auto *ptr) { return ptr->func_name(__VA_ARGS__); })
