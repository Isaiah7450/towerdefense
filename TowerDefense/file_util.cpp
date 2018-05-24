// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <string>
#include <limits>
#include "./file_util.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace utility::file {
		std::wstring getNextToken(std::wistream& is, int& line) {
			std::wstring next {};
			is >> next;
			if (next[next.size() - 1] == L'\n') {
				// Newline character found!
				++line;
			}
			if (next[0] == '"') {
				// String Token
				// Note that surrounding quotes weren't trimmed
				return getQuotedToken(is, next, line);
			}
			if (next[next.size() - 1] == L'\n') {
				next.erase(next.end() - 1);
				++line;
			}
			else if (next[next.size() - 1] == L',') {
				// Commas are generally optional so they are stripped
				next.erase(next.end() - 1);
			}
			// Callers should check if the first character is a <, [, or {
			// to determine the token's type.
			return next;
		}

		std::wstring getQuotedToken(std::wistream& is, std::wstring buffer, const int& line) {
			bool found_end = false;
			bool is_first = true;
			std::wstring next {buffer};
			std::wstring string_token {};
			while (!found_end) {
				wchar_t lookahead_char = 0;
				wchar_t current_char = 0;
				for (unsigned int i = 0; i < next.size() - 1; ++i) {
					lookahead_char = next[i + 1];
					current_char = next[i];
					if (lookahead_char == L'"' && current_char != L'\\') {
						found_end = true;
						// Yeah, not the best solution, but the easiest
						// plus I don't really need to super support
						// escape sequences anyway.
						if (i + 2 != next.size()) {
							next.erase(next.begin() + i + 2, next.end());
						}
						break;
					}
					else if (lookahead_char == L'\n') {
						// Illegal character found!
						throw DataFileException {L"Newlines cannot appear in quoted strings."s, line};
					}
				}
				if (!is_first) {
					string_token += L" "s + next;
				}
				else {
					is_first = false;
					string_token = next;
				}
				if (is.eof() && !found_end) {
					throw DataFileException {L"Unexpected EOF encountered while parsing a quoted string."s, line};
				}
				else if (!found_end) {
					is >> next;
				}
			} // End while loop
			return string_token;
		}
	}
}