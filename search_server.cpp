#include "search_server.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <exception>
#include <execution>

#include "document.h"
#include "read_input_functions.h"

using namespace std;

SearchServer::SearchServer(string_view stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text)) {}

SearchServer::SearchServer(const string& stop_words_text)
	: SearchServer(SplitIntoWords(stop_words_text)) {}


set<int>::iterator SearchServer::begin() {
	return document_ids_.begin();
}

set<int>::iterator SearchServer::end() {
	return document_ids_.end();
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
	static const map<string_view, double> empty_map = {};
	if (document_to_words_.count(document_id) == 0) return empty_map;
	return document_to_words_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
	return RemoveDocument(execution::seq, document_id);
}

void SearchServer::RemoveDocument(const execution::sequenced_policy&, int document_id) {
	if (document_to_words_.count(document_id) == 0) {
		return;
	}

	for (auto& [word, freq] : document_to_words_.at(document_id)) {
		word_to_document_freqs_.at(word).erase(document_id);
	}

	document_ids_.erase(document_id);
	documents_.erase(document_id);

	document_to_words_.erase(document_id);
}

void SearchServer::RemoveDocument(const execution::parallel_policy&, int document_id) {
	if (document_to_words_.count(document_id) == 0) {
		return;
	}

	document_ids_.erase(document_id);
	documents_.erase(document_id);

	const auto& words = document_to_words_.at(document_id);
	vector<string_view> words_for_delete(words.size());
	transform(
		execution::par,
		words.begin(), words.end(),
		words_for_delete.begin(),
		[](const auto& item) {
			return item.first;
		}
	);
	for_each(
		execution::par,
		words_for_delete.begin(), words_for_delete.end(),
		[this, document_id](string_view word) {
			word_to_document_freqs_.at(word).erase(document_id);
		});

	document_to_words_.erase(document_id);
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
	int rating_sum = 0;
	if (ratings.empty()) return 0;
	for (const int rating : ratings) {
		rating_sum += rating;
	}
	return rating_sum / static_cast<int>(ratings.size());
}


bool SearchServer::IsValidWord(string_view word) {
	return none_of(word.begin(), word.end(), [](char c) {
		return c >= '\0' && c < ' ';
		});
}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
	if ((document_id < 0) || (documents_.count(document_id) > 0)) {
		throw invalid_argument("invalid document id");
	}

	const auto [it, inserted] = documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status, string(document) });
	const vector<string_view> words = SplitIntoWordsNoStop(it->second.text);
	const double inv_word_count = 1.0 / words.size();

	for (const string_view word : words) {
		word_to_document_freqs_[word][document_id] += inv_word_count;
		document_to_words_[document_id][word] += inv_word_count;
	}

	document_ids_.insert(document_id);
}

int SearchServer::GetDocumentCount() const {
	return documents_.size();
}


tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
	return MatchDocument(execution::seq, raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::sequenced_policy&, string_view raw_query, int document_id) const {
	const auto query = ParseQuery(raw_query);

	for (const string_view word : query.minus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			return { {},  documents_.at(document_id).status };
		}
	}

	vector<string_view> result_words;
	for (const string_view word : query.plus_words) {
		if (word_to_document_freqs_.count(word) == 0) {
			continue;
		}
		if (word_to_document_freqs_.at(word).count(document_id)) {
			result_words.push_back(word);
		}
	}
	return { result_words,  documents_.at(document_id).status };



}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::parallel_policy&, string_view raw_query, int document_id) const {
	const auto query = ParseQuery(raw_query, true);

	const auto word_checker =
		[this, document_id](string_view word) {
		const auto it = word_to_document_freqs_.find(word);
		return it != word_to_document_freqs_.end() && it->second.count(document_id);
	};

	if (any_of(execution::par, query.minus_words.begin(), query.minus_words.end(), word_checker)) {
		return { {}, documents_.at(document_id).status };
	}

	vector<string_view> result_words(query.plus_words.size());
	auto words_end = copy_if(
		execution::par,
		query.plus_words.begin(), query.plus_words.end(),
		result_words.begin(),
		word_checker
	);

	sort(result_words.begin(), words_end);
	words_end = unique(result_words.begin(), words_end);
	result_words.erase(words_end, result_words.end());

	return { result_words, documents_.at(document_id).status };
}


bool SearchServer::IsStopWord(string_view word) const {
	return stop_words_.count(word) > 0;
}


vector<string_view> SearchServer::SplitIntoWords(string_view text) const {
	vector<string_view> words;
	if (text.empty()) return words;

	while (true) {
		const auto space = text.find(' ');
		words.push_back(text.substr(0, space));
		if (space == text.npos) {
			break;
		}
		else {
			text.remove_prefix(space + 1);
		}
	}
	return words;

}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
	vector<string_view> words;
	for (const string_view word : SplitIntoWords(text)) {
		if (!IsValidWord(word)) {
			throw invalid_argument("words has bad symbols");
		}
		if (!IsStopWord(word)) {
			words.push_back(word);
		}
	}
	return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
	if (text.empty()) throw invalid_argument("query is empty");
	bool is_minus = false;

	if (text[0] == '-') {
		is_minus = true;
		text = text.substr(1);
	}
	if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
		throw invalid_argument("query isn't correct");
	}
	return { text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(string_view text, bool skip_sort) const {
	Query query;
	for (const auto word : SplitIntoWords(text)) {
		const QueryWord query_word = ParseQueryWord(word);
		if (!query_word.is_stop) {
			if (query_word.is_minus) {
				query.minus_words.push_back(query_word.data);
			}
			else {
				query.plus_words.push_back(query_word.data);
			}
		}
	}
	if (!skip_sort) {
		for (auto* words : { &query.plus_words, &query.minus_words }) {
			sort(words->begin(), words->end());
			words->erase(unique(words->begin(), words->end()), words->end());
		}
	}
	return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
	return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

SearchServer CreateSearchServer() {
	SearchServer search_server(ReadLine());
	return search_server;
}

std::vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
	return FindTopDocuments(execution::seq, raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
	return FindTopDocuments(execution::seq, raw_query);
}
