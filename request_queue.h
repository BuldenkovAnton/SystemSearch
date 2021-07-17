#pragma once
#include <string>
#include <vector>
#include <deque>

#include "search_server.h"
#include "document.h"

class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& search_server);

	template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

	std::vector<Document> AddFindRequest(const std::string& raw_query);

	int GetNoResultRequests() const;

private:
	struct QueryResult {
		explicit QueryResult(const std::vector<Document>& d);

		const std::vector<Document> docs;
	};

	std::deque<QueryResult> requests_;
	const static int sec_in_day_ = 1440;
	const SearchServer& server_;
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
	const auto result = server_.FindTopDocuments(raw_query, document_predicate);
	while(requests_.size() > sec_in_day_) {
		requests_.pop_front();
	}
	requests_.push_back(QueryResult(result));
	if (requests_.size() > sec_in_day_) requests_.pop_front();
	return result;
}
