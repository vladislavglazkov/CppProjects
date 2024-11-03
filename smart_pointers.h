#include <vector>
#include <algorithm>



struct DefaultDeleter {
    template <typename U>
    void operator() (U* ptr) {
        delete ptr;
    }
};

struct make_fun_t {

};
make_fun_t make_fun_tag;

template <typename U>
using DefaultAllocator = std::allocator<U>;



struct ControlBlock {
    size_t strongSize = 0;
    size_t weakSize = 0;


    virtual ~ControlBlock() {

    }
    ControlBlock(size_t strong, size_t weak) :strongSize(strong), weakSize(weak) {

    }

    virtual void* getPtr() = 0;

    virtual void removeRef(int strong, int weak, void* ptr = 0) = 0;
};


template <typename U, typename Alloc>
struct MakeControlBlock : public ControlBlock {

    alignas(U) std::byte obj[sizeof(U)];
    Alloc alloc;

    void* getPtr() override {
        return &obj;
    }

    template <typename... Args>
    MakeControlBlock(size_t strong, size_t weak, Alloc alloc, Args&&... args) : ControlBlock(strong, weak), alloc(alloc) {
        std::allocator_traits<Alloc>::construct(alloc, reinterpret_cast<U*>(obj), std::forward<Args>(args)...);
    }

    void removeRef(int strong, int weak, void*) override {

        weakSize -= weak;
        if (strongSize - strong == 0 && strong != 0) {

            std::allocator_traits<Alloc>::destroy(alloc, reinterpret_cast<U*>(&obj));

        }
        strongSize -= strong;

        if (weakSize + strongSize == 0) {

            using MyAlloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<MakeControlBlock>;
            MyAlloc tempAlloc = MyAlloc(alloc);
            std::allocator_traits<MyAlloc>::deallocate(tempAlloc, this, 1);

        }


    }


};

template <typename U, typename Deleter, typename Alloc>
struct RegularControlBlock :public ControlBlock {
    Deleter del;
    Alloc alloc;

    void* getPtr() override {
        return 0;
    }


    RegularControlBlock(int strong, int weak, Deleter del, Alloc alloc) :ControlBlock(strong, weak), del(del), alloc(alloc) {

    }

    void removeRef(int strong, int weak, void* ptr = 0) override {
        weakSize -= weak;

        if (strongSize - strong == 0 && strong != 0) {

            del(static_cast<U*>(ptr));
        }
        strongSize -= strong;

        if (weakSize + strongSize == 0) {
            using MyAlloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<RegularControlBlock>;
            MyAlloc tempAlloc = MyAlloc(alloc);
            std::allocator_traits<MyAlloc>::deallocate(tempAlloc, this, 1);
        }
    }

};



template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {

    template <typename U>
    friend class SharedPtr;


    template<typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(Alloc alloc, Args&&... args);

    template<typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&... args);

    ControlBlock* block;
    T* ptr;
    bool fromMake;


    T* getPtr()const {
        if (!fromMake) {
            return ptr;
        }

        return (static_cast<T*>(block->getPtr()));

    }



    ControlBlock* getControlBlock() const {
        return block;
    }

    void removeRef() {
        if (block) {
            block->removeRef(1, 0, ptr);
        }
    }

    void addOne() {
        if (block) {
            block->strongSize++;
        }
    }

    template<typename Alloc, typename ...Args>
    SharedPtr(make_fun_t, Alloc alloc, Args&&... args) :block(0), ptr(0), fromMake(1) {
        using MyAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<MakeControlBlock<T, Alloc> >;

        MyAlloc tempAlloc(alloc);
        block = std::allocator_traits<MyAlloc>::allocate(tempAlloc, 1);
        new(block) MakeControlBlock<T, Alloc>(1, 0, tempAlloc, std::forward<Args>(args)...);

        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            reinterpret_cast<T&>((reinterpret_cast<MakeControlBlock<T, Alloc> *> (block))->obj).wptr = WeakPtr<T>(*this);
        }

    }

public:

    SharedPtr() :block(0), ptr(0), fromMake(0) {

    }

    template <typename Y, typename Deleter = DefaultDeleter, typename Allocator = std::allocator<Y> >
    SharedPtr(Y* value, Deleter del = Deleter(), Allocator alloc = Allocator()) : block(0), ptr(value), fromMake(0) {
        if (value != 0) {

            using MyAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<RegularControlBlock<Y, Deleter, Allocator> >;
            MyAlloc all(alloc);
            block = std::allocator_traits<MyAlloc>::allocate(all, 1);
            new (block) RegularControlBlock<Y, Deleter, Allocator>(1, 0, del, alloc);

            if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
                value->wptr = WeakPtr<Y>(*this);
            }
        }
    }


    SharedPtr(const SharedPtr<T>& from) : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        addOne();
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& from) : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        addOne();
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& from, T* alias) : block(from.block), ptr(alias), fromMake(from.fromMake) {
        addOne();
    }


    template <typename Y>
    SharedPtr(SharedPtr<Y>&& from, T* alias) noexcept : block(from.block), ptr(alias), fromMake(from.fromMake) {
        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;

    }
    SharedPtr(SharedPtr<T>&& from) noexcept : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& from) noexcept : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
    }

    SharedPtr& operator=(const SharedPtr& from) {
        if (this == &from)
            return *this;

        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;
        addOne();
        return *this;
    }

    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& from) {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<SharedPtr<U>*>(&from)))
            return *this;

        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;
        addOne();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& from) noexcept {
        if (this == (&from))
            return *this;

        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;

        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
        return *this;
    }

    template<typename U>
    SharedPtr& operator=(SharedPtr<U>&& from) noexcept {
        if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(const_cast<SharedPtr<U>*>(&from)))
            return *this;
        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;

        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
        return *this;
    }






    T& operator* ()const {
        return *getPtr();
    }


    T* operator-> ()const {
        return getPtr();
    }

    T* get()const {
        return getPtr();
    }

    void swap(SharedPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(block, other.block);
        std::swap(fromMake, other.fromMake);
    }

    size_t use_count()const {
        if (!block) {
            return 0;
        }
        return getControlBlock()->strongSize;
    }

    template< typename Y = T, typename Deleter = DefaultDeleter, typename Alloc = std::allocator<Y> >
    void reset(Y* ptr = NULL, Deleter d = Deleter(), Alloc alloc = Alloc()) {

        removeRef();

        fromMake = false;
        this->ptr = ptr;
        block = 0;
        if (ptr != 0) {
            using MyAlloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<RegularControlBlock<Y, Deleter, Alloc> >;
            MyAlloc all(alloc);
            block = std::allocator_traits<MyAlloc>::allocate(all, 1);
            new (block) RegularControlBlock<Y, Deleter, Alloc>(1, 0, d, alloc);
        }
    }

    ~SharedPtr()
    {
        removeRef();
    }

    template <typename U>
    friend class WeakPtr;

};


template <typename T>
class WeakPtr {
    template <typename U>
    friend class WeakPtr;

    ControlBlock* block;
    T* ptr;
    bool fromMake;

    void removeRef() {
        if (block) {
            block->removeRef(0, 1, ptr);
        }
    }

    void addOne() {
        if (block) {
            block->weakSize++;
        }
    }

    ControlBlock* getControlBlock() const {
        return block;
    }
public:


    WeakPtr() :block(0), ptr(0), fromMake(0) {
    }

    ~WeakPtr() {
        removeRef();
    }
    template <typename U>
    WeakPtr(const SharedPtr<U>& from) :block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        addOne();
    }

    WeakPtr(const WeakPtr& from) :block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        addOne();
    }

    template <typename U>
    WeakPtr(const WeakPtr<U>& from) : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        addOne();
    }


    WeakPtr(WeakPtr&& from) :block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
    }

    template <typename U>
    WeakPtr(WeakPtr<U>&& from) : block(from.block), ptr(from.ptr), fromMake(from.fromMake) {
        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
    }


    WeakPtr& operator=(const WeakPtr& from) {
        if ((void*)this == (void*)(&from))
            return *this;
        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;
        addOne();
        return *this;
    }

    template<typename U>
    WeakPtr& operator=(const WeakPtr<U>& from) {
        if ((void*)this == (void*)(&from))
            return *this;
        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;
        addOne();
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& from) {
        if ((void*)this == (void*)(&from))
            return *this;
        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;

        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
        return *this;
    }

    template<typename U>
    WeakPtr& operator=(WeakPtr<U>&& from) {
        if ((void*)this == (void*)(&from))
            return *this;
        removeRef();

        block = from.block;
        ptr = from.ptr;
        fromMake = from.fromMake;

        from.block = 0;
        from.ptr = 0;
        from.fromMake = false;
        return *this;
    }

    void swap(WeakPtr& other) {
        std::swap(ptr, other.ptr);
        std::swap(block, other.block);
        std::swap(fromMake, other.fromMake);
    }

    void reset() {

        removeRef();


        fromMake = false;
        this->ptr = 0;
        block = 0;

    }

    size_t use_count()const {
        if (!block) {
            return 0;
        }
        return getControlBlock()->strongSize;
    }

    bool expired()const {
        return use_count() == 0;
    }

    SharedPtr<T> lock()const {
        SharedPtr<T> res;
        if (!expired()) {
            res.block = block;
            res.ptr = ptr;
            res.fromMake = fromMake;
            res.addOne();
        }
        return res;
    }

    template <typename U>
    friend class EnableSharedFromThis;
};

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return SharedPtr<T>(make_fun_tag, std::allocator<T>(), std::forward<Args>(args)...);
}

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(Alloc alloc, Args&&... args) {
    return SharedPtr<T>(make_fun_tag, alloc, std::forward<Args>(args)...);
}


template <typename T>
class EnableSharedFromThis {

    template <typename U>
    friend class SharedPtr;


    WeakPtr<T> wptr;
protected:
    SharedPtr<T> shared_from_this() {
        if (wptr.block == 0) {
            throw std::bad_weak_ptr();
        }
        return wptr.lock();
    }
};