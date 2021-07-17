#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>

#include "document.h"
#include "paginator.h"
#include "search_server.h"
#include "request_queue.h"

template<typename Key, typename Value>
std::ostream& operator <<(std::ostream& os, const std::pair<Key, Value>& container) {
	os << container.first << ": " << container.second;
	return os;
}

template<typename T, typename U>
std::ostream& operator <<(std::ostream& os, const std::map<T, U>& container) {
	bool isFirst = true;
	os << '{';
	for (const auto& item : container) {
		if (isFirst) {
			os << item;
			isFirst = false;
		}
		else {
			os << std::string(", ") << item;
		}
	}
	os << '}';
	return os;
}

template<typename T>
std::ostream& operator <<(std::ostream& os, const std::vector<T>& container) {
	bool isFirst = true;
	os << '[';
	for (const auto& item : container) {
		if (isFirst) {
			os << item;
			isFirst = false;
		}
		else {
			os << std::string(", ") << item;
		}
	}
	os << ']';
	return os;
}

template<typename T>
std::ostream& operator <<(std::ostream& os, const std::set<T>& container) {
	bool isFirst = true;
	os << '{';
	for (const auto& item : container) {
		if (isFirst) {
			os << item;
			isFirst = false;
		}
		else {
			os << std::string(", ") << item;
		}
	}
	os << '}';
	return os;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
	const std::string& func, unsigned line, const std::string& hint) {
	if (t != u) {
		std::cerr << std::boolalpha;
		std::cerr << file << '(' << line << std::string("): ") << func << std::string(": ");
		std::cerr << std::string("ASSERT_EQUAL(") << t_str << std::string(", ") << u_str << std::string(") failed: ");
		std::cerr << t << std::string(" != ") << u << '.';
		if (!hint.empty()) {
			std::cerr << std::string(" Hint: ") << hint;
		}
		std::cerr << std::endl;
		abort();
	}
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))


template <typename T>
void AssertImpl(const T& str, const std::string& s, const std::string& file_name, const std::string& function_name, unsigned line, const std::string& hint) {
	if (str == false) {
		std::cerr << file_name << '(' << line << std::string("): ");
		std::cerr << function_name << std::string(": ASSERT(") << s << std::string(") failed.");

		if (!hint.empty()) {
			std::cerr << std::string(" Hint: ") << hint;
		}
		std::cerr << std::endl;
		abort();
	}
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))


template <typename T>
void RunTestImpl(const T& expr, const std::string& function_name) {
	expr();
	std::cerr << function_name << std::string(" OK") << std::endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func)


void TestConstructor();

void TestExcludeStopWordsFromAddedDocumentContent();

void TestAddDocument();

void TestFindDocumentsException();

void TestMinusStop();

void TestMatching();

void TestMatchingException();

void TestSortRelevance();

void TestRating();

void TestFindByPredicate();

void TestFindByStatus();

void TestRelevance();

void TestException();

void TestGetDocumentIDException();

void TestPagination();

void TestRequestQueue();

void TestGetWordFrequencies();

void TestRemoveDocument();

void TestDeleteDuplicate();

void TestSearchServer();


