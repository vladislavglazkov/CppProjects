

#pragma once
#include <vector>
#include <algorithm>
#include <memory>


template <size_t N>
class StackStorage {

public:
    char data[N];
    size_t cpos;

    StackStorage() :cpos(0) {

    }
    StackStorage(const StackStorage& from) = delete;
    StackStorage& operator=(const StackStorage& from) = delete;

    template<typename T>
    void* allocate(size_t sz) {
        if ((size_t)(data + cpos) % alignof(T) != 0) {
            cpos += (alignof(T) - ((size_t)(data + cpos) % alignof(T)));
        }

        cpos += sz;
        return reinterpret_cast<void*>(&data[cpos - sz]);
    }
    bool operator==(const StackStorage& other)const {
        return data == other.data;
    }


};
template <typename T, size_t N>
class StackAllocator {

    template<typename U, size_t M>
    friend class StackAllocator;

private:
    StackStorage<N>* storage;

public:

    using value_type = T;
    using pointer = T*;
    using difference_type = long long;

    template<typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackAllocator(StackStorage<N>& storage) :storage(&storage) {

    }

    template<typename U>
    StackAllocator(const StackAllocator<U, N>& from) : storage(from.storage) {

    }

    template<typename U>
    StackAllocator& operator=(const StackAllocator<U, N>& from) {
        storage = from.storage;
    }
    ~StackAllocator() {

    }

    [[nodiscard]] T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(storage->template allocate<T>(n * sizeof(T)));
    }

    static constexpr StackAllocator select_on_container_copy_construction(const StackAllocator& alloc) {
        return alloc;
    }

    void deallocate(T*, size_t) {

    }

};


template <typename T, typename Allocator = std::allocator<T> >
class List {
private:


    class BaseNode {
    public:
        BaseNode* prev, * next;

        BaseNode() :prev(this), next(this) {

        }

    };

    class Node : public BaseNode {
    public:
        T value;
        Node(const T& value) :value(value) {

        }
        Node() :value() {

        }
    };

    using NodeAllocator = typename std::template allocator_traits<Allocator>::template rebind_alloc<Node>;
    [[no_unique_address]] NodeAllocator nodeAlloc;
    BaseNode root;
    size_t sz = 0;

    template<bool isConst>
    class BaseIterator {
        friend class List;


    private:
        using nodeptrtype = std::conditional_t<isConst, const BaseNode*, BaseNode*>;
        nodeptrtype node;
    public:

        BaseIterator(nodeptrtype node) :node(node) {

        }

        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;
        using value_type = T;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = long long;


        BaseIterator& operator++() {
            node = node->next;
            return *this;
        }
        BaseIterator& operator--() {
            node = node->prev;
            return *this;
        }
        BaseIterator operator++(int) {
            BaseIterator res = *this;
            node = node->next;
            return res;
        }
        BaseIterator operator--(int) {
            BaseIterator res = *this;
            node = node->prev;
            return res;
        }

        bool operator==(const BaseIterator& other)const {
            return node == other.node;
        }
        bool operator!=(const BaseIterator& other)const {
            return !(*this == other);
        }

        reference operator*()const {
            return ((reinterpret_cast<Node*>(const_cast<BaseNode*>(node)))->value);
        }

        pointer operator->()const {
          return &((reinterpret_cast<Node*>(const_cast <BaseNode*>(node)))->value);
        }


        operator BaseIterator<true>()const {
            return BaseIterator<true>(node);
        }
    };

public:
    using iterator = BaseIterator<false>;
    using const_iterator = BaseIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(root.next);
    }

    const_iterator begin()const {
        return const_iterator(root.next);
    }
    const_iterator cbegin() const {
        return const_iterator(root.next);

    }


    std::reverse_iterator<iterator> rbegin() {
        return std::reverse_iterator<iterator>(end());
    }

    std::reverse_iterator<const_iterator> rbegin() const {
        return std::reverse_iterator<const_iterator>(end());
    }
    std::reverse_iterator<const_iterator> crbegin() const {
        return std::reverse_iterator<const_iterator>(cend());
    }


    iterator end() {
        return iterator(&root);
    }

    const_iterator end() const {
        return const_iterator(&root);
    }
    const_iterator cend() const {
        return const_iterator(&root);
    }


    std::reverse_iterator<iterator> rend() {
        return std::reverse_iterator<iterator>(begin());
    }

    std::reverse_iterator<const_iterator> rend() const {
        return std::reverse_iterator<const_iterator>(begin());
    }
    std::reverse_iterator<const_iterator> crend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }


    void destruct()noexcept {
        while (sz > 0) {
            pop_back();
        }
    }

    List(Allocator alloc = Allocator()) :nodeAlloc(NodeAllocator(alloc)), root(), sz(0) {}

    List(size_t n, Allocator alloc = Allocator()) : List(NodeAllocator(alloc)) {
        for (size_t i = 0; i < n; i++) {
            insert(cend());
        }
    }



    List(size_t n, const T& val, Allocator alloc = Allocator()) :List(NodeAllocator(alloc)) {
        for (size_t i = 0; i < n; i++) {
            push_back(val);
        }
    }



    ~List() {
        destruct();
    }



    List(const List& other) :List(NodeAllocator(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.nodeAlloc))) {
        for (auto& h : other) {
            push_back(h);
        }
    }

    void swap(List& other) noexcept {
        std::swap(root, other.root);
        std::swap(sz, other.sz);
        std::swap(nodeAlloc, other.nodeAlloc);

        if (sz != 0) {
            root.next->prev = &root;
            root.prev->next = &root;
        }

        if (other.sz != 0) {
            other.root.next->prev = &other.root;
            other.root.prev->next = &other.root;
        }
    }

    List& operator=(const List& other) {


        NodeAllocator newAlloc = nodeAlloc;
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
            newAlloc = other.nodeAlloc;
        }

        List temp(newAlloc);

        for (auto& h : other) {

            temp.push_back(h);

        }

        swap(temp);

        return *this;

    }

    template <typename... Args>
    void insert(const_iterator pos, const Args&... values) {
        sz++;
        Node* newNode = std::allocator_traits<NodeAllocator>::allocate(nodeAlloc, 1);
        try {
            new (newNode) Node(values...);
        }
        catch (...) {
            std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, newNode, 1);
            sz--;
            throw;
        }

        BaseNode* posNode = const_cast<BaseNode*>(pos.node);

        BaseNode* cprv = posNode->prev;;
        (newNode)->next = posNode;;
        (newNode)->prev = posNode->prev;;

        cprv->next = newNode;
        posNode->prev = newNode;
    }

    


    void erase(const_iterator pos) {
        sz--;
        BaseNode* posNode = const_cast<BaseNode*>(pos.node);
        auto next = posNode->next;
        auto prev = posNode->prev;
        next->prev = prev;
        prev->next = next;

        (static_cast<Node*>(posNode))->~Node();

        std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, static_cast<Node*>(posNode), 1);
    }

    void push_back(const T& value) {

        insert(cend(), value);
    }
    void push_front(const T& value) {
        insert(cbegin(), value);
    }

    void pop_back() {
        erase(--cend());

    }
    void pop_front() {
        erase(cbegin());
    }

    Allocator get_allocator()const {
        return Allocator(nodeAlloc);
    }

    size_t size()const {
        return sz;
    }

};
