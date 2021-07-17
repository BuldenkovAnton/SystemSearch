#include "test_example_functions.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

#include "document.h"
#include "search_server.h"
#include "paginator.h"
#include "request_queue.h"
#include "remove_duplicates.h"

using namespace std;

void TestConstructor() {
	try {
		SearchServer server("in t\x12he"s);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "words has bad symbols"s, "stop words has bad symbols"s);
	}
}

void TestExcludeStopWordsFromAddedDocumentContent() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Must be find one document"s);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL_HINT(doc0.id, doc_id, "ID must be compare"s);
	}
	{
		SearchServer server("in the"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
	}
}

void TestAddDocument() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		auto found_docs = server.FindTopDocuments("cat city"s);
		ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Must be find one document"s);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL_HINT(doc0.id, doc_id, "ID must be compare"s);
	}
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::BANNED, ratings);
		auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Must be empty. Default find by status actual, we add document by status Banned"s);
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(-1, content, DocumentStatus::ACTUAL, ratings);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "invalid document id"s, "ID must be > 0"s);
		}
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "invalid document id"s, "ID must be different"s);
		}
	}
	{
		const string content1 = "cat in the ci\x12ty"s;
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content1, DocumentStatus::ACTUAL, ratings);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "words has bad symbols"s, "content has words with bad symbols"s);
		}
	}
}

void TestFindDocumentsException() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.FindTopDocuments("c\x12ity"s);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query has bad symbols"s);
		}
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.FindTopDocuments("city --the"s);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query has many -"s);
		}
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.FindTopDocuments("city - "s);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query hasn't word after -"s);
		}
	}
}

void TestMinusStop() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT_HINT(server.FindTopDocuments("city -cat"s).empty(), "Must be empty. In query find -word"s);
	}
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		auto found_docs = server.FindTopDocuments("cat -dog"s);
		ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Must be find one document"s);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL_HINT(doc0.id, doc_id, "ID must be compare"s);
	}
}

void TestMatching() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		const vector<string> answer = { "cat"s, "city"s };
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto [words, status] = server.MatchDocument("cat city"s, doc_id);

		vector<string> result;
		for (const auto& word : words) {
			result.push_back(string(word));
		}

		ASSERT_EQUAL_HINT(status, DocumentStatus::ACTUAL, "We add document by status Actual"s);
		ASSERT_EQUAL_HINT(result.size(), answer.size(), "Not equal return words"s);
	}
	{
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::BANNED, ratings);
		const auto [words, status] = server.MatchDocument("cat -city"s, doc_id);
		ASSERT_EQUAL_HINT(status, DocumentStatus::BANNED, "We add document by status Banned"s);
		ASSERT_HINT(words.empty(), "Return words not empty!"s);
	}
}


void TestMatchingException() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.MatchDocument("c\x12ity"s, doc_id);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query in matching has bad symbols"s);
		}
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.MatchDocument("city --the"s, doc_id);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query in matching has many -"s);
		}
	}
	{
		try {
			SearchServer server(""s);
			server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
			server.MatchDocument("city - "s, doc_id);
		}
		catch (const exception& e) {
			ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query in matching hasn't word after -"s);
		}
	}
}


void TestSortRelevance() {
	const vector<int> answer = { 1, 0, 2 };

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	vector<int> found_docs_id;
	for (const auto& doc : server.FindTopDocuments("пушистый ухоженный кот"s)) {
		found_docs_id.push_back(doc.id);
	}

	ASSERT_EQUAL_HINT(found_docs_id, answer, "Documents sort aren't correct"s);
}

void TestRating() {
	const vector<int> answer = { 5, 2, -1 };

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	vector<int> found_docs_rating;
	for (const auto& doc : server.FindTopDocuments("пушистый ухоженный кот"s)) {
		found_docs_rating.push_back(doc.rating);
	}

	ASSERT_EQUAL_HINT(found_docs_rating, answer, "Documents rating aren't correct"s);
}

void TestFindByPredicate() {
	const vector<int> answer = { 1, 3, 0 };

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	const vector<Document> found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, []([[maybe_unused]] int document_id, [[maybe_unused]] DocumentStatus status, int rating)
		{
			return rating > 1;
		});

	vector<int> found_docs_id;
	for (const auto& doc : found_docs) {
		found_docs_id.push_back(doc.id);
	}

	ASSERT_EQUAL_HINT(found_docs_id, answer, "Documents find by predicate not correct"s);
}

void TestFindByStatus() {
	const int answer = 3;

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });


	const vector<Document> found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
	ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Must be one document"s);
	const Document& doc0 = found_docs[0];
	ASSERT_EQUAL_HINT(doc0.id, answer, "Must be one document with status Banned"s);
}

void TestRelevance() {

	const vector<double> answer = { 0.866434, 0.173287, 0.173287 };

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	const vector<Document> found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
	ASSERT_EQUAL_HINT(found_docs.size(), 3u, "Must be three documents"s);

	for (size_t i = 0; i < found_docs.size(); ++i) {
		ASSERT_HINT(abs(found_docs.at(i).relevance - answer.at(i)) <= EPSILON, "Documents relevance are not correct"s);
	}
}

void TestException() {
	SearchServer server("и в на"s);
	cerr << "Exception" << endl;
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	try {
		server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "invalid document id"s, "document width id already add"s);
	}

	try {
		server.AddDocument(-1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "invalid document id"s, "document id < 0"s);
	}


	try {
		server.AddDocument(3, "большой пёс скво\x12рец евгений"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "words has bad symbols"s, "words in document has bad symbols"s);
	}

	try {
		server.AddDocument(3, "большой пёс скво\x12рец евгений"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "invalid document id"s, "words in document has bad symbols"s);
	}

	try {
		server.FindTopDocuments("пушистый --кот"s);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query isn't correct"s);
	}

	try {
		server.FindTopDocuments("пушистый -"s);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "query isn't correct"s);
	}

	try {
		server.MatchDocument("модный --пёс"s, 1);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "matching: query isn't correct"s);
	}

	try {
		server.MatchDocument("пушистый - хвост"s, 1);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "query isn't correct"s, "matching: query isn't correct"s);
	}

}

void TestGetDocumentIDException() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	try {
		SearchServer server(""s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
	}
	catch (const exception& e) {
		ASSERT_EQUAL_HINT(e.what(), "vector::_M_range_check: __n (which is 5) >= this->size() (which is 1)"s, "documents not have document by index"s);
	}
}

void TestPagination() {
	SearchServer server("and with"s);

	server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

	const auto search_results = server.FindTopDocuments("curly dog"s);
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);
	ASSERT_EQUAL_HINT(pages.end() - pages.begin(), 2, "Interval not correct"s);

}

void TestRequestQueue() {

	SearchServer server("и в на"s);
	RequestQueue request_queue(server);

	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::BANNED, { 1, 3, 2 });
	server.AddDocument(5, "большой пёс василий"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
	ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 0, "Must be empty queue"s);

	for (int i = 0; i < 2000; ++i) {
		request_queue.AddFindRequest("пустой запрос"s);
	}
	ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 1440, "Must be 1440 in queue"s);

	request_queue.AddFindRequest("-большой -ошейник"s);
	ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 1440, "Must be 1440 in queue"s);

	request_queue.AddFindRequest("пушистый пёс"s);
	ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 1439, "Must be 1439 in queue"s);

	request_queue.AddFindRequest("скворец"s);
	ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 1439, "Must be 1439 in queue"s);

}

void TestGetWordFrequencies() {
	map<string, double> answer = { {"funny"s, 0.25}, {"nasty"s, 0.25}, {"pet"s, 0.25}, {"rat"s, 0.25} };
	SearchServer server("and with"s);
	server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });

	map<string_view, double> result = server.GetWordFrequencies(1);
	map<string, double> result_for_compair;
	for (auto& [word, value] : result) {
		result_for_compair.insert({string(word), value});
	}
	ASSERT_EQUAL_HINT(result_for_compair, answer, "Wrong offten words"s);
}

void TestRemoveDocument() {
	SearchServer server("and with"s);
	server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1, "Must be add document"s);

	server.RemoveDocument(1);
	ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "Must be remove document"s);
}

void TestDeleteDuplicate() {

	SearchServer server("and with"s);
	server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
	{
		server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
		ASSERT_EQUAL_HINT(server.GetDocumentCount(), 3, "Must be add duplicate"s);

		RemoveDuplicates(server, false);
		ASSERT_EQUAL_HINT(server.GetDocumentCount(), 2, "Must be delete duplicate with id = 3"s);
	}
	{
		server.AddDocument(3, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
		ASSERT_EQUAL_HINT(server.GetDocumentCount(), 3, "Must be add. Not duplicate"s);

		server.AddDocument(4, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
		ASSERT_EQUAL_HINT(server.GetDocumentCount(), 4, "Must be add duplicate"s);

		RemoveDuplicates(server, false);
		ASSERT_EQUAL_HINT(server.GetDocumentCount(), 3, "Must be delete duplicate with id = 4"s);
	}
}

void TestSearchServer() {
	RUN_TEST(TestConstructor);
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestAddDocument);
	RUN_TEST(TestFindDocumentsException);
	RUN_TEST(TestMinusStop);
	RUN_TEST(TestMatching);
	RUN_TEST(TestMatchingException);
	RUN_TEST(TestSortRelevance);
	RUN_TEST(TestRating);
	RUN_TEST(TestFindByPredicate);
	RUN_TEST(TestFindByStatus);
	RUN_TEST(TestRelevance);
	RUN_TEST(TestException);
	RUN_TEST(TestGetDocumentIDException);
	RUN_TEST(TestPagination);
	RUN_TEST(TestRequestQueue);
	RUN_TEST(TestGetWordFrequencies);
	RUN_TEST(TestRemoveDocument);
	RUN_TEST(TestDeleteDuplicate);
	cerr << "Search server testing finished"s << endl;
}

