#pragma once
#include <concepts>
#include <type_traits>

template <typename T>
class Debugg {
    Debugg() = delete;
};

template <typename... T>
struct Tuple;



template <typename T>
concept DefaultConstructible = std::is_default_constructible_v<T>;

template <typename... Ts>
struct are_all_distinct;

template <>
struct are_all_distinct<> {
    static constexpr bool value = true;
};

template <typename Head,typename... Tail>
struct are_all_distinct<Head,Tail...> {
    static constexpr bool value = are_all_distinct<Tail...>::value&&!(std::is_same_v<Head,Tail>||...);
};

template <typename... Ts>
using are_all_distinct_v = are_all_distinct<Ts...>::value;

template <typename... Ts> 
concept AllDistinct = are_all_distinct<Ts...>::value;

template <typename T>
concept EmptyListInitializable = requires{
    T{};
};


struct tuple_cat_t {

} tuple_cat_tag;

struct tuple_cat_t2 {
    
} tuple_cat_tag2;



template <typename T>
struct type_wrapper {
    using type = T;
};


template <typename... Ts>
struct get_first;

template <typename First,typename... Rest>
struct get_first<First,Rest...> {
    using type = First;
};


template <>
struct Tuple<> {
    template<typename... T>
    friend class Tuple;

public:

    static constexpr size_t size = 0;

    bool operator==(const Tuple&)const {
        return true;
    }

    std::weak_ordering operator<=>(const Tuple&)const {
        return std::weak_ordering::equivalent;
    }

    Tuple() {

    }
};


template<typename Head, typename... Tail>
struct Tuple<Head, Tail...> {

    auto operator<=>(const Tuple&) const = default;
    bool operator==(const Tuple&) const  = default;

    template<typename... T>
    friend class Tuple;

    template <size_t Index, typename... Ts>
    friend decltype(auto) get(const Tuple<Ts...>& tuple);

    template <size_t Index, typename... Ts>
    friend decltype(auto) get(Tuple<Ts...>& tuple);

    template <size_t Index, typename... Ts>
    friend decltype(auto) get(Tuple<Ts...>&& tuple);

    template <typename T, typename... Ts>
    friend decltype(auto) get(const Tuple<Ts...>& tuple);

    template <typename T, typename... Ts>
    friend decltype(auto) get(Tuple<Ts...>& tuple);

    template <typename T, typename... Ts>
    friend decltype(auto) get(Tuple<Ts...>&& tuple);

    template <size_t Index, typename... Ts>
    friend decltype(auto) get(const Tuple<Ts...>&& tuple);

    template <typename T, typename... Ts>
    friend decltype(auto) get(const Tuple<Ts...>&& tuple);
    
    template<typename... Tuples>
    friend decltype(auto) tupleCatHelper(const Tuple<>& t1, Tuples&&... t2s);
    template<typename... Tuples>
    friend decltype(auto) tupleCatHelper(Tuple<>&& t1, Tuples&&... t2s);
    template<typename... Ts, typename... Tuples>
    friend decltype(auto) tupleCatHelper(const Tuple<Ts...>& t1, Tuples&&... t2s);
    template<typename... Ts, typename... Tuples>
    friend decltype(auto) tupleCatHelper(Tuple<Ts...>&& t1, Tuples&&... t2s);



    Head data;
    Tuple<Tail...> tail;
    static constexpr size_t size = Tuple<Tail...>::size;

    

    
    Tuple(tuple_cat_t,type_wrapper<Head>,Head head,Tuple<Tail...> tail) :data(std::move(head)), tail(std::move(tail)) {

    }

    Tuple(tuple_cat_t2, type_wrapper<Head>, Head head, Tuple<Tail...> tail) :data(std::move(head)), tail(std::move(tail)) {

    }

    template<size_t Index>
    auto&& getnth() && {
        if constexpr (Index == 0) {
            return std::move(data);
            
        }
        else {
            return std::move(tail).template getnth<Index - 1>();
        }
    }

    template<size_t Index>
    auto& getnth() & {
        if constexpr (Index == 0) {
            return data;
        }
        else {

            return tail.template getnth<Index - 1>();
        }
    }



    template<size_t Index>
    auto& getnth() const& {
        if constexpr (Index == 0) {
            return data;
        }
        else{

            return tail.template getnth<Index - 1>();
        }
    }

    template<size_t Index>
    auto&& getnth() const&& {
        if constexpr (Index == 0) {
            return std::move(data);
        }
        else {

            return std::move(tail).template getnth<Index - 1>();
        }
    }




    template<typename T>
    auto& gettype() const& requires AllDistinct<Head, Tail...> {
        if constexpr (std::is_same_v<T, Head>) {
            return data;
        }
        else {

            return tail.template gettype<T>();
        }
    }

    template<typename T>
    auto& gettype() & requires AllDistinct<Head, Tail...> {
        if constexpr (std::is_same_v<T, Head>) {
            return data;
        }
        else {
            return tail.template gettype<T>();
        }
    }

    template<typename T>
    auto&& gettype() && requires AllDistinct<Head, Tail...> {
        if constexpr (std::is_same_v<T, Head>) {
            return std::forward<Head>(data);
        }
        else {
            return (std::move(tail)).template gettype<T>();
        }
    }

    template<typename T>
    auto&& gettype() const && requires AllDistinct<Head, Tail...> {
        if constexpr (std::is_same_v<T, Head>) {
            return std::forward<const Head>(data);
        }
        else {
            return std::move(tail).template gettype<T>();
        }
    }



public:
    using HeadType = Head;
    
    explicit (!EmptyListInitializable<Head>||((!EmptyListInitializable<Tail>)||...)) Tuple()
        requires DefaultConstructible<Head> && (DefaultConstructible<Tail>&&...) 
        : data(),tail()
    {
        
        
    }


    explicit(!std::is_convertible_v<const Head&,Head>||((!std::is_convertible_v<const Tail&,Tail>)||...)) Tuple(const Head& head , const Tail&... tail ) requires(std::is_copy_constructible_v<Head>&&(std::is_copy_constructible_v<Tail>&&...)) : data(head),tail(tail...) {

    }

    template <typename UHead, typename... UTail>
    explicit(!std::is_convertible_v<UHead,Head>||((!std::is_convertible_v<UTail,Tail>)||...))
    Tuple(UHead&& head, UTail&&... tail) requires(sizeof... (Tail) == sizeof...(UTail) && std::is_constructible_v<Head, UHead&&> && (std::is_constructible_v<Tail, UTail&&>&&...)) : data(std::forward<UHead>(head)), tail(std::forward<UTail>(tail)...) {
    }


    template<typename UHead, typename... UTail>
    explicit(!std::is_convertible_v<UHead, Head> || ((!std::is_convertible_v<UTail, Tail>) || ...))
    Tuple(const Tuple<UHead,UTail...> &other)
    requires(sizeof...(Tail)==sizeof...(UTail)&&std::is_constructible_v<Head,const UHead&>&&(std::is_constructible_v<Tail,const UTail&>&&...)&&
    (sizeof...(Tail) > 0 || (!std::is_convertible_v<decltype(other), Head> && !std::is_constructible_v<Head, decltype(other)> && !std::is_same_v<Head, UHead>)))

        : data(other.data), tail(other.tail)
    {
    }

    template<typename UHead, typename... UTail>
    explicit(!std::is_convertible_v<UHead, Head> || ((!std::is_convertible_v<UTail, Tail>) || ...))
    Tuple(Tuple<UHead, UTail...>&& other)
        requires(sizeof...(Tail) == sizeof...(UTail) && std::is_constructible_v<Head, UHead&&> && (std::is_constructible_v<Tail, UTail&&>&&...) &&
    (sizeof...(Tail) > 0 || (!std::is_convertible_v<decltype(other), Head> && !std::is_constructible_v<Head, decltype(other)> && !std::is_same_v<Head, UHead>)))
    : data(std::forward<UHead>(other.data)),tail(std::move(other.tail))
    {
    }



    Tuple(const Tuple& other) = default;

    Tuple(Tuple&& other) :data(std::move(other.data)), tail(std::move(other.tail)) {
        
    }
    
    template <typename U,typename V>
    Tuple(const std::pair<U, V>& other) :data(other.first), tail(Tuple<V>(other.second)) {

    }

    template <typename U, typename V>
    Tuple(std::pair<U, V>&& other) : data(std::forward<U>(other.first)), tail(Tuple<V>(std::forward<V>(other.second))) {

    }

    Tuple& operator= (const Tuple& other) requires(std::is_copy_assignable_v<Head> && (std::is_copy_assignable_v<Tail>&&...))
    {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<Tuple*>(&other))) {
            return *this;
        }
        data = other.data;
        
        tail = other.tail;
        return *this;

    }

    Tuple& operator= (Tuple&& other) requires(std::is_move_assignable_v<Head> && (std::is_move_assignable_v<Tail>&&...))
    {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<Tuple*>(&other))) {
            return *this;
        }
        data = std::move(other.data);

        tail = std::move(other.tail);
        return *this;
    }


    template<typename UHead,typename... UTail>
    Tuple& operator=(const Tuple<UHead,UTail...>& other)
        requires(std::is_assignable_v<Head&, const UHead&> && (std::is_assignable_v<Tail&, const UTail&>&&...))
    {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<Tuple<UHead, UTail...>*>(&other))) {
            return *this;
        }
        data = other.data;

        tail = other.tail;
        return *this;

    }

    template<typename UHead, typename... UTail>
    Tuple& operator=(Tuple<UHead, UTail...>&& other)
        requires(std::is_assignable_v<Head&, UHead&&> && (std::is_assignable_v<Tail&, UTail&&>&&...))
    {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<Tuple<UHead, UTail...>*>(&other))) {
            return *this;
        }
        data = std::forward<UHead>(other.data);

        tail = std::move(other.tail);
        return *this;

    }
};

template <size_t Index, typename... Ts>
decltype(auto) get(const Tuple<Ts...> &tuple) {
    return tuple.template getnth<Index>();
}

template <size_t Index, typename... Ts>
decltype(auto) get(Tuple<Ts...>& tuple) {
    return tuple.template getnth<Index>();
}

template <size_t Index, typename... Ts>
decltype(auto) get(Tuple<Ts...>&& tuple) {
    return (std::move(tuple)).template getnth<Index>();
}

template <size_t Index, typename... Ts>
decltype(auto) get(const Tuple<Ts...>&& tuple) {
    return (std::move(tuple)).template getnth<Index>();
}




template <typename T, typename... Ts>
decltype(auto) get(const Tuple<Ts...>& tuple) {
    return tuple.template gettype<T>();
}

template <typename T, typename... Ts>
decltype(auto) get(Tuple<Ts...>& tuple) {
    return tuple.template gettype<T>();
}

template <typename T, typename... Ts>
decltype(auto) get(Tuple<Ts...>&& tuple) {
    return (std::move(tuple)).template gettype<T>();
}

template <typename T, typename... Ts>
decltype(auto) get(const Tuple<Ts...>&& tuple) {
    return (std::move(tuple)).template gettype<T>();
}

template <class T>
struct unwrap_refwrapper
{
    using type = T;
};

template <class T>
struct unwrap_refwrapper<std::reference_wrapper<T>>
{
    using type = T&;
};

template <class T>
using unwrap_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;


template<typename... Ts>
decltype(auto) makeTuple(Ts... args) {
    return Tuple<unwrap_decay_t<Ts>...>(std::forward<Ts>(args)...);

}

template<typename... Ts>
decltype(auto) tie(Ts&... args) {
    return Tuple<Ts&...>(args...);
}

template<typename... Ts>
decltype(auto) forwardAsTuple(Ts&&... args) {
    return Tuple<Ts&&...>(std::forward<Ts>(args)...);
}




template<typename Head, typename... Ts>
Tuple(tuple_cat_t, type_wrapper<Head>, Head, Tuple<Ts...>) -> Tuple<Head, Ts...>;

//template<typename Head, typename... Ts>
//Tuple(tuple_cat_t, type_wrapper<Head&>, Head&, Tuple<Ts...>) -> Tuple<Head&, Ts...>;




template<typename Head, typename... Ts>
Tuple(tuple_cat_t2, type_wrapper<Head>, Head&&, Tuple<Ts...>) -> Tuple<Head, Ts...>;

template<typename Head, typename... Ts>
Tuple(tuple_cat_t2, type_wrapper<Head>, const Head&, Tuple<Ts...>) -> Tuple<Head, Ts...>;

template<typename Head, typename... Ts>
Tuple(tuple_cat_t2, type_wrapper<Head&&>, Head&&, Tuple<Ts...>) -> Tuple<Head&&, Ts...>;

template <typename U, typename V>
Tuple(const std::pair<U, V>&) -> Tuple<U, V>;

template <typename U, typename V>
Tuple(std::pair<U, V>&&) -> Tuple<U, V>;


template<typename... Tuples>
decltype(auto) tupleCatHelper(const Tuple<>& t1, Tuples&&... t2s);

template<typename... Tuples>
decltype(auto) tupleCatHelper(Tuple<>&& t1, Tuples&&... t2s);

template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper(const Tuple<Ts...>& t1, Tuples&&... t2s);

template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper(Tuple<Ts...>&& t1, Tuples&&... t2s);




template<typename... Tuples>
decltype(auto) tupleCatHelper(Tuple<>&&, Tuples&&... t2s) {
    if constexpr (sizeof...(t2s) == 0) {
        return Tuple<>();
    }
    if constexpr (sizeof...(t2s) != 0) {
        return tupleCatHelper(std::forward<Tuples&&>(t2s)...);
    }
}
template<typename... Tuples>
decltype(auto) tupleCatHelper(const Tuple<>&, Tuples&&... t2s) {
    if constexpr (sizeof...(t2s) == 0) {
        return Tuple<>();
    }
    if constexpr (sizeof...(t2s) != 0) {
        return tupleCatHelper(std::forward<Tuples&&>(t2s)...);
    }
}



template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper(const Tuple<Ts...>& t1, Tuples&&... t2s) {
    using HeadType = Tuple<Ts...>::HeadType;
    return Tuple(tuple_cat_tag,type_wrapper<const HeadType&>(), (get<0>(t1)), tupleCatHelper(t1.tail, std::forward<Tuples>(t2s)...));
}



template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper(Tuple<Ts...>&& t1, Tuples&&... t2s) {
    using HeadType = Tuple<Ts...>::HeadType;

    return Tuple(tuple_cat_tag, type_wrapper<HeadType&&>(), std::move(get<0>(t1)), tupleCatHelper(std::move(t1.tail), std::forward<Tuples>(t2s)...));
}




template<typename... Tuples>
decltype(auto) tupleCatHelper2(Tuple<>&&, Tuples&&... t2s) {
    if constexpr (sizeof...(t2s) == 0) {
        return Tuple<>();
    }
    if constexpr (sizeof...(t2s) != 0) {
        return tupleCatHelper2(std::forward<Tuples&&>(t2s)...);
    }
}
template<typename... Tuples>
decltype(auto) tupleCatHelper2(const Tuple<>&, Tuples&&... t2s) {
    if constexpr (sizeof...(t2s) == 0) {
        return Tuple<>();
    }
    if constexpr (sizeof...(t2s) != 0) {
        return tupleCatHelper2(std::forward<Tuples&&>(t2s)...);
    }
}



template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper2(const Tuple<Ts...>& t1, Tuples&&... t2s) {
    using HeadType = Tuple<Ts...>::HeadType;
    return Tuple(tuple_cat_tag2, type_wrapper<HeadType>(), (get<0>(t1)), tupleCatHelper2(t1.tail, std::forward<Tuples>(t2s)...));
}



template<typename... Ts, typename... Tuples>
decltype(auto) tupleCatHelper2(Tuple<Ts...>&& t1, Tuples&&... t2s) {
    using HeadType = Tuple<Ts...>::HeadType;
    return Tuple(tuple_cat_tag2, type_wrapper<HeadType>(), (get<0>(t1)), tupleCatHelper2(std::move(t1.tail), std::forward<Tuples>(t2s)...));
}


template<typename... Tuples>
decltype(auto) tupleCat(Tuples&&... ts) {
    auto temp = tupleCatHelper(std::forward<Tuples>(ts)...);
    using realType = decltype(tupleCatHelper2(std::forward<Tuples>(ts)...));
    return realType(std::move(temp));
}



