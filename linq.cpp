#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>
#include <cassert>
#include <memory>

class FibIterator {
    public:
        using Item = int;
        FibIterator(Item first, Item second): first_(first), second_(second) {}
        FibIterator& operator++() {
            Item sum = first_ + second_;
            first_ = second_;
            second_ = sum;
            return *this;
        }
        Item operator*() const {
            return first_;
        }
        bool is_empty() const {
            return is_empty_;
        }
    private:
        Item first_;
        Item second_;
        bool is_empty_ = false;
};

class WordCountIterator {
    public:
        using Item = std::string;
        WordCountIterator(const std::vector<Item>::iterator& begin, const std::vector<Item>::iterator& end): begin_(begin), end_(end) {}
        WordCountIterator& operator++() {
            ++begin_;
            if (begin_ == end_) {
                is_empty_ = true;
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return *begin_; 
        }
        bool is_empty() const {
            return is_empty_;
        }
    private:
        std::vector<Item>::iterator begin_;
        std::vector<Item>::iterator end_;
        bool is_empty_ = false;
};

template<typename Iterator, typename Key>
class GroupBy {
    public:
        typedef std::pair<Key, std::vector<typename Iterator::Item> > Item;
        GroupBy(const Iterator& iter, const std::function<Key(typename Iterator::Item)>& key_generator):
            iter_(iter), key_generator_(key_generator) {
                Initialize();
        }
        GroupBy(const GroupBy& g): iter_(g.iter_), key_generator_(g.key_generator_), is_empty_(g.is_empty_), mapper_(g.mapper_), map_iterator_(mapper_.begin()) {}
        GroupBy<Iterator, Key>& operator++() {
            ++map_iterator_;

            auto iter = map_iterator_;
            ++iter;
            if (iter == mapper_.end()) {
                is_empty_ = true;
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return *map_iterator_;
        }
        bool is_empty() const {
            return is_empty_;
        }

    private:
        Iterator iter_;
        std::function<Key(typename Iterator::Item)> key_generator_;
        bool is_empty_ = false;
        std::map<Key, std::vector<typename Iterator::Item> > mapper_{};
        typename std::map<Key, std::vector<typename Iterator::Item> >::iterator map_iterator_{};

        void Initialize() {
            while (!iter_.is_empty()) {
                auto value = *iter_;
                mapper_[key_generator_(value)].push_back(value);
                ++iter_;
            }
            map_iterator_ = mapper_.begin();
        }
};

template<typename Iterator, typename Key>
class OrderBy {
    public:
        using Item = typename Iterator::Item;
        OrderBy(const Iterator& iter, const std::function<Key(typename Iterator::Item)>& sorter, bool desc):
            iter_(iter), sorter_(sorter), desc_(desc) {
                Initialize();
        }
        OrderBy(const OrderBy& g): iter_(g.iter_), sorter_(g.sorter_), is_empty_(g.is_empty_), mapper_(g.mapper_), map_iterator_(mapper_.begin()), desc_(g.desc_) {}
        OrderBy<Iterator, Key>& operator++() {
            ++map_iterator_;

            auto iter = map_iterator_;
            ++iter;
            if (iter == mapper_.end()) {
                is_empty_ = true;
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return (*map_iterator_).second;
        }
        bool is_empty() const {
            return is_empty_;
        }

    private:
        Iterator iter_;
        std::function<Key(typename Iterator::Item)> sorter_;
        bool is_empty_ = false;
        std::vector<std::pair<Key, typename Iterator::Item> > mapper_{};
        typename std::vector<std::pair<Key, typename Iterator::Item> >::iterator map_iterator_{};
        bool desc_;

        void Initialize() {
            while (!iter_.is_empty()) {
                auto value = *iter_;
                mapper_.push_back({sorter_(value),value});
                ++iter_;
            }
            if (desc_) {
               std::sort(mapper_.rbegin(), mapper_.rend()); 
            } else {
                std::sort(mapper_.begin(), mapper_.end());
            }
            map_iterator_ = mapper_.begin();
        }
};

template<typename Iterator>
class Flatten {
    public:
        using Item = typename std::remove_reference<decltype(std::declval<typename Iterator::Item>().front())>::type;
        Flatten(const Iterator& iter):
            iter_(iter), current_(*iter), entity_iter_((current_).begin()) {
                entity_iter_ = current_.begin();
                assert (current_.begin() == entity_iter_);
        }
        Flatten(const Flatten& fl): iter_(fl.iter_), current_(fl.current_), entity_iter_((current_).begin())  {

        }
        Flatten<Iterator>& operator++() {
            ++entity_iter_;
            if (iter_.is_empty() && entity_iter_ == current_.end()) {
                is_empty_ = true;
                return *this;
            }
            if (entity_iter_ == current_.end()) {
                do {
                    ++iter_;
                    if (iter_.is_empty()) {
                        is_empty_ = true;
                        return *this;
                    }
                    current_ = *iter_;
                    entity_iter_ = current_.begin();
                } while (current_.size() == 0);
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return *entity_iter_;
        }
        bool is_empty() const {
            return is_empty_ && iter_.is_empty();
        }
    private:
        Iterator iter_;
        bool is_empty_ = false;
        typename Iterator::Item current_;
        decltype(std::declval<typename Iterator::Item>().begin()) entity_iter_;

};

template<typename Iterator>
class Where {
    public:
        using Item = typename Iterator::Item;
        Where(const Iterator& iter, const std::function<bool(typename Iterator::Item)>& filter):
            iter_(iter), filter_(filter) {}
        Where<Iterator>& operator++() {
            if (iter_.is_empty()) {
                is_empty_ = true;
                return *this;
            }
            ++iter_;
            if (iter_.is_empty()) {
                is_empty_ = true;
                return *this;
            }
            while (!filter_(*iter_)) {
                if (iter_.is_empty()) {
                    is_empty_ = true;
                    return *this;
                }
                ++iter_;
                if (iter_.is_empty()) {
                    is_empty_ = true;
                    return *this;
                }
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return *iter_;
        }
        bool is_empty() const {
            return is_empty_ || iter_.is_empty();
        }

    private:
        Iterator iter_;
        std::function<bool(typename Iterator::Item)> filter_;
        bool is_empty_ = false;
};

template<typename Iterator, typename Result>
class Select {
    public:
        using Item = Result;
        Select(const Iterator& iter, const std::function<Result(typename Iterator::Item)>& transformer):
            iter_(iter), transformer_(transformer) {}
        Select<Iterator, Result>& operator++() {
            ++iter_;
            if (iter_.is_empty()) {
                is_empty_ = true;
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return transformer_(*iter_);
        }
        bool is_empty() const {
            return is_empty_ || iter_.is_empty();
        }

    private:
        Iterator iter_;
        std::function<Result(typename Iterator::Item)> transformer_;
        bool is_empty_ = false;
};

template<typename Iterator>
class Take {
    public:
        using Item = typename Iterator::Item;
        Take(const Iterator& iter, int count):
            iter_(iter), count_(count) {}
        Take<Iterator>& operator++() {
            current_++;
            if (current_ >= count_) {
                is_empty_ = true;
                return *this;
            } else if (iter_.is_empty()) {
                is_empty_ = true;
                return *this;
            } else {
                ++iter_;
            }
            return *this;
        }
        Item operator*() const {
            assert(!is_empty());
            return *iter_;
        }
        bool is_empty() const {
            return is_empty_ || iter_.is_empty();
        }

    private:
        Iterator iter_;
        int count_;
        bool is_empty_ = false;
        int current_ = 0;
};

template <typename Iterator>
class Range {
    using Item = typename Iterator::Item;
    public:
        Range(const Iterator& iter): iter_(iter) {}
        Range<Where<Iterator>> where(const std::function<bool(Item)>& filter) {
            return Range<Where<Iterator>>(Where<Iterator>(iter_, filter));
        }
        Range<Flatten<Iterator>> flatten() {
            return Range<Flatten<Iterator>>(Flatten<Iterator>(iter_));
        }
        template <typename Result>
        Range<Select<Iterator, Result>> select(const std::function<Result(Item)>& filter) {
            return Range<Select<Iterator, Result>>(Select<Iterator, Result>(iter_, filter));
        }
        Range<Take<Iterator>> take(int count) {
            return Range<Take<Iterator>>(Take<Iterator>(iter_, count));
        }
        template <typename Key>
        Range<GroupBy<Iterator, Key>> groupBy(const std::function<Key(Item)>& key_generator) {
            return Range<GroupBy<Iterator, Key>>(GroupBy<Iterator, Key>(iter_, key_generator));
        }
        template <typename Key>
        Range<OrderBy<Iterator, Key>> orderBy(const std::function<Key(Item)>& sorter, bool desc) {
            return Range<OrderBy<Iterator, Key>>(OrderBy<Iterator, Key>(iter_, sorter, desc));
        }
        std::vector<Item> toList() {
            std::vector<Item> items;
            for (; !iter_.is_empty(); ++iter_) {
                if (iter_.is_empty()) {
                    break;
                }
                items.push_back(*iter_);
            }
            return items;
        }
    public:
        Iterator iter_;
};


int main() {
    FibIterator fib(0, 1);

    auto f = Range<FibIterator>(fib).where([&](int x) {return x % 3 == 0;}).select<int>([&](int x) {return (x % 2 == 0) ? x * x: x;}).take(5).toList();

    std::cout << "Fibbonachi nums is ";
    for (const auto& elem: f) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    std::vector<std::string> text = {
        "So why, when we\'re learning English",
        "or learning a language, do we say",
        "Oh, I don\'t want to study this any more,",
        "I already know it. I already know the present tense.",
        "I already know these words. I already know this grammar.",
        "I already know this lesson. What, maybe you listened to it",
        "7 times, 5 times? It\'s not enough. We need a lot of",
        "repetition, a lot of repetition with power, with intensity.",
        "Your whole life, that\'s why babies learn so quickly and so well,",
        "that\'s why I\'m a master of English and not a master of Spanish.",
        "Because every day for 40 years I have been listening to English, the same basic English.",
        "So why,", "So why,", "So why,", "So why,", "So why,", "So why,", "So why,",
    };
    WordCountIterator wc(text.begin(), text.end());
    auto r = Range<WordCountIterator>(wc).select<std::vector<std::string>>([&](std::string str) {
        std::vector<std::string> strs;
        std::string delimiter = " ";
        int start = 0;
        int ind = 0;
        while (ind != str.size()) {
            ind = str.find(delimiter, start);
            if (ind == -1) {
                ind = str.size();
            }
            std::string token = str.substr(start, ind - start);
            start = ind + 1;
            strs.push_back(token);
        }
        return strs;
    }).flatten()
        .groupBy<std::string>([&](const std::string& str) {return str;})
        .select<std::pair<std::string, int> >([&](const std::pair<std::string, std::vector<std::string>>& p) { return std::make_pair(p.first, (int)p.second.size());})
        .orderBy<int>([&](const std::pair<std::string, int>& p) {return p.second;}, /*desc*/ true).toList();
    
    std::cout << "===========================================" << std::endl;
    std::cout << "Word Count is " << std::endl;
    for (const auto& elem: r) {
        std::cout << elem.first << ' ' << elem.second << std::endl;
    }
}
