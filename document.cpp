#include "document.h"
#include <iostream>

using namespace std;

Document::Document() = default;

Document::Document(int id, double relevance, int rating)
	: id(id)
	, relevance(relevance)
	, rating(rating) {}

ostream& operator <<(ostream& os, const DocumentStatus& container) {
	os << static_cast<int>(container);
	return os;
}

ostream& operator<< (ostream& out, const Document& doc) {
	out << "{ "s;
	out << "document_id = "s << doc.id << ", "s;
	out << "relevance = "s << doc.relevance << ", "s;
	out << "rating = "s << doc.rating;
	out << " }"s;
	return out;
}
