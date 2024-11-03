#define CPP23 1
#pragma once
#pragma clang optimize on
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <array>
#include <future>
#include <deque>




template <typename T>
class myAlloc {
public:


    myAlloc() = default;

    template<class U>
    constexpr myAlloc(const myAlloc <U>&) noexcept {}

    struct Block {
        T* start, * end;
        size_t remain;
        T* cur = 0;
        size_t usedAllocs = 0;
        Block* next = 0;
    };
    static constexpr size_t maxDesiredSize = 10000000;
    static size_t currentlyAllocatedSize;
    static constexpr size_t basicSize = 1024;
    static constexpr size_t Tsize = sizeof(T);
    static Block* first;
    typedef T value_type;


    bool operator==(const myAlloc&)const {
        return true;
    }
    bool operator!=(const myAlloc&)const {
        return false;
    }

    T* allocate(size_t n) {

        Block* cblock = first;
        Block* prev = 0;
        while (cblock) {
            if (cblock->remain >= n) {
                break;
            }
            prev = cblock;
            cblock = cblock->next;

        }
        if (cblock) {
            if (cblock != first) {
                prev->next = cblock->next;
                cblock->next = first;
                first = cblock;
            }
        }

        if (!cblock) {
            Block* nw = new Block();
            nw->start = new T[basicSize];
            currentlyAllocatedSize += basicSize;
            nw->end = nw->start + basicSize;
            nw->cur = nw->start;
            nw->next = first;
            nw->remain = basicSize;
            first = nw;
            cblock = nw;
        }

        cblock->usedAllocs++;
        T* ans = cblock->cur;
        cblock->cur += n;
        cblock->remain -= n;
        return ans;
    }
    void deallocate(T* addr, size_t) {
        Block* cblock = first;
        Block* prev = 0;
        while (cblock != 0) {
            if (addr >= cblock->start && addr <= cblock->end) {
                cblock->usedAllocs--;
                if (cblock->usedAllocs == 0) {
                    /*if (!first) {
                        prev->next = cblock->next;
                        cblock->next = first;
                        first = cblock;
                    }*/
                    if (currentlyAllocatedSize > maxDesiredSize) {
                        if (cblock == first) {
                            first = first->next;

                        }
                        else {
                            prev->next = cblock->next;
                        }

                        delete[] cblock->start;
                        delete cblock;
                    }
                    cblock->remain = basicSize;
                    cblock->cur = cblock->start;
                }
                break;
            }

            prev = cblock;
            cblock = cblock->next;
        }
    }

};

template <typename T>
myAlloc<T>::Block* myAlloc<T>::first = 0;

template <typename T>
size_t myAlloc<T>::currentlyAllocatedSize = 0;

const size_t largeSize = 40;


class DecBase {
private:
    int value;

public:
    DecBase(unsigned int value) : value(value) {

    }
    DecBase() :DecBase(0) { }
    DecBase(std::string& value) : value(std::stoi(value)) {}
    static constexpr int base = 1000000000;
    static constexpr int strlen = 9;

    friend std::strong_ordering operator<=>(const DecBase&, const DecBase&) = default;


    std::pair<bool, DecBase> operator+(const DecBase& other) const {
        int ans = value + other.value;

        if (ans >= base)
            return { 1,DecBase(ans - base) };
        return { 0,ans };
    }
    DecBase operator-() const {
        return (base - value) % base;
    }

    std::pair<bool, DecBase> operator-(const DecBase& other)const {
        return { other.value > value, (*this + (-other)).second };
    }

    std::pair<DecBase, DecBase> operator*(const DecBase& other) const {


        DecBase lesser = DecBase(((long long)value * other.value) % base);
        DecBase greater = DecBase(((long long)value * other.value) / base);
        return { greater,lesser };
    }
    DecBase operator/ (const DecBase& other)const {
        return value / other.value;
    }
    long double asDouble()const {
        return value;
    }



    std::string toString()const {
        long long ops = value;
        std::string ans;
        for (int i = 0; i < strlen; i++) {
            ans += (ops % 10) + '0';
            ops /= 10;
        }
        reverse(ans.begin(), ans.end());
        return ans;

    }

};

class BigNatural {
    friend class BigInteger;
protected:
    std::deque<DecBase, myAlloc<DecBase> > blocks;
    int shifted = 0;
    int getBlockSize() const {
        return (blocks.size() + shifted);
    }
    void shift(int by) {
        shifted += by;
    }

    DecBase getIthBlockFromEnd(int i) const {
        if (!existsIthBlockFromEnd(i))
            return 0;
        if (i < shifted) {
            return 0;
        }
        i -= shifted;
        return blocks[blocks.size() - i - 1];
    }
    DecBase& getIthBlockFromEndRef(int i) {
        while (!existsIthBlockFromEnd(i)) {
            blocks.push_front(0);
        }

        while (i < shifted) {
            --shifted;
            blocks.push_back(0);
        }

        i -= shifted;
        return blocks[blocks.size() - i - 1];

    }
    BigNatural() :blocks(std::deque<DecBase, myAlloc<DecBase > >{}) {}

    BigNatural(DecBase db) :blocks(std::deque<DecBase, myAlloc<DecBase> >{db}) { sweep(); }

    void removeFromEnd() {
        if (shifted > 0)
            --shifted;
        else
            blocks.pop_back();
    }
    bool isZero()const {
        return getBlockSize() == 0;
    }
    BigNatural& operator-=(const BigNatural& other) {
        int pos = other.shifted;
        bool toAdd = 0;
        while ((other.existsIthBlockFromEnd(pos)) || toAdd) {
            DecBase myVal = getIthBlockFromEnd(pos);
            DecBase otherVal = other.getIthBlockFromEnd(pos);


            auto tempRes = myVal - otherVal;

            auto finRes = tempRes.second - toAdd;
            finRes.first |= tempRes.first;
            toAdd = finRes.first;
            getIthBlockFromEndRef(pos++) = finRes.second;

        }
        sweep();
        return *this;
    }

    

    BigNatural& operator+=(const BigNatural& other) {
        int pos = other.shifted;
        bool toAdd = 0;
        while ((other.existsIthBlockFromEnd(pos)) || toAdd) {
            DecBase myVal = getIthBlockFromEnd(pos);
            DecBase otherVal = other.getIthBlockFromEnd(pos);


            auto tempRes = myVal + otherVal;

            auto finRes = tempRes.second + toAdd;
            finRes.first |= tempRes.first;
            toAdd = finRes.first;


            getIthBlockFromEndRef(pos++) = finRes.second;

        }
        sweep();
        return *this;
    }

    std::weak_ordering operator<=>(const BigNatural& other) const {
        char sign = 0;


        if (getBlockSize() < other.getBlockSize())
            sign = -1;
        else if (getBlockSize() > other.getBlockSize())
            sign = 1;
        else {
            for (int i = 0; i < getBlockSize(); i++) {
                if (getIthBlockFromEnd(getBlockSize() - i - 1) < other.getIthBlockFromEnd(getBlockSize() - i - 1)) {
                    sign = -1;
                    break;
                }
                if (getIthBlockFromEnd(getBlockSize() - i - 1) > other.getIthBlockFromEnd(getBlockSize() - i - 1)) {
                    sign = 1;
                    break;
                }

            }
        }

        if (sign == 1) {
            return std::strong_ordering::greater;
        }
        if (sign == -1)
            return std::strong_ordering::less;
        return std::strong_ordering::equal;

    }

    bool existsIthBlockFromEnd(int i)const {
        return (getBlockSize() - i - 1 >= 0);
    }
    void operator*=(const DecBase& other) {
        if (other == 0) {
            blocks.clear();
            shifted = 0;
        }
        int pos = shifted;
        DecBase toAdd = 0;
        while (existsIthBlockFromEnd(pos) || toAdd != 0) {
            DecBase my = getIthBlockFromEnd(pos);
            auto temp1 = my * other;
            auto temp2 = temp1.second + toAdd;
            temp1.first = (temp1.first + DecBase(temp2.first)).second;
            getIthBlockFromEndRef(pos++) = temp2.second;
            toAdd = temp1.first;

        }
        sweep();

    }
    
    std::pair<BigNatural, BigNatural> split(int k) const {
        BigNatural my = *this;
        if (getBlockSize() <= k) {
            return { BigNatural(),*this };
        }
        BigNatural sec;
        for (int i = 0; i < k; i++) {
            sec.blocks.push_front(my.getIthBlockFromEnd(0));
            my.removeFromEnd();
        }
        sec.sweep();
        return { my,sec };

    }

    void sweep() {
        while (blocks.size() > 0 && blocks.front() == 0)
            blocks.pop_front();
        if (blocks.size() == 0)
            shifted = 0;
    }

    DecBase findBest(const BigNatural& other, int shift) const {
        DecBase L = 0, R = DecBase::base - 1;

        BigNatural mcheck = other;
        mcheck.shift(shift);
        mcheck *= R;
        if (mcheck <= *this)
            return R;



        while ((R - L).second > 1) {
            DecBase med = (L + (R - L).second / 2).second;
            BigNatural ops = other;
            ops.shift(shift);
            ops *= med;
            if (ops <= *this) {
                L = med;
            }
            else {
                R = med;
            }

        }

        return L;

    }
    BigNatural operator/(const BigNatural& other) const {
        BigNatural my = *this;
        /*if (my.getBlockSize() > 2000)
            return BigNatural();*/
        BigNatural ans;
        BigNatural toSbc;
        BigNatural toAdd;
        for (int i = 0; i < getBlockSize(); i++) {

            int shiftBy = getBlockSize() - i - 1;
            if (other.getBlockSize() + shiftBy > getBlockSize()) {
                continue;
            }
            DecBase ops = my.findBest(other, shiftBy);
            toSbc = other;
            toSbc *= ops;
            toSbc.shift(shiftBy);
            my -= toSbc;
            toAdd = (ops);
            toAdd.shift(shiftBy);
            ans += toAdd;
        }
        ans.sweep();

        return ans;
    }
    BigNatural(const std::string& str) {
        int pos = static_cast<int>(str.size());
        while (pos > 0) {
            int oplen = std::min(pos, DecBase::strlen);
            std::string val = str.substr(pos - oplen, oplen);
            blocks.push_front(val);
            pos -= oplen;
        }
        sweep();
    }
    std::string toString() const {
        std::string ans;
        for (int i = 0; i < getBlockSize(); i++) {
            auto h = getIthBlockFromEnd(getBlockSize() - i - 1);
            ans += h.toString();
        }
        while (!ans.empty() && ans.front() == '0')
            ans = ans.substr(1);
        if (ans.empty())
            ans = "0";
        return ans;
    }

    friend BigNatural operator*(const BigNatural& myy, const BigNatural& ot) {

        const BigNatural* my = &myy, * other = &ot;
        if (my->getBlockSize() < other->getBlockSize())
            std::swap(my, other);
        if (other->isZero())
            return BigNatural();

        /*if (other.getBlockSize() == 1) {
            BigNatural ans = my;
            ans *= other.getIthBlockFromEnd(0);
            return ans;
        }*/
        if ((long long)other->getBlockSize() * my->getBlockSize() <= 800) {
            BigNatural ans;
            BigNatural temp;
            for (int i = 0; i < other->getBlockSize(); ++i) {
                temp = *my;
                temp *= other->getIthBlockFromEnd(i);
                temp.shift(i);
                ans += temp;

            }
            ans.sweep();
            return ans;
        }

        int med = my->getBlockSize() / 2;
        auto mySplit = my->split(med);
        auto otherSplit = other->split(med);

        BigNatural abTcd, aTc, bTd;

        aTc = mySplit.first * otherSplit.first;

        auto& mySum = mySplit.first;
        mySum += mySplit.second;
        auto& otherSum = otherSplit.first;
        otherSum += otherSplit.second;



        abTcd = mySum * otherSum;
        bTd = mySplit.second * otherSplit.second;

        abTcd -= aTc;
        abTcd -= bTd;

        aTc.shift(2 * med);
        abTcd.shift(med);

        aTc += abTcd;
        aTc += bTd;
        aTc.sweep();

        return aTc;

    }


};






class BigInteger : private BigNatural {
private:
    bool neg;


    const BigNatural& self() const {
        return *this;
    }
    BigNatural& self() {
        return *this;
    }

    static std::weak_ordering transform(std::weak_ordering arg1, bool invert) {
        if (!invert)
            return arg1;
        if (arg1 == std::weak_ordering::equivalent)
            return std::weak_ordering::equivalent;
        if (arg1 == std::weak_ordering::greater)
            return std::weak_ordering::less;
        return std::weak_ordering::greater;
    }

    static BigInteger gcdIn(const BigInteger& arg1, const BigInteger& arg2)
    {

        if (arg2 == 0)
            return arg1;
        return gcdIn(arg2, arg1 % arg2);


    }

    void sweep() {
        if (isZero())
            neg = 0;

    }

public:

    BigInteger() : BigNatural(), neg(0) {}

    BigInteger& operator++() {
        *this -= 1;
        return *this;
    }
    BigInteger& operator--() {
        *this -= 1;
        return *this;
    }
    BigInteger operator++(int) {
        BigInteger ans = *this;
        *this += 1;
        return ans;
    }
    BigInteger operator--(int) {
        BigInteger ans = *this;
        *this -= 1;
        return ans;
    }


    bool isNeg()const {
        return neg;
    }
    
    friend std::weak_ordering operator<=>(const BigInteger& my, const BigInteger& other) {
        if (my.neg == other.neg) {
            return transform(my.self().operator<=>(other.self()), my.neg);
        }
        if (my.neg)
            return std::weak_ordering::less;
        return std::weak_ordering::greater;

    }

    explicit BigInteger(const BigNatural& from) :BigNatural(from), neg(0) {}

    BigInteger(std::string str) :neg(0) {
        if (str.empty())
            return;
        if (str.front() == '-') {
            neg = 1;
            str = str.substr(1);
        }
        self() = BigNatural(str);
        sweep();

    }
    BigInteger(int from) :BigInteger(std::to_string(from)) {}

    BigInteger& operator= (BigNatural& other) {
        self()=other;
        return *this;
    }

    friend BigInteger operator-(const BigInteger& my) {
        BigInteger ans = my;
        ans.invert();
        return ans;
    }
    void invert() {
        if (!isZero())
            neg ^= 1;
    }

    BigInteger& operator+=(const BigInteger& other) {
        if (neg == other.neg) {
            self() += other.self();
        }
        else if (self() > other.self()) {
            self() -= other.self();
        }
        else {
            BigNatural temp = other.self();
            temp -= self();
            *this = temp;
            this->invert();

        }
        sweep();
        return *this;
    }
    BigInteger& operator-=(const BigInteger& other) {
        *this += (-other);

        return *this;
    }

    friend BigInteger operator+(const BigInteger& my, const BigInteger& other) {
        BigInteger ans = my;
        ans += other;
        return ans;
    }
    friend BigInteger operator-(const BigInteger& my, const BigInteger& other) {
        BigInteger ans = my;
        ans -= other;
        return ans;
    }
    
    BigInteger& operator*=(const DecBase& other) {
        self() *= (other);
        sweep();
        return *this;

    }
    BigInteger operator*(const DecBase& other) const {
        BigInteger ans = *this;
        ans *= other;
        return ans;
    }

    BigInteger& operator*=(const BigInteger& other) {
        if (other.neg)
            invert();
        self() = (self() * other.self());
        sweep();
        return *this;
    }
    BigInteger& operator/=(const BigInteger& other) {
        if (other.neg) {
            invert();
        }
        self() = (self() / other.self());
        sweep();
        return *this;
    }

    BigInteger& operator%=(const BigInteger& other) {
        *this = (*this % other);
        return *this;
    }

    friend BigInteger operator/(const BigInteger& my, const BigInteger& other) {
        BigInteger ans = my;
        ans /= other;
        return ans;
    }

    friend BigInteger operator*(const BigInteger& my, const BigInteger& other) {
        BigInteger ans = my;
        ans *= other;
        return ans;
    }
    friend BigInteger operator%(const BigInteger& my, const BigInteger& other) {
        BigInteger res = BigInteger(my.self());
        res -= BigInteger((my.self() / other.self()) * other.self());
        if (my.neg)
            res.invert();
        return res;
    }
    explicit operator long double() const {
        long double ans = 0;
        for (int i = 0; i < getBlockSize(); ++i) {
            auto h = blocks[i];
            ans += h.asDouble();
            ans *= (long double)DecBase::base;
        }
        if (neg)
            ans = -ans;

        return ans;
    }
    friend bool operator==(const BigInteger& my, const BigInteger& other) {
        if (my.neg != other.neg)
            return false;
        if (my.getBlockSize() != other.getBlockSize()) {
            return false;
        }
        for (int i = 0; i < my.getBlockSize(); ++i) {
            if (my.blocks[i] != other.blocks[i]) {
                return false;
            }
        }
        return true;
    }
    BigInteger abs()const {
        return BigInteger(self());
    }
    void toAbs() {
        neg = false;
    }

   

    static BigInteger gcd(BigInteger arg1, BigInteger arg2)
    {
        arg1.toAbs();
        arg2.toAbs();
        if (arg2 == 0)
            return arg1;
        return gcdIn(arg2, arg1 % arg2);


    }

    explicit operator bool()const {
        return !isZero();
    }
    static BigInteger fromDecimalPower(int power) {
        BigInteger ops(1);
        while (power >= DecBase::strlen) {
            ops.shift(1);
            power -= DecBase::strlen;
        }
        long long val = 1;
        while (power > 0) {
            val *= 10;
            --power;
        }
        ops.blocks.front() = 1 * val;
        return ops;

    }
    std::string toString() const {

        std::string ans;
        if (neg)
            ans = "-";
        ans += self().toString();
        return ans;
    }
};


class Rational {
private:

    BigInteger num, denom;
public:

    Rational() :num(), denom(1) {}
    Rational(int ll) :num(ll), denom(1) {}
    Rational(BigInteger arg) :num(arg), denom(1) {}
    Rational(BigInteger _num, BigInteger _denom) : num(_num), denom(_denom) {
        if (denom.isNeg()) {
            num.invert();
            denom.invert();
        }
        BigInteger gcd = BigInteger::gcd(num, denom);
        num /= gcd;
        denom /= gcd;
    }
    static Rational createForced(const BigInteger& num, const BigInteger& denom);

    friend Rational operator+(const Rational& my, const Rational& other) {
        return Rational(my.num * other.denom + my.denom * other.num, my.denom * other.denom);

    }
    friend Rational operator- (const Rational& my) {
        Rational ans = my;
        ans.num.invert();
        return ans;
    }

    friend Rational operator-(const Rational& my, const Rational& other) {
        return my + (-other);
    }
    Rational& operator+=(const Rational& other) {
        *this = *this + other;
        return *this;
    }
    Rational& operator-=(const Rational& other) {
        *this = *this - other;
        return *this;
    }

    friend Rational operator*(const Rational& my, const Rational& other) {

        Rational short1(my.num, other.denom);
        Rational short2(other.num, my.denom);
        return createForced(short1.num * short2.num, short1.denom * short2.denom);

    }
    friend Rational operator/(const Rational& my, const Rational& other) {
        Rational short1(my.num, other.num);
        Rational short2(other.denom, my.denom);
        return createForced(short1.num * short2.num, short1.denom * short2.denom);
    }

    Rational& operator*=(const Rational& other) {
        *this = *this * other;
        return *this;
    }
    Rational& operator/=(const Rational& other) {
        *this = *this / other;
        return *this;
    }
    friend bool operator==(const Rational& my, const Rational& other) {
        return my.num == other.num && my.denom == other.denom;
    }

    friend std::weak_ordering operator<=>(const Rational& my, const Rational& other) {
        Rational diff = my - other;
        if (diff.num == 0)
            return std::weak_ordering::equivalent;
        if (diff.num < BigInteger(0))
            return std::weak_ordering::less;

        return std::weak_ordering::greater;
    }
    Rational abs() {
        Rational ans = *this;
        ans.num.toAbs();
        return ans;

    }

    std::string asDecimal(size_t prec = 0)const {
        int precision = static_cast<int>(prec);
        BigInteger attemptLow = (num * BigInteger::fromDecimalPower(precision)).abs() / denom;
        BigInteger attemptHi = attemptLow + 1;

        BigInteger testLow = attemptLow * denom - num.abs() * BigInteger::fromDecimalPower(precision);
        BigInteger testHi = attemptHi * denom - num.abs() * BigInteger::fromDecimalPower(precision);
        BigInteger sel(0);
        if ((testLow).abs() <= (testHi).abs()) {
            sel = attemptLow;
        }
        else {
            sel = attemptHi;
        }
        std::string ans = "";
        if (num < BigInteger(0)) {
            ans += '-';
        }


        if (sel == 0) {
            ans += "0";
            if (precision > 0) {
                ans += '.';
                for (int i = 0; i < precision; i++) {
                    ans += '0';
                }
            }
        }
        else {
            std::string fromo = sel.toString();
            std::string val = "";
            while (val.size() + fromo.size() < prec) {
                val += '0';
            }
            fromo = val + fromo;
            ans += fromo.substr(0, fromo.size() - prec);
            if (fromo.size() == prec) {
                ans += "0";
            }
            if (precision > 0) {
                ans += '.';
                ans += fromo.substr(fromo.size() - prec, precision);
            }
        }

        return ans;


    }

    std::string toString()const {
        std::string ans;

        ans += num.toString();
        if (denom != 1) {
            ans += "/";
            ans += denom.toString();
        }
        return ans;
    }

    explicit operator long double()const {
        return (static_cast<long double>(num)) / (static_cast<long double>(denom));
    }
    explicit operator double()const {
        return static_cast<long double>(*this);
    }
};
std::istream& operator>>(std::istream& ops, BigInteger& to) {
    std::string s;
    ops >> s;
    to = BigInteger(s);
    return ops;
}
std::ostream& operator<<(std::ostream& ops, const BigInteger& from) {
    std::string s = from.toString();
    ops << s;
    return ops;


}

std::istream& operator>>(std::istream& ops, Rational& to) {
    std::string s;
    ops >> s;
    to = Rational(s);
    return ops;
}

BigInteger operator""_bi(const char* str, size_t) {
    return BigInteger(str);
}
Rational Rational::createForced(const BigInteger& num, const BigInteger& denom) {

    Rational ans;
    ans.num = num;
    ans.denom = denom;
    return ans;
}





template <size_t N, size_t i>
struct isPrimeHelper {
    constexpr static bool value = (N % i == 0 ? 0 : isPrimeHelper<N, i - 1>::value);
};

template <size_t N>
struct isPrimeHelper<N, 1> {
    constexpr static bool value = true;
};



template <size_t L, size_t R, size_t V>
struct sqrtHelper {
    constexpr static size_t M = (L + R) / 2;
    constexpr static size_t value = (M * M >= V ? sqrtHelper<L, M, V>::value : sqrtHelper<M + 1, R, V>::value);
};


template <size_t L, size_t V>
struct sqrtHelper<L, L, V> {

    constexpr static size_t value = L;
};

template <size_t N>
struct constexprSqrt {
    constexpr static size_t value = sqrtHelper<0, N, N>::value;
};


template<size_t N>
struct isPrime {
    constexpr static bool value = isPrimeHelper<N, sqrtHelper<0, N, N>::value>::value;
};


template<size_t Mod>
class Residue {
private:
    size_t val;
public:
    Residue(int n) {
        bool flag = 0;
        if (n < 0) {
            n = -n;
            flag = 1;
        }
        val = n % Mod;
        if (flag) {
            val = Mod - val;
            val %= Mod;
        }
    }
    Residue() {
        val = 0;
    }
    explicit operator int() const {
        return val;
    }

    Residue& operator+=(const Residue& other) {
        val += other.val;
        val %= Mod;
        return *this;
    }
    Residue operator-()const {
        auto ans = *this;
        ans.val = Mod - ans.val;
        ans.val %= Mod;
        return ans;
    }
    Residue& operator-=(const Residue& other) {
        *this += (-other);
        return *this;
    }
    Residue& operator++() {
        *this += Residue(1);
        return *this;
    }
    Residue& operator--() {
        *this -= Residue(1);
        return *this;
    }
    Residue operator++(int)const {
        auto ans = *this;
        *this += Residue(1);
        return ans;
    }
    Residue operator--(int)const {
        auto ans = *this;
        *this -= Residue(1);
        return ans;
    }
    Residue operator+(const Residue& other)const {
        auto ans = *this;
        ans += other;
        return ans;
    }
    Residue operator-(const Residue& other)const {
        return *this + (-other);
    }
    Residue& operator*=(const Residue& other) {
        val *= other.val;
        val %= Mod;
        return *this;
    }
    Residue operator*(const Residue& other)const {
        auto ans = *this;
        ans *= other;
        return ans;
    }
    Residue pow(size_t deg)const {
        if (deg == 0) {
            return Residue(1);
        }
        if (deg == 1) {
            return *this;
        }
        if (deg % 2 == 1) {
            return (pow(deg - 1) * (*this));
        }
        auto temp = pow(deg / 2);
        return temp * temp;
    }
    Residue getInv()const {
        static_assert(isPrime<Mod>::value);
        return pow(Mod - 2);
    }
    Residue& operator/=(const Residue& other) {
        *this *= other.getInv();
        return *this;
    }
    Residue operator/(const Residue& other)const {
        auto ans = *this;
        ans /= other;
        return ans;
    }

    bool operator==(const Residue& other)const {
        return val == other.val;
    }
    bool operator!=(const Residue& other)const {
        return !(*this == other);
    }

};


template <size_t N, typename Field>
Field dotProduct(const std::array<Field, N>& a, const std::array<Field, N>& b) {
    Field ans;
    for (size_t i = 0; i < N; i++) {
        ans += (a[i] * b[i]);
    }
    return ans;

}

template <size_t N, size_t M, typename Field, typename T>
class MatrixExpression {
public:
    static constexpr bool leaf = 0;

    Field operator[] (size_t i, size_t j)const {
        return static_cast<const T&>(*this).operator[](i, j);
    }

    std::array<Field, N> getColumn(size_t index)const {
        std::array<Field, N> ans;
        for (size_t i = 0; i < N; i++) {
            ans[i] = (*this)[i, index];
        }
        return ans;
    }
    std::array<Field, M> getRow(size_t index)const {
        std::array<Field, M> ans;
        for (size_t i = 0; i < M; i++) {
            ans[i] = (*this)[index, i];
        }
        return ans;
    }

    template <typename S>
    bool operator==(const MatrixExpression<N, M, Field, S>& other)const {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                if (!((*this)[i, j] == other[i, j]))
                    return false;
            }
        }
        return true;
    }

    template <typename S>
    bool operator!=(const MatrixExpression<N, M, Field, S>& other)const {
        return !(*this == other);
    }


};

template <size_t N, size_t M, typename Field,typename T> 
class MatrixOperator :public MatrixExpression < N, M, Field, T>{
public:
    Field val = Field(1);
    
    Field operator[] (size_t i, size_t j)const {
        return static_cast<const T&>(*this).operator[](i, j);
    }


    friend T operator* (const T& arg1, const Field& arg2) {
        T ans = arg1;
        ans.val *= arg2;
        return ans;
    }
    friend T operator* (const Field& arg2, const T& arg1) {
        T ans = arg1;
        ans.val *= arg2;
        return ans;
    }


    friend T operator/ (const T& arg1, const Field& arg2) {
        T ans = arg1;
        ans.val /= arg2;
        return ans;
    }
};

template <size_t N, size_t M, typename Field, typename T1, typename T2, bool isDiff>
class MatrixSumOrDifference :public MatrixOperator<N, M, Field, MatrixSumOrDifference<N, M, Field, T1, T2, isDiff> > {
public:
    std::shared_ptr<T1> ptr1;
    typename std::conditional_t<T1::leaf, const T1&, const T1> arg1;
    std::shared_ptr<T2> ptr2;
    typename std::conditional_t<T2::leaf, const T2&, const T2> arg2;



    MatrixSumOrDifference(const T1& arg1, const T2& arg2) :ptr1(T1::leaf ? new T1(arg1) : 0), arg1(T1::leaf ? *ptr1 : arg1), ptr2(T2::leaf ? new T2(arg2) : 0), arg2(T2::leaf ? *ptr2 : arg2) {

    }

    Field operator[] (size_t i, size_t j)const {
        if constexpr (isDiff) {
            return (arg1[i, j] - arg2[i, j]) * MatrixOperator<N,M,Field,MatrixSumOrDifference<N,M,Field,T1,T2,isDiff> >::val;
        }
        if constexpr (!isDiff) {
            return (arg1[i, j] + arg2[i, j]) * MatrixOperator<N, M, Field, MatrixSumOrDifference<N, M, Field, T1, T2, isDiff> >::val;
        }
    }

    

};

template< size_t N, size_t M, typename Field, typename T1>
class MatrixReverse:public MatrixOperator<N, M, Field, MatrixReverse<N,M,Field, T1> > {
public:
    std::shared_ptr<T1> ptr1;
    typename std::conditional_t<T1::leaf, const T1&, const T1> arg1;

    MatrixReverse(const T1& arg1) :ptr1(T1::leaf ? new T1(arg1) : 0), arg1(T1::leaf ? *ptr1 : arg1) {

    }

    Field operator[] (size_t i, size_t j)const {
        return -arg1[i, j] * MatrixOperator<N, M, Field, MatrixReverse<N, M, Field, T1> >::val;
    }
};

template <size_t N, size_t M, typename Field, typename T1, typename T2>
class MatrixProduct :public MatrixOperator<N, M, Field, MatrixProduct<N, M, Field, T1, T2> > {
public:
    std::shared_ptr<T1> ptr1;
    typename std::conditional_t<T1::leaf, const T1&, const T1> arg1;
    std::shared_ptr<T2> ptr2;
    typename std::conditional_t<T2::leaf, const T2&, const T2> arg2;



    MatrixProduct(const T1& arg1, const T2& arg2) :ptr1(T1::leaf ? new T1(arg1) : 0), arg1(T1::leaf ? *ptr1 : arg1), ptr2(T2::leaf ? new T2(arg2) : 0), arg2(T2::leaf ? *ptr2 : arg2) {}

    Field operator[] (size_t i, size_t j)const {
        return dotProduct(arg1.getRow(i), arg2.getColumn(j))* MatrixOperator<N, M, Field, MatrixProduct<N,M,Field,T1,T2> >::val;
    }

    

};

template <size_t N, size_t M, typename Field, typename T1, typename T2>
using MatrixSum = MatrixSumOrDifference<N, M, Field, T1, T2, false>;

template <size_t N, size_t M, typename Field, typename T1, typename T2>
using MatrixDifference = MatrixSumOrDifference<N, M, Field, T1, T2, true>;

template <size_t N, size_t M, typename Field, typename T, typename S>
MatrixSum<N, M, Field, T, S> operator+(const MatrixExpression<N, M, Field, T>& arg1, const MatrixExpression<N, M, Field, S>& arg2) {


    return MatrixSum<N, M, Field, T, S>(static_cast<const T&> (arg1), static_cast<const S&>(arg2));
}

template <size_t N, size_t M, typename Field, typename T, typename S>
MatrixDifference<N, M, Field, T, S> operator-(const MatrixExpression<N, M, Field, T>& arg1, const MatrixExpression<N, M, Field, S>& arg2) {

    return MatrixDifference<N, M, Field, T, S>(static_cast<const T&> (arg1), static_cast<const S&>(arg2));
}


template <size_t N, size_t M, size_t K, typename Field, typename T, typename S>
MatrixProduct<N, M, Field, T, S> operator*(const MatrixExpression<N, K, Field, T>& arg1, const MatrixExpression<K, M, Field, S>& arg2) {

    return MatrixProduct<N, M, Field, T, S>(static_cast<const T&> (arg1), static_cast<const S&>(arg2));
}


template <size_t N,size_t M,typename Field,typename T>
MatrixReverse<N, M, Field, T> operator-(const MatrixExpression<N,M,Field,T>& arg) {
    return MatrixReverse<N, M, Field, T>(static_cast<const T&> (arg));
}

template<size_t N, size_t M, typename Field = Rational>
class Matrix :public MatrixExpression<N, M, Field, Matrix<N, M, Field> > {
    Field vals[N][M];
    static Field F0;
public:
    static constexpr bool leaf = 1;


    Matrix() {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                vals[i][j] = F0;
            }
        }
    }
    Matrix(const Matrix& other) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                vals[i][j] = other[i, j];
            }
        }
    }

    template<typename T>
    Matrix(const MatrixExpression<N, M, Field, T>& from) {

        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                auto res = from[i, j];
                vals[i][j] = res;
            }
        }
    }

    Matrix(const std::initializer_list<std::initializer_list<long long> > from) {
        size_t i = 0, j = 0;
        for (auto& h : from) {
            for (auto& hh : h) {
                vals[i][j++] = Field(hh);
            }
            j = 0;
            i++;
        }
    }
    Matrix& operator=(const Matrix& other) {
        auto temp = other;
        std::swap(temp.vals, vals);
        return *this;
    }



    Matrix& operator+=(const Matrix& other) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                vals[i][j] += other.vals[i][j];
            }
        }
        return *this;
    }

    

    Matrix& operator-=(const Matrix& other) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                vals[i][j] -= other.vals[i][j];
            }
        }
        return *this;
    }



    Matrix& operator*=(const Field& other) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                vals[i][j] *= other;
            }
        }
        return *this;
    }
    Matrix operator*(const Field& other)const {
        auto ans = *this;
        ans *= other;
        return ans;
    }

    Field& operator[](size_t i, size_t j) {
        return vals[i][j];

    }
    const Field& operator[](size_t i, size_t j)const {
        return vals[i][j];
    }

    Field elSwap(size_t i, size_t j, Matrix* other = 0) {
        auto ci = MatrixExpression<N, M, Field, Matrix<N, M, Field> >::getRow(i);
        auto cj = MatrixExpression<N, M, Field, Matrix<N, M, Field> >::getRow(j);
        for (size_t k = 0; k < M; k++) {
            vals[i][k] = cj[k];
            vals[j][k] = ci[k];
        }
        if (other) {
            other->elSwap(i, j, 0);
        }
        if (i == j)
            return Field(1);
        return Field(-1);
    }
    Field elMultiply(size_t i, const Field& val, Matrix* other = 0) {
        for (size_t k = 0; k < M; k++) {
            vals[i][k] *= val;
        }
        if (other) {
            other->elMultiply(i, val, 0);
        }
        return val;
    }

   
    Field elAdd(size_t to, size_t orig, const Field& coeff, Matrix* other = 0) {
        for (size_t k = 0; k < M; k++) {

            if (vals[orig][k] != F0) {
                vals[to][k] += vals[orig][k] * coeff;
            }
        }
        if (other) {
            other->elAdd(to, orig, coeff, 0);
        }
        return Field(1);
    }

    Field toUpTriang(Matrix* other = 0) {
        size_t curRow = 0;
        Field chg = Field(1);
        for (size_t curCol = 0; curCol < M; curCol++) {
            size_t selRow = SIZE_MAX;
            for (size_t r = curRow; r < N; r++) {
                if (vals[r][curCol] != F0) {
                    selRow = r;
                    break;
                }
            }

            if (selRow == SIZE_MAX)
                continue;
            chg *= elSwap(curRow, selRow, other);
            for (size_t r = curRow + 1; r < N; r++) {
                Field coeff = -(vals[r][curCol] / vals[curRow][curCol]);
                chg *= elAdd(r, curRow, coeff, other);
            }
            curRow++;
        }
        return chg;
    }
    Field trace() {
        static_assert(N == M);
        Field ans = F0;
        for (size_t k = 0; k < N; k++) {
            ans += vals[k][k];
        }
        return ans;
    }

    Field mainDiagProduct() {
        static_assert(N == M);
        Field ans = Field(1);
        for (size_t k = 0; k < N; k++) {
            ans *= vals[k][k];
        }
        return ans;
    }

    Field det() {
        auto mt2 = *this;
        Field proc = mt2.toUpTriang();
        return mt2.mainDiagProduct() / proc;
    }

    void toElem(Matrix* other = 0) {
        for (size_t c = M - 1; c != SIZE_MAX; c--) {
            size_t r = N - 1;
            for (; r != SIZE_MAX; r--) {
                if (vals[r][c] != F0) {
                    break;
                }
            }
           
            if (r != SIZE_MAX) {
                for (size_t rr = r - 1; rr != SIZE_MAX; rr--) {
                    Field coeff = -(vals[rr][c] / vals[r][c]);
                    elAdd(rr, r, coeff, other);
                }

            }

        }
        for (size_t r = 0; r < N; r++) {
            for (size_t c = 0; c < M; c++) {
                if (vals[r][c] != F0) {
                    elMultiply(r, Field(1) / vals[r][c], other);
                }
            }
        }
    }

    static Matrix unityMatrix() {
        static_assert(N == M);
        Matrix<N, N, Field> ans;
        for (size_t i = 0; i < N; i++) {
            ans[i, i] = 1;
        }
        return ans;

    }

    void invert() {
        static_assert(N == M);
        auto ans = Matrix::unityMatrix();
        toUpTriang(&ans);
        toElem(&ans);
        (*this) = ans;
    }
    Matrix inverted() const {
        auto ans = *this;
        ans.invert();
        return ans;
    }

    Matrix<M, N, Field> transposed()const {
        Matrix<M, N, Field> ans;
        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < M; j++) {
                ans[j, i] = vals[i][j];
            }
        }
        return ans;
    }

    size_t rank()const {
        auto temp = *this;
        temp.toUpTriang();

        size_t r = 0;
        size_t c = 0;
        size_t ans = 0;
        while (r < N && c < M) {
            if (temp[r, c] != F0) {
                ans++;
                r++;
                c++;
            }
            else {
                c++;
            }
        }
        return ans;
    }


};

template<size_t N, size_t M, typename Field>
Field Matrix<N, M, Field>::F0 = Field(0);

template <typename Field, size_t N, size_t M>
Matrix<N, M, Field> operator*(Field a, const Matrix<N, M, Field>& b) {
    return b * a;
}

template <size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;






template<size_t N, typename Field>
SquareMatrix<N, Field>& operator*=(SquareMatrix<N, Field>& a, const SquareMatrix<N, Field>& b) {
    a = a * b;
    return a;
}


