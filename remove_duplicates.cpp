#include "remove_duplicates.h"
#include "search_server.h"

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <execution>

using namespace std;

void RemoveDuplicates(SearchServer& search_server, bool show_hint) {
	std::set<std::set<std::string_view>> existing_docs;
	std::vector<int> found_duplicates;

	for (int document_id : search_server) {
		auto& freqs = search_server.GetWordFrequencies(document_id);
		std::set<std::string_view> words;

		std::transform(
				freqs.begin(), freqs.end(),
				std::inserter(words, words.begin()),
				[](auto p) {
					return p.first;
		});

		if (existing_docs.count(words) > 0) {
			if (show_hint){
				cout << "Found duplicate document id " << document_id << endl;
			}

			found_duplicates.push_back(document_id);
		} else {
			existing_docs.insert(words);
		}
	}

	for (int id : found_duplicates) {
		search_server.RemoveDocument(id);
	}
}
