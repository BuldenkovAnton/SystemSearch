#pragma once
#include <iostream>

template <typename It>
class IteratorRange {
public:

	IteratorRange(It range_begin, It range_end)
		: it_begin(range_begin)
		, it_end(range_end) {}

	auto begin() const {
		return it_begin;
	}

	auto end() const {
		return it_end;
	}

	int size() {
		return distance(it_begin, it_end);
	}

private:
	It it_begin;
	It it_end;
};


template <class It>
class Paginator {
public:
	Paginator(It range_begin, It range_end, size_t page_size) {
			It start, bIt = range_begin;
			size_t c = 1;


			for (start = range_begin; start != range_end; start++) {

				if (c > page_size) {
					pages_.push_back({ bIt, start });
					bIt = start;
					c = 1;
				}
				c++;
			}

			if (bIt != range_end) {
				pages_.push_back({ bIt, range_end });
			}
		}

	auto begin() const {
		return pages_.begin();
	}

	auto end() const {
		return pages_.end();
	}
private:
	std::vector<IteratorRange<It>> pages_;
};

template<typename It>
std::ostream& operator<< (std::ostream& out, const IteratorRange<It> item) {
	for (auto it = item.begin(); it != item.end(); ++it) {
		out << *it;
	}
	return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
	return Paginator(std::begin(c), std::end(c), page_size);
}
