#pragma once
#include <vector>

template <typename T>
struct Debugg {
    Debugg() = delete;
};




template <typename T>
struct DiscardThis;

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const > {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile > {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)noexcept> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const noexcept> {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile noexcept> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile noexcept> {
    using type = Ret(Args...);
};





template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)&> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const & > {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile &> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile & > {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)& noexcept > {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...) const & noexcept> {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile & noexcept> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile & noexcept> {
    using type = Ret(Args...);
};






template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)&&> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const&& > {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile&&> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile&& > {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...) && noexcept > {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...) const&& noexcept> {
    using type = Ret(Args...);
};
template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)volatile&& noexcept> {
    using type = Ret(Args...);
};

template <typename Ret, typename T, typename... Args>
struct DiscardThis<Ret(T::*)(Args...)const volatile&& noexcept> {
    using type = Ret(Args...);
};



template <bool IsMoveOnly>
struct MCPtrs;

template<>
struct MCPtrs<false> {
    using moveType = void(*)(void*, void*);
    using copyType = void(*)(void*, const void*);
    using destroyType = void(*)(void*);

    moveType moveConstructorPtr;
    copyType copyConstructorPtr;
    destroyType destroyPtr;


};

template<>
struct MCPtrs<true> {
    using moveType = void(*)(void*, void*);
    using destroyType = void(*)(void*);

    moveType moveConstructorPtr;
    destroyType destroyPtr;


};




//template<bool IsMoveOnly, typename Func, typename U>
//static void* ptrStorage[3];


template <bool IsMoveOnly,typename T>
class BaseFunction;


template <bool IsMoveOnly,typename Ret,typename... Args>
class BaseFunction<IsMoveOnly,Ret(Args...)> {
public:

   
    bool small() {
        return ptr == &buffer;
    }

    static constexpr int maxBufferSize = 16;
    
    void* ptr=nullptr;
    alignas (std::max_align_t) mutable char buffer[maxBufferSize];
    
    template <typename T>
    static Ret invoker(void* fptr, Args... args) {
        return std::invoke(*reinterpret_cast<T*>(fptr), std::forward<Args>(args)...);
    }

    template <typename T>
    static void copyConstructor(void* my_, const void* other_) {
        const BaseFunction& other = *reinterpret_cast<const BaseFunction*>(other_);
        BaseFunction& my = *reinterpret_cast<BaseFunction*>(my_);
        void* ans = nullptr;
        if (other.buffer == other.ptr) {
            ans = my.buffer;
            new(ans) T(*reinterpret_cast<T*>(other.ptr));
        }
        else {
            ans = new T(*reinterpret_cast<T*>(other.ptr));
        }
        my.ptr = ans;


    }

    template <typename T>
    static void moveConstructor(void* to, void* from) {
        new(to) T(std::move(*reinterpret_cast<T*>(from)));
    }

    



    template <typename T>
    static void destroyer(void* fptr) {
        reinterpret_cast<T*>(fptr)->~T();
    }
    
    using invokeType = Ret(*)(void*, Args...);

    invokeType invokePtr;
    MCPtrs<IsMoveOnly> mcptrs;

    template<typename T>
    void initMCP() {
        mcptrs.moveConstructorPtr = (typename MCPtrs<IsMoveOnly>::moveType)(&moveConstructor<T>);
        mcptrs.destroyPtr = ((typename MCPtrs<IsMoveOnly>::destroyType) & destroyer<std::decay_t<T> >);
        if constexpr (!IsMoveOnly) {
            mcptrs.copyConstructorPtr = (typename MCPtrs<IsMoveOnly>::copyType)(&copyConstructor<T>);
        }

    }

    BaseFunction(const BaseFunction& other) :  invokePtr(other.invokePtr), mcptrs(other.mcptrs) {
        if (other.ptr!=nullptr) {
            mcptrs.copyConstructorPtr(this, &other);
        }
        
    }

    BaseFunction(BaseFunction&& other) : invokePtr(other.invokePtr), mcptrs(other.mcptrs) {
        if (other.ptr != nullptr) {

            if (other.ptr == other.buffer) {
                ptr = buffer;
                mcptrs.moveConstructorPtr(ptr, other.ptr);
                mcptrs.destroyPtr(other.ptr);
                other.ptr = nullptr;

            }
            else {
                ptr = other.ptr;
                other.ptr = nullptr;

            }

        }
    }


    BaseFunction() {

    }
    BaseFunction(nullptr_t) {

    }


    

    template<typename T>
    BaseFunction(T&& func) requires requires(T&& func,Args... args) { std::invoke(func, std::forward<Args>(args)...); } && (!std::is_same_v<std::decay_t<T>, BaseFunction>) :  invokePtr((invokeType)(&invoker<std::decay_t<T> >)) {
        using ActualT = std::decay_t<T>;
        initMCP<ActualT>();
        if constexpr (sizeof(ActualT) <= maxBufferSize) {
            ptr = buffer;
            new(buffer) ActualT(std::forward<T>(func));
        }
        else {
            ptr = new ActualT(std::forward<T>(func));
        }
    }

    



    
    

    Ret operator() (Args... args) const{
        if (ptr==nullptr) {
            throw std::bad_function_call();
        }
        return (*invokePtr)(ptr,std::forward<Args>(args)...);
    }
    

    void clear() {
        if (ptr == nullptr)
            return;
        (*mcptrs.destroyPtr)(ptr);
        if (ptr != buffer) {
            delete[] reinterpret_cast<char*>(ptr);
        }
    }

    ~BaseFunction() {
        clear();
    }


    BaseFunction& operator=(const BaseFunction& other) {
        if (this == &other) {
            return *this;
        }
        clear();

        invokePtr = other.invokePtr;
        mcptrs = other.mcptrs;

        if (other.ptr != nullptr) {
            mcptrs.copyConstructorPtr(this, &other);
        }
        else {
            ptr = nullptr;
        }
        return *this;
    }

    BaseFunction& operator=(BaseFunction&& other) {
        if (this == &other) {
            return *this;
        }

        clear();

        invokePtr = other.invokePtr;
        mcptrs = other.mcptrs;

        if (other.ptr != nullptr) {

            if (other.ptr == other.buffer) {
                ptr = buffer;
                mcptrs.moveConstructorPtr(ptr, other.ptr);
                mcptrs.destroyPtr(other.ptr);
                other.ptr = nullptr;

            }
            else {
                ptr = other.ptr;
                other.ptr = nullptr;

            }

        }
        else {
            ptr = nullptr;
        }
        return *this;
    }

    template<typename T>
    BaseFunction& operator=(T&& func) requires requires(T&& func, Args... args) { std::invoke(func, std::forward<Args>(args)...); } && (!std::is_same_v<std::decay_t<T>, BaseFunction>) {
        clear();

        invokePtr = ((invokeType)(&invoker<std::decay_t<T> >));
        //destroyPtr = ((destroyType)&destroyer<std::decay_t<T> >);

        using ActualT = std::decay_t<T>;
        initMCP<ActualT>();
        if constexpr (sizeof(ActualT) <= maxBufferSize) {
            ptr = buffer;
            new(buffer) ActualT(std::forward<T>(func));
        }
        else {
            ptr = new ActualT(std::forward<T>(func));
        }
        return *this;
    }

    operator bool() const {
        return ptr!=nullptr;
    }
    
    const std::type_info& target_type()const {
        return typeid(this);
    }

    using T = Ret(Args...);
    T* target() {
        return reinterpret_cast<T*>(target);
        
    }

    const T* target() const {
        return reinterpret_cast<T*>(target);

    }

};

//template<typename R, class... ArgTypes >
//bool operator==(std::nullptr_t,
//    const BaseFunction<R(ArgTypes...)>& f) {
//    return !((bool)f);
//}

template<bool IsMoveOnly,typename R, class... ArgTypes >
bool operator==(const BaseFunction<IsMoveOnly,R(ArgTypes...)>& f,std::nullptr_t) {
    return !bool(f);
}

//template <typename T>
//using BaseFunction = BaseFunction<false, T>;
//
//template <typename T>
//using MoveOnlyBaseFunction = BaseFunction<true, T>;

template<typename T>
class Function :public BaseFunction<false, T> {
    using Base = BaseFunction<false, T>;
    using Base::Base;
};

template<typename T>
class MoveOnlyFunction :public BaseFunction<true, T> {
    using Base = BaseFunction<true, T>;
    using Base::Base;
};

template< typename R, typename... ArgTypes >
Function(R(*)(ArgTypes...)) -> Function<R(ArgTypes...)>;

template<typename T>
//requires std::is_same_v<Ret(T::*)(Args...),decltype(&T::operator())>
Function(T) -> Function <typename DiscardThis<decltype(&T::operator())>::type>;


template< typename R, typename... ArgTypes >
MoveOnlyFunction(R(*)(ArgTypes...)) -> MoveOnlyFunction<R(ArgTypes...)>;

template<typename T>
//requires std::is_same_v<Ret(T::*)(Args...),decltype(&T::operator())>
MoveOnlyFunction(T) -> MoveOnlyFunction <typename DiscardThis<decltype(&T::operator())>::type>;
