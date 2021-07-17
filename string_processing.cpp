#include "string_processing.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

#include "document.h"
#include "search_server.h"

using namespace std;


void PrintDocument(const Document& document) {
	cout << "{ "s
		<< "document_id = "s << document.id << ", "s
		<< "relevance = "s << document.relevance << ", "s
		<< "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string_view>& words, DocumentStatus status) {
	cout << "{ "s
		<< "document_id = "s << document_id << ", "s
		<< "status = "s << static_cast<int>(status) << ", "s
		<< "words ="s;
	for (const string_view word : words) {
		cout << ' ' << word;
	}
	cout << "}"s << endl;
}

void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,
	const vector<int>& ratings) {
	try {
		search_server.AddDocument(document_id, document, status, ratings);
	}
	catch (const exception& e) {
		cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
	}
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
	cout << "Результаты поиска по запросу: "s << raw_query << endl;
	try {
		for (const Document& document : search_server.FindTopDocuments(raw_query)) {
			PrintDocument(document);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка поиска: "s << e.what() << endl;
	}
}

void MatchDocuments( SearchServer& search_server, string_view query) {
	try {
		cout << "Матчинг документов по запросу: "s << query << endl;
		for (auto it = search_server.begin(); it != search_server.end(); it++) {
			const auto [words, status] = search_server.MatchDocument(query, *it);
			PrintMatchDocumentResult(*it, words, status);
		}
	}
	catch (const exception& e) {
		cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
	}
}
