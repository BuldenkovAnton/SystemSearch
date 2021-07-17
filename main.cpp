#include "process_queries.h"
#include "search_server.h"
#include "string_processing.h"
#include "test_example_functions.h"

#include <execution>
#include <iostream>
#include <string>
#include <vector>

using namespace std;



int main() {
	TestSearchServer();
	SearchServer search_server("and with"s);

	search_server.AddDocument(0, "white cat and yellow hat"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(2, "nasty dog with big eyes"s, DocumentStatus::ACTUAL, { 1, 2 });
	search_server.AddDocument(3, "nasty pigeon john"s, DocumentStatus::ACTUAL, { 1, 2 });

	cout << "ACTUAL by default:"s << endl;

	for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
		PrintDocument(document);
	}
	cout << "BANNED:"s << endl;

	for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}

	cout << "Even ids:"s << endl;

	for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
		PrintDocument(document);
	}

	return 0;
}
