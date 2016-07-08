#include <sstream>
#include <string>
#include <vector>


auto split(const std::string &s, char delim = ' ') {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.emplace_back(std::move(item));
        }
    }
    return elems;
}

// Convenience functions returning iterators to keys and values of an
// associative container.

namespace {

template <typename Iterator>
class IteratorBase {
public:
    using difference_type = typename Iterator::difference_type;
    using iterator_category = typename Iterator::iterator_category;

    explicit IteratorBase(Iterator it) : it_(it) {}
    auto& operator++() { ++it_; return *this; }
    auto operator==(IteratorBase other) { return it_ == other.it_; }
    auto operator!=(IteratorBase other) { return !(*this == other); }

protected:
    Iterator it_;
};


template <typename Iterator>
class KeysIterator : public IteratorBase<Iterator> {
public:
    using value_type = typename Iterator::value_type::first_type;
    using pointer = value_type*;
    using reference = value_type&;

    using IteratorBase<Iterator>::IteratorBase;
    auto operator*() { return IteratorBase<Iterator>::it_->first; }
};

template <typename Iterator>
class ValuesIterator : public IteratorBase<Iterator> {
public:
    using value_type = typename Iterator::value_type::second_type;
    using pointer = value_type*;
    using reference = value_type&;

    using IteratorBase<Iterator>::IteratorBase;
    auto operator*() { return IteratorBase<Iterator>::it_->second; }
};

template <typename Container, template<typename T> class WrapperIterator>
struct IteratorWrapper {
    using Iterator = typename Container::const_iterator;
    explicit IteratorWrapper(Container& c) : c_(c) {}
    auto begin() { return WrapperIterator<Iterator>{c_.begin()}; }
    auto end() { return WrapperIterator<Iterator>{c_.end()}; }

private:
    Container& c_;
};

} // namespace anonymous


template <typename Container>
auto keys(Container& c) {
    return IteratorWrapper<Container, KeysIterator>(c);
}

template <typename Container>
auto values(Container& c) {
    return IteratorWrapper<Container, ValuesIterator>(c);
}


template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

namespace std {

// pair hash
template <typename T1, typename T2>
struct hash<std::pair<T1, T2>> {
    auto operator()(const std::pair<T1, T2>& p) const {
        size_t seed = 0;
        hash_combine(seed, p.first);
        hash_combine(seed, p.second);
        return seed;
    }
};

} // namespace std
