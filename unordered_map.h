

#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include <memory>
#include <stdexcept>

template<typename T>
class Debugg {
public:
    Debugg() = delete;

};

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
        //return storage->deallocate(p);
    }



};


template <typename T, typename Allocator = std::allocator<T> >
    requires requires{std::declval<T>().destroyData(std::declval<Allocator>()); }
class List {
private:
    template <typename Key, typename Value, typename Hash, typename Equal, typename InAlloc>
    friend class UnorderedMap;


    class BaseNode {
    public:
        BaseNode* prev, * next;

        BaseNode() :prev(this), next(this) {

        }

    };

    /*template <bool reversed = 0>
    class BaseNodeMoves {
    public:
        static BaseNode*& prev(BaseNode* node) {
            if constexpr (reversed)
                return node->next;
            return node->prev;
        }

        static BaseNode*& next(BaseNode* node) {
            return BaseNodeMoves<!reversed>::prev(node);
        }
    };*/

    class Node : public BaseNode {
    public:
        T value;
        Node(const T& value) = delete;


        Node(T&& value) = delete;

        template <typename... Args>
        Node(Args&&... args) : value(std::forward<Args>(args)...) {

        }



        Node() = delete;
    };

    using NodeAllocator = typename std::template allocator_traits<Allocator>::template rebind_alloc<Node>;
    [[no_unique_address]] NodeAllocator nodeAlloc;
    BaseNode root;
    size_t sz = 0;

    template<bool isConst>
    class BaseIterator {
        friend class List;

        template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
        friend class UnorderedMap;



    private:
        using nodeptrtype = std::conditional_t<isConst, const BaseNode*, BaseNode*>;
        nodeptrtype node;
        operator BaseIterator<false>() const {
            return BaseIterator<false>(const_cast<BaseNode*> (node));
        }



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
        

        reference operator*()const {
            return (((Node*)(node))->value);
        }

        pointer operator->()const {
            return &(((Node*)(node))->value);
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
private:



    iterator insertNode(const_iterator pos, Node* value) noexcept {
        sz++;
        Node* newNode = value;
        BaseNode* posNode = const_cast<BaseNode*>(pos.node);

        BaseNode* cprv = posNode->prev;;
        (newNode)->next = posNode;;
        (newNode)->prev = posNode->prev;;

        cprv->next = newNode;
        posNode->prev = newNode;

        return iterator(newNode);
    }

    Node* eraseNode(const_iterator pos) noexcept {
        sz--;
        BaseNode* posNode = const_cast<BaseNode*>(pos.node);
        auto next = posNode->next;
        auto prev = posNode->prev;
        next->prev = prev;
        prev->next = next;
        posNode->next = posNode;
        posNode->prev = posNode;

        return static_cast<Node*> (posNode);
    }


public:


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
            //std::cout << "POP BACK IN DESTRUCT" << std::endl;
            pop_back();
        }
    }

    List(Allocator alloc = Allocator()) :nodeAlloc(NodeAllocator(alloc)), root(), sz(0) {}

    List(size_t n, Allocator alloc = Allocator()) : nodeAlloc(NodeAllocator(alloc)), root(), sz(0) {
        for (size_t i = 0; i < n; i++) {
            try {
                insertDefault(cend());
            }
            catch (...) {
                destruct();
                throw;
            }
        }
    }



    List(size_t n, const T& val, Allocator alloc = Allocator()) : nodeAlloc(NodeAllocator(alloc)), root(), sz(0) {
        for (size_t i = 0; i < n; i++) {
            try {
                push_back(val);
            }
            catch (...) {
                destruct();
                throw;
            }
        }
    }



    ~List() {
        destruct();
    }

    List(List&& other) :nodeAlloc(other.nodeAlloc), root(other.root), sz(other.sz) {
        if (sz != 0) {
            root.next->prev = &root;
            root.prev->next = &root;
        }
        else {
            root.next = root.prev = &root;
        }
        other.sz = 0;
        other.root.next = other.root.prev = &other.root;
    }

    List(const List& other) :nodeAlloc(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.nodeAlloc)), root(), sz(0) {
        //alloc = std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc);
        //nodeAlloc = NodeAllocator(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.nodeAlloc));
        for (auto& h : other) {
            try {
                push_back(h);
            }
            catch (...) {
                destruct();
                throw;
            }
        }
    }


    List& operator= (List&& other) {
        if (&other == this)
            return *this;

        destruct();

        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
            nodeAlloc = other.nodeAlloc;
        }
        root = other.root;
        sz = other.sz;

        if (sz != 0) {
            root.next->prev = &root;
            root.prev->next = &root;
        }
        else {
            root.next = &root;
            root.prev = &root;
        }

        other.sz = 0;
        other.root.next = other.root.prev = &other.root;
        return *this;
    }

    List& operator=(const List& other) {
        if (&other == this)
            return *this;

        NodeAllocator newAlloc = nodeAlloc;
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
            newAlloc = other.nodeAlloc;
        }

        NodeAllocator oldAlloc = nodeAlloc;
        size_t oldsz = sz;


        BaseNode oldRoot = root;

        Node* firstNode = (sz == 0 ? 0 : static_cast<Node*> (root.next));

        root.next = root.prev = &root;

        nodeAlloc = newAlloc;
        sz = 0;



        for (auto& h : other) {
            try {
                push_back(h);
            }
            catch (...) {
                destruct();
                sz = oldsz;
                nodeAlloc = oldAlloc;
                root = oldRoot;
                throw;
            }
        }

        for (size_t i = 0; i < oldsz; i++) {
            Node* next = static_cast<Node*>(firstNode->next);
            (firstNode->value).destroyData(oldAlloc);

            std::allocator_traits<NodeAllocator>::destroy(oldAlloc, firstNode);
            std::allocator_traits<NodeAllocator>::deallocate(oldAlloc, firstNode, 1);

            firstNode = next;
        }
        return *this;

    }

    Node* getNode(const T& value) {
        Node* res;

        res = std::allocator_traits<NodeAllocator>::allocate(nodeAlloc, 1);
        try {
            std::allocator_traits<NodeAllocator>::construct(nodeAlloc, res, value);
        }
        catch (...) {
            std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, res, 1);
            throw;
        }

        return res;
    }

    template <typename... Args>
    Node* getNode(Args&&... value) {
        Node* res;

        res = std::allocator_traits<NodeAllocator>::allocate(nodeAlloc, 1);
        try {
            std::allocator_traits<NodeAllocator>::construct(nodeAlloc, res, std::forward<Args>(value)...);
        }
        catch (...) {
            std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, res, 1);
            throw;
        }

        return res;
    }

    void releaseNode(void* node) {
        (static_cast<Node*> (node)->value).destroyData(nodeAlloc);
        std::allocator_traits<NodeAllocator>::destroy(nodeAlloc, static_cast<Node*>(node));
        std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, static_cast<Node*>(node), 1);
    }


    iterator insert(const_iterator pos, const T& value) {
        Node* newNode = getNode(nodeAlloc, value);
        insertNode(pos, newNode);
        return iterator(newNode);
    }

    iterator insert(const_iterator pos, T&& value) {
        Node* newNode = getNode(nodeAlloc, std::move(value));
        insertNode(pos, newNode);

        return iterator(newNode);
    }

    void insertDefault(const_iterator pos) {
        sz++;
        Node* newNode = std::allocator_traits<NodeAllocator>::allocate(nodeAlloc, 1);
        try {

            new(newNode) Node();
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

        (static_cast<Node*> (posNode)->value).destroyData(nodeAlloc);


        std::allocator_traits<NodeAllocator>::destroy(nodeAlloc, static_cast<Node*> (posNode));


        std::allocator_traits<NodeAllocator>::deallocate(nodeAlloc, static_cast<Node*>(posNode), 1);
    }



    void push_back(const T& value) {

        insert(cend(), value);
    }

    void push_back(T&& value) {

        insert(cend(), std::move(value));
    }
    void push_front(const T& value) {
        insert(cbegin(), value);
    }

    void push_front(T&& value) {
        insert(cbegin(), std::move(value));
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


template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, Value> > >
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;

private:

    class Node {

    public:
        alignas (NodeType) char data[sizeof(NodeType)];
        size_t hash;



        Node(size_t hash, Allocator alloc, const NodeType& other) : hash(hash) {
            std::allocator_traits<Allocator>::construct(alloc, reinterpret_cast<NodeType*>(data), other);
        }
        Node(size_t hash, Allocator alloc, NodeType&& other) : hash(hash) {
            std::allocator_traits<Allocator>::construct(alloc, reinterpret_cast<NodeType*>(data), std::move(other));
        }



        template <typename... Args>
        Node(size_t hash, Allocator alloc, Args&&... args) : hash(hash) {
            std::allocator_traits<Allocator>::construct(alloc, reinterpret_cast<NodeType*>(data), std::forward<Args>(args)...);

        }

        /*template <typename T>
        Node(T&& ops, size_t hash) : data(std::forward<T>(ops)), hash(hash) {

        }*/
        NodeType& getData() const {
            return *(const_cast<NodeType*>(reinterpret_cast<const NodeType*>(data)));
        }

        Node(Allocator alloc, Node&& from) : hash(std::move(from.hash)) {
            std::allocator_traits<Allocator>::construct(alloc, reinterpret_cast<NodeType*>(data), from.getData());

        }
        Node(Allocator alloc, const Node& from) : hash(from.hash) {
            std::allocator_traits<Allocator>::construct(alloc, reinterpret_cast<NodeType*>(data), std::move(from.getData()));

        }

        Node& operator=(const Node& other) {
            data = other.data;
            hash = other.hash;
            return *this;
        }
        Node& operator=(Node&& other) {
            data = std::move(data);
            hash = std::move(other.hash);
            return *this;
        }

        template <typename OtherAllocator>
        void destroyData(OtherAllocator oa) {
            Allocator alloc(oa);
            std::allocator_traits<Allocator>::destroy(alloc, reinterpret_cast<NodeType*>(data));
        }

        void destruct(Allocator alloc) {
            std::allocator_traits<Allocator>::destruct(alloc, reinterpret_cast<NodeType*>(data));

        }

    };
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using IterAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<typename std::pair<typename List<Node, NodeAllocator>::iterator, bool> >;


    template<bool isConst>
    class BaseIterator {
        friend class UnorderedMap;




    private:
        using iterType = std::conditional_t<isConst, typename List<Node, NodeAllocator>::const_iterator, typename List<Node, NodeAllocator>::iterator>;
        iterType iter;
        operator BaseIterator<false>() const {
            return BaseIterator<false>(iter);
        }
    public:

        BaseIterator(iterType iter) : iter(iter) {

        }
        BaseIterator() {

        }
        using pointer = std::conditional_t<isConst, const NodeType*, NodeType*>;
        using reference = std::conditional_t<isConst, const NodeType&, NodeType&>;
        using value_type = NodeType;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = long long;


        BaseIterator& operator++() {
            ++iter;
            return *this;
        }
        BaseIterator& operator--() {
            --iter;
            return *this;
        }
        BaseIterator operator++(int) {
            BaseIterator res = *this;
            ++(*this);
            return res;
        }
        BaseIterator operator--(int) {
            BaseIterator res = *this;
            --(*this);
            return res;
        }

        bool operator==(const BaseIterator& other)const {
            return iter == other.iter;
        }
        

        reference operator* () const {

            return (reinterpret_cast<reference>(iter->data));
        }

        pointer operator->()const {

            //Debugg<decltype((*iter).data)> ops;

            return reinterpret_cast<pointer>(&(this->operator*()));
        }


        operator BaseIterator<true>()const {
            return BaseIterator<true>(iter);
        }
    };


    float loadFactor;
    size_t factor;

    List<Node, NodeAllocator> vals;
    using bucketType = std::pair<typename List<Node, NodeAllocator>::iterator, bool>;
    std::vector<bucketType, IterAllocator> buckets;


    [[no_unique_address]] NodeAllocator nodeAlloc;
    [[no_unique_address]] IterAllocator iterAlloc;
    [[no_unique_address]] Hash hash;
    [[no_unique_address]] Equal equal;

    static constexpr size_t sizes[] = { 11,53,593,5003,50021,500009,5000011,50000017ll,500000009ll ,5000000017ll,50000000009ll };

    size_t calcSize(size_t realSize) {
        int n = sizeof(sizes) / sizeof(size_t);
        for (int i = 0; i < n; i++) {
            if (realSize / loadFactor < sizes[i]) {
                return sizes[i];
            }
        }
        return sizes[n - 1];
    }
    using listNode = typename List<Node, NodeAllocator>::Node;



public:
    using iterator = BaseIterator<false>;
    using const_iterator = BaseIterator<true>;

    typename List<Node, NodeAllocator>::iterator getBucket(size_t pos) const {
        if (!buckets[pos].second)
            return vals.end();
        return buckets[pos].first;
    }
    typename List<Node, NodeAllocator>::iterator setBucket(size_t pos, typename List<Node, NodeAllocator>::iterator it) {
        if (it == vals.end()) {
            buckets[pos] = { it,0 };
        }
        else {
            buckets[pos] = { it,1 };
        }
        return it;
    }

    std::pair<iterator, bool> insertNode(listNode* node) noexcept {
        size_t chash = node->value.hash % factor;

        auto checkIt = getBucket(chash);

        while (checkIt != vals.end() && checkIt->hash == node->value.hash) {
            if (equal(checkIt->getData().first, node->value.getData().first)) {
                vals.releaseNode(node);
                return { checkIt,false };
            }
            checkIt++;
        }

        auto it = getBucket(chash);
        auto res = setBucket(chash, vals.insertNode(it, node));
        return { iterator(res),true };
    }

    void rehash(size_t size) {
        if (calcSize(size) <= factor) {
            return;
        }
        factor = calcSize(size);

        List<Node, NodeAllocator> oldVals = std::move(vals);
        buckets = std::vector<std::pair<typename List<Node, NodeAllocator>::iterator, bool>, IterAllocator>(factor, { vals.end(),false });

        while (oldVals.size() > 0) {
            auto res = oldVals.eraseNode(oldVals.begin());
            insertNode(res);

        }
    }

    void rehash() {
        if (calcSize(size()) <= factor) {
            return;
        }
        rehash(vals.size());

    }

public:

    float load_factor()const {
        return ((float)size()) / factor;
    }

    void max_load_factor(float ml) {
        loadFactor = ml;
        rehash();
    }

    float max_load_factor()const {
        return loadFactor;
    }

    iterator begin() {
        return iterator(vals.begin());
    }

    const_iterator begin()const {
        return const_iterator(vals.begin());
    }
    const_iterator cbegin() const {
        return const_iterator(vals.begin());

    }



    iterator end() {
        return iterator(vals.end());
    }

    const_iterator end() const {
        return const_iterator(vals.end());
    }
    const_iterator cend() const {
        return const_iterator(vals.end());
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


    std::reverse_iterator<iterator> rend() {
        return std::reverse_iterator<iterator>(begin());
    }

    std::reverse_iterator<const_iterator> rend() const {
        return std::reverse_iterator<const_iterator>(begin());
    }
    std::reverse_iterator<const_iterator> crend() const {
        return std::reverse_iterator<const_iterator>(cbegin());
    }


    size_t size()const {
        return vals.size();
    }



    UnorderedMap(Allocator alloc = Allocator()) :loadFactor(0.5), factor(calcSize(0)), vals(NodeAllocator(alloc)), buckets(std::vector<std::pair<typename List<Node, NodeAllocator>::iterator, bool>, IterAllocator>(factor, { vals.end(),0 }, IterAllocator(alloc))), nodeAlloc(alloc) {

    }


    Allocator get_allocator()const {
        return Allocator(nodeAlloc);
    }


    auto insert(const NodeType& data) {
        //Node node(data, hash(data.first));
        size_t hashval = hash(data.first);

        listNode* listNode;


        listNode = vals.getNode(0, get_allocator(), data);
        listNode->value.hash = hashval;

        auto res = insertNode(listNode);

        rehash();

        return res;

    }

    auto insert(NodeType&& data) {
        //Node node(std::move(data), hash(data.first));
        size_t hashval = hash(data.first);
        listNode* listNode;

        listNode = vals.getNode(0, get_allocator(), std::move(data.first), std::move(data.second));
        listNode->value.hash = hashval;

        auto res = insertNode(listNode);

        rehash();

        return res;

    }

    template <typename It>
    void insert(It first, It last) {
        while (first != last) {
            insert(*(first++));
        }

    }


    void erase(const_iterator it) {
        size_t chashf = it.iter->hash % factor;
        if (it.iter == static_cast<typename List<Node, NodeAllocator>::const_iterator>(getBucket(chashf))) {
            const_iterator nxt = it;
            nxt++;
            if (nxt == cend() || nxt.iter->hash % factor != chashf) {
                setBucket(chashf, vals.end());
            }
            else {
                setBucket(chashf, nxt.iter);
            }
        }
        vals.erase(it.iter);

    }

    void erase(const_iterator begin, const_iterator end) {
        while (begin != end) {
            erase(begin++);
        }
    }

    template <typename Self>
    decltype(auto) find(this Self&& self, const Key& key) {
        size_t hashf = self.hash(key) % self.factor;
        typename List<Node, NodeAllocator>::const_iterator it = self.getBucket(hashf);

        using ret = std::conditional_t<std::is_const_v<std::remove_reference_t<Self> >, const_iterator, iterator>;

        while (it != self.vals.cend() && it->hash % self.factor == hashf) {
            if (self.equal(key, it->getData().first))
                return ret(it);
            it++;

        }
        return ret(self.end());
    }


    Value& operator[](const Key& key) {
        auto it = find(key);
        if (it == end()) {
            return insert(NodeType(key, Value())).first->second;
        }
        return it->second;
    }
    Value& operator[](Key&& key) {
        auto it = find(key);
        if (it == end()) {
            return insert(NodeType(std::move(key), Value())).first->second;
        }
        return it->second;
    }

    template <typename Self>
    decltype(auto) at(this Self&& self, const Key& key) {
        auto it = self.find(key);
        if (it == self.end()) {
            throw std::out_of_range("Requested key not found.");

        }
        using ret = std::conditional_t<std::is_const_v<std::remove_reference_t<Self> >, const Value&, Value&>;
        return reinterpret_cast<ret>(it->second);
    }



    UnorderedMap(const UnorderedMap& other) :UnorderedMap() {



        nodeAlloc = std::allocator_traits<Allocator>::select_on_container_copy_construction(Allocator(nodeAlloc));
        loadFactor = other.loadFactor;
        for (auto& h : other) {
            insert(h);
        }
    }

    UnorderedMap(UnorderedMap&& other) :loadFactor(other.loadFactor), factor(other.factor), vals(std::move(other.vals)), buckets(std::move(other.buckets)), nodeAlloc(other.nodeAlloc), hash(other.hash), equal(other.equal) {
        other.vals = List<Node>();
        other.max_load_factor(0.5);
        other.factor = 0;

        other.rehash();

    }

    void swap(UnorderedMap& other) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
            std::swap(nodeAlloc, other.nodeAlloc);
            std::swap(iterAlloc, other.iterAlloc);

        }
        
        std::swap(factor, other.factor);
        std::swap(loadFactor, other.loadFactor);
        std::swap(vals, other.vals);
        std::swap(buckets, other.buckets);
    }


    UnorderedMap& operator=(UnorderedMap&& other) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
            nodeAlloc = other.nodeAlloc;
            iterAlloc = other.iterAlloc;

        }
        
        bool plainMove = false;
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<Allocator>::is_always_equal::value) {
            plainMove = true;
        }
        else {
            if (nodeAlloc == other.nodeAlloc && iterAlloc == other.iterAlloc) {
                plainMove = true;
            }
            else {
                vals = List<Node, NodeAllocator>(other.nodeAlloc);
                factor = other.factor;
                loadFactor = other.loadFactor;
                buckets = std::vector<bucketType, IterAllocator>(other.iterAlloc);

                buckets = std::vector<std::pair<typename List<Node, NodeAllocator>::iterator, bool>, IterAllocator>(factor, { vals.end(),0 });
                for (auto it = other.begin(); it != other.end(); it++) {
                    emplace(std::move(it->first), std::move(it->second));
                }


                while (other.size() > 0) {
                    other.erase(other.begin());
                }

            }
        }
        if (plainMove) {
            vals = std::move(other.vals);
            buckets = std::move(other.buckets);
            factor = other.factor;
            loadFactor = other.loadFactor;
        }

        other.factor = 0;

        other.rehash();
        return *this;

    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
            nodeAlloc = other.nodeAlloc;
            iterAlloc = other.iterAlloc;

        }

        auto newList = other.vals;
        auto newVector = other.buckets;
        factor = other.factor;
        loadFactor = other.loadFactor;
        vals = newList;
        buckets = newVector;

        return *this;
    }

    template <typename... Args>
    auto emplace(Args&&... args) {

        listNode* listNode;
        listNode = vals.getNode(0, get_allocator(), std::forward<Args>(args)...);
        try {
            listNode->value.hash = hash(listNode->value.getData().first);
        }
        catch (...) {
            vals.releaseNode(listNode);
        }


        auto res = insertNode(listNode);

        rehash();
        return res;
        // res.first;
    }

    template <typename U>
    auto insert(U&& val) {
        return emplace(std::forward<U>(val));
    }

    void reserve(size_t sz) {
        rehash(std::max(size(), sz));
    }



};