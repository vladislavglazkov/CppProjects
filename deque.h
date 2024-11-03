//TODO: sort out constructor of const_iterator
//rewrite insert without operator=
#include<cstring>
#include<stdexcept>
#include<iterator>

template<typename T>
class Deque {
private:
    static const size_t CHUNK_SIZE = 64;

    class naive_deque {
    public:
        using value_type = T*;
        static value_type my_new() {
            return reinterpret_cast<value_type>(new char[CHUNK_SIZE * sizeof(T)]);
        }

        size_t beg, end, cap;
        T** data;
        naive_deque(size_t count) : beg(count), end(count * 2), cap(count * 3),
            data(new value_type[cap]) {}
        naive_deque() : naive_deque(2) {}
        naive_deque(const naive_deque& other) : beg(other.beg), end(other.end), cap(other.cap),
            data(new value_type[cap]) {
            std::memcpy(data + beg, other.data + beg, end - beg);
        }

        naive_deque& operator=(const naive_deque& other) {
            naive_deque tmp(other);
            std::swap(beg, tmp.beg);
            std::swap(end, tmp.end);
            std::swap(cap, tmp.cap);
            std::swap(data, tmp.data);
            return *this;
        }

        void reserve(size_t new_cap, size_t new_beg = 0) {
            if (new_cap <= cap) {
                return;
            }
            value_type* tmp = new value_type[new_cap];
            std::memcpy(tmp + new_beg, data + beg, end - beg);
            std::swap(tmp, data);
            delete[] tmp;
            end = end + new_beg - beg;
            beg = new_beg;
        }

        value_type& operator[](size_t i) {
            return data[beg + i];
        }

        const value_type& operator[](size_t i) const {
            return data[beg + i];
        }

        void push_back() {
            if (data[end] == nullptr) {
                data[end] = my_new();
            }
            if (end + 1 == cap) {
                reserve(cap << 1, beg);
            }
            ++end;
        }

        void push_front() {
            value_type tmp = my_new();
            if (beg == 1) {
                reserve(cap << 1, cap);
            }
            --beg;
            if (data[beg] == nullptr) {
                data[beg] = tmp;
            }
        }

        void pop_back() {
            delete data[--end];
        }

        void pop_front() {
            delete data[beg++];
        }

        size_t size() const {
            return end - beg;
        }

        value_type& back() {
            return data[end - 1];
        }

        const value_type& back() const {
            return data[end - 1];
        }
    };

    naive_deque data;
    size_t _beg, _end;

    template<bool is_const>
    struct base_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using pointer_type = std::conditional_t<is_const, const T*, T*>;
        using value_type = std::conditional_t<is_const, const T, T>;
        using reference_type = std::conditional_t<is_const, const T&, T&>;;
    private:
        pointer_type inner;
        pointer_type* outer;
        size_t in_pos;
    public:

        base_iterator(pointer_type* outer, size_t in_pos) :
            inner((*outer) + in_pos), outer(outer), in_pos(in_pos) {}

        base_iterator(const base_iterator&) = default;

        base_iterator& operator=(const base_iterator&) = default;

        operator base_iterator<true>() const {
            return base_iterator<true>(outer, in_pos);
        }

        base_iterator& operator+=(int count) {
            in_pos += count;
            outer += in_pos / CHUNK_SIZE;
            in_pos %= CHUNK_SIZE;
            if (in_pos < 0) {
                in_pos += CHUNK_SIZE;
                --outer;
            }
            inner = (*outer) + in_pos;
            return *this;
        }

        base_iterator& operator-=(int count) {
            *this += -count;
            return *this;
        }

        base_iterator operator+(int count) const {
            base_iterator result = *this;
            result += count;
            return result;
        }

        base_iterator operator-(int count) const {
            base_iterator result = *this;
            result -= count;
            return result;
        }

        base_iterator& operator++() {
            *this += 1;
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator result = *this;
            *this += 1;
            return result;
        }

        base_iterator& operator--() {
            *this += -1;
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator result = *this;
            *this += -1;
            return result;
        }

        int operator-(base_iterator other) const {
            return (outer - other.outer) * CHUNK_SIZE + in_pos - other.in_pos;
        }

        reference_type operator*() const {
            return *inner;
        }

        pointer_type operator->() const {
            return inner;
        }

        bool operator<(const base_iterator& other) const {
            return *this - other < 0;
        }

        bool operator<=(const base_iterator& other) const {
            return *this - other <= 0;
        }

        bool operator>(const base_iterator& other) const {
            return *this - other > 0;
        }

        bool operator>=(const base_iterator& other) const {
            return *this - other >= 0;
        }

        bool operator==(const base_iterator& other) const {
            return (outer == other.outer) && (in_pos == other.in_pos);
        }

        bool operator!=(const base_iterator& other) const {
            return !((outer == other.outer) && (in_pos == other.in_pos));
        }
    };
public:

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(&data[0], _beg);
    }

    iterator end() {
        return iterator(&data.back(), _end);
    }

    const_iterator begin() const {
        return const_iterator(&data[0], _beg);
    }

    const_iterator end() const {
        return const_iterator(&data.back(), _end);
    }

    const_iterator cbegin() const {
        return const_iterator(&data[0], _beg);
    }

    const_iterator cend() const {
        return const_iterator(&data.back(), _end);
    }

    reverse_iterator rbegin() {
        if (_end == 0) {
            return reverse_iterator(iterator(&data.back() - 1, CHUNK_SIZE - 1));
        }
        return reverse_iterator(iterator(&data.back(), _end - 1));
    }

    reverse_iterator rend() {
        if (_beg == 0) {
            return reverse_iterator(iterator(&data[0] - 1, CHUNK_SIZE));
        }
        return reverse_iterator(iterator(&data[0], _beg - 1));
    }

    const_reverse_iterator rbegin() const {
        if (_end == 0) {
            return const_reverse_iterator(const_iterator(&data.back() - 1, CHUNK_SIZE - 1));
        }
        return const_reverse_iterator(const_iterator(&data.back(), _end - 1));
    }

    const_reverse_iterator rend() const {
        if (_beg == 0) {
            return const_reverse_iterator(const_iterator(&data[0] - 1, CHUNK_SIZE));
        }
        return const_reverse_iterator(const_iterator(&data[0], _beg - 1));
    }

    const_reverse_iterator crbegin() const {
        if (_end == 0) {
            return const_reverse_iterator(const_iterator(&data.back() - 1, CHUNK_SIZE - 1));
        }
        return const_reverse_iterator(const_iterator(&data.back(), _end - 1));
    }

    const_reverse_iterator crend() const {
        if (_beg == 0) {
            return const_reverse_iterator(const_iterator(&data[0] - 1, CHUNK_SIZE));
        }
        return const_reverse_iterator(const_iterator(&data[0], _beg - 1));
    }

    Deque() : data(1), _beg(0), _end(0) {}

    Deque(const Deque& other) = default;

    Deque(size_t count, const T& value) : data(1), _beg(0), _end(0) {
        for (size_t i = 0; i < count; ++i) {
            try {
                push_back(value);
            }
            catch (...) {
                this->~Deque();
                throw;
            }
        }
    }

    Deque(size_t count) : data(1), _beg(0), _end(0) {
        for (size_t i = 0; i < count; ++i) {
            try {
                if (_end == CHUNK_SIZE - 1) {
                    data.push_back();
                    _end = 0;
                }
                new (data.back() + _end++) T();
            }
            catch (...) {
                this->~Deque();
                throw;
            }
        }
    }

    Deque& operator=(const Deque& other) = default;

    ~Deque() {
        while (size() != 0) {
            pop_back();
        }
    }

    size_t size() const {
        return _end - _beg + CHUNK_SIZE * (data.size() + 1);
    }

    T& operator[](size_t i) {
        i += _beg;
        return data[i / CHUNK_SIZE][i % CHUNK_SIZE];
    }

    const T& operator[](size_t i) const {
        i += _beg;
        return data[i / CHUNK_SIZE][i % CHUNK_SIZE];
    }

    const T& at(size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("exception thrown at method at()");
        }
        return (*this)[i];
    }

    T& at(size_t i) {
        if (i >= size()) {
            throw std::out_of_range("exception thrown at method at()");
        }
        return (*this)[i];
    }

    void push_back(const T& value) {
        if (_end == CHUNK_SIZE - 1) {
            data.push_back();
            _end = 0;
        }
        try {
            new (data.back() + _end++) T(value);
        }
        catch (...) {
            if (_end == 0) {
                data.pop_back();
                _end = CHUNK_SIZE - 1;
            }
            throw;
        }
    }

    void push_front(const T& value) {
        if (_beg == 0) {
            data.push_front();
            _beg = CHUNK_SIZE - 1;
        }
        try {
            new (data[0] + _beg--) T(value);
        }
        catch (...) {
            if (_beg == CHUNK_SIZE - 1) {
                data.pop_front();
                _beg = 0;
            }
            throw;
        }
    }

    void pop_back() {
        if (_end == 0) {
            data.pop_back();
            _end = CHUNK_SIZE;
        }
        --_end;
        data.back()[_end].~T();
    }

    void pop_front() {
        data[0][_beg].~T();
        ++_beg;
        if (_beg == CHUNK_SIZE) {
            data.pop_front();
            _beg = 0;
        }
    }

    void insert(iterator it, T value) {
        while (it != end()) {
            T tmp(*it);
            *it = value;
            value = tmp;
            ++it;
        }
        push_back(value);
    }

    void erase(iterator it) {
        while (it != end()) {
            T tmp(*it);
            *it = *(it + 1);
            *(it + 1) = tmp;
            ++it;
        }
        pop_back();
    }
};