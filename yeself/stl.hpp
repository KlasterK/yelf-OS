#ifndef YESELF_STL_HPP
#define YESELF_STL_HPP

#include <stdint.h>
#include <stddef.h>


template<typename T, typename U>
struct Pair
{
    T first;
    U second;
};


template<typename T>
constexpr T min(T a, T b) { return (a < b) ? a : b; }

template<typename T>
constexpr T max(T a, T b) { return (a > b) ? a : b; }

template<typename T>
constexpr T clamp(T x, T lower, T upper) { return max(lower, min(upper, x)); }

template<typename T>
constexpr T ceildiv(T numerator, T divisor) { return (numerator + divisor - 1) / divisor; }


template<typename T, typename U>
struct IsSame 
{
    static constexpr bool Value = false;
};

template<typename T>
struct IsSame<T, T> 
{
    static constexpr bool Value = true;
};

template<typename T, typename... Ts>
struct IsOneOf
{
    static constexpr bool Value = (IsSame<T, Ts>::Value || ...);
};


template<typename T, typename First, typename... Rest>
constexpr int GetTypeIdx(int index = 0) 
{
    if constexpr (IsSame<T, First>::Value)
        return index;
    else if constexpr (sizeof...(Rest) == 0)
        return -1;
    else
        return GetTypeIdx<T, Rest...>(index + 1);
}

template<size_t Idx, typename... Ts>
struct TypeAtIdx {};

template<typename T, typename... Ts>
struct TypeAtIdx<0, T, Ts...> 
{
    using Type = T;
};

template<size_t Idx, typename T, typename... Ts>
struct TypeAtIdx<Idx, T, Ts...> 
{
    using Type = typename TypeAtIdx<Idx - 1, Ts...>::Type;
};


template<typename... Ts>
constexpr size_t GetMaxSizeof()
{
    size_t n{};
    ((n = max(n, sizeof(Ts))), ...);
    return n;
}

template<typename... Ts>
constexpr size_t GetMaxAlignof()
{
    size_t n{};
    ((n = max(n, alignof(Ts))), ...);
    return n;
}


struct Monostate {};

template<typename T> 
struct RemoveReference { using Type = T; };
template<typename T> 
struct RemoveReference<T &> { using Type = T; };
template<typename T> 
struct RemoveReference<T &&> { using Type = T; };

template<typename T>
struct IsLvalueReference { static constexpr bool Value = false; };
template<typename T>
struct IsLvalueReference<T &> { static constexpr bool Value = true; };

template<typename T>
typename RemoveReference<T>::Type Move(T &&v)
{
    return static_cast<typename RemoveReference<T>::Type &&>(v);
}

template<typename T>
constexpr T &&Forward(typename RemoveReference<T>::Type &v) 
{ 
    return static_cast<T &&>(v); 
}
template<typename T>
constexpr T &&Forward(typename RemoveReference<T>::Type &&v) 
{ 
    static_assert(!IsLvalueReference<T>::Value); 
    return static_cast<T &&>(v); 
}


template<typename... Ts>
class RefVariant
{
public:
    template<typename T>
    RefVariant(T &value) 
        : m_value(&value)
        , m_type_idx(get_type_idx<T>())
    {
        static_assert(IsOneOf<T, Ts...>::Value);
    }

    template<typename T>
    bool is() const
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        return m_type_idx == GetTypeIdx<T, Ts...>();
    }

    template<typename T>
    T *get_if() const
    {
        if(is<T>())
            return static_cast<T *>(m_value);
        return nullptr;
    }

    template<typename T>
    T &get_unchecked() const
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        return *static_cast<T *>(m_value);
    }

private:
    void *m_value;
    int m_type_idx;
};


template<typename T>
class ScopedRef
{
public:
    inline ScopedRef(T &instance, void (*deleter)(T &)) 
        : m_instance(instance), m_deleter(deleter) {}
    inline ~ScopedRef() { m_deleter(m_instance); }
    inline T &operator*() { return m_instance; }

private:
    T &m_instance;
    void (*m_deleter)(T &);
};


template<typename... Ts>
class Variant 
{
public:
    template<typename T>
    Variant(T &&instance) : m_type_idx(GetTypeIdx<T, Ts...>())
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        *reinterpret_cast<T *>(m_storage) = Forward<T>(instance);
    }

    ~Variant()
    {
        DeletersTable[m_type_idx](m_storage);
    }

    template<typename T>
    bool is() const
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        return GetTypeIdx<T, Ts...>() == m_type_idx;
    }

    template<typename T>
    T *get_if()
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        return GetTypeIdx<T, Ts...>() == m_type_idx 
            ? reinterpret_cast<T *>(&m_storage) 
            : nullptr;
    }

    template<typename T>
    T *get_if() const
    {
        static_assert(IsOneOf<T, Ts...>::Value);
        return GetTypeIdx<T, Ts...>() == m_type_idx 
            ? reinterpret_cast<const T *>(&m_storage) 
            : nullptr;
    }

private:
    alignas(GetMaxAlignof<Ts...>()) uint8_t m_storage[GetMaxSizeof<Ts...>()];
    int m_type_idx;

    static constexpr void (*DeletersTable[]) (void *) = {
        +[](void *p) { static_cast<Ts *>(p)->~Ts(); } ...
    };
};


#endif // YESELF_STL_HPP
