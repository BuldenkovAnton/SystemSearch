#include "request_queue.h"
#include <vector>
#include <string>
#include <algorithm>
#include <deque>

#include "search_server.h"
#include "document.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server)
	: server_(search_server) {}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
	return  RequestQueue::AddFindRequest(raw_query,
			[status]([[maybe_unused]] int document_id, DocumentStatus document_status, [[maybe_unused]] int rating) {
				return document_status == status;
			});
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
	return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
	return count_if(requests_.begin(), requests_.end(), [](const QueryResult& item) {
		return item.docs.size() == 0;
		});
}

RequestQueue::QueryResult::QueryResult(const std::vector<Document>& d)
	: docs(d) {}
