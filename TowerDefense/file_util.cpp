// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <string>
#include <limits>
#include <utility>
#include "./file_util.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace utility::file {
		std::pair<TokenTypes, std::wstring> getNextToken(std::wistream& is, int& line) {
			std::wstring next {};
			is >> next;
			if (is.eof()) {
				return std::make_pair(TokenTypes::End_Of_File, L""s);
			}
			else if (next[next.size() - 1] == L'\n') {
				next.erase(next.end() - 1);
				++line;
			}
			if (next[0] == '"') {
				return std::make_pair(TokenTypes::String, getQuotedToken(is, next, line));
			}
			else if (next[0] == '#' || next[0] == ';') {
				if (next[next.size() - 1] != '\n') {
					skipComments(is);
				}
				// Recursive call after skipping comments
				return getNextToken(is, line);
			}
			if (next[next.size() - 1] == L',') {
				// Commas are generally optional so they are stripped
				next.erase(next.end() - 1);
			}
			if (next[0] == L'[') {
				// Note that new lines should have been handled by this point.
				if (next[next.size() - 1] != L']') {
					throw DataFileException {L"Invalid section header."s, line};
				}
				next.erase(next.begin());
				next.erase(next.end() - 1);
				return std::make_pair(TokenTypes::Section, next);
			}
			else if (next[0] == L'<' || next[next.size() - 1] == L'>') {
				if (next[0] == L'<') {
					next.erase(next.begin());
				}
				if (next.at(next.size() - 1) == L'>') {
					next.erase(next.end() - 1);
				}
				// The empty list is considered invalid, but I don't
				// check this.
				return std::make_pair(TokenTypes::List,
					next.at(0) == L'"' ? getQuotedToken(is, next, line) : next);
			}
			else if (next[0] == L'{' || next[next.size() - 1] == L'}') {
				if (next[0] == L'{') {
					next.erase(next.begin());
				}
				else if (next[next.size() - 1] == L'}') {
					next.erase(next.end() - 1);
				}
				if (!next.empty()) {
					// This makes parsing and handling tokens easier
					throw DataFileException {L"Braces must stand alone; you cannot have extra characters"
						L" immediately before or after them."s, line};
				}
				return std::make_pair(TokenTypes::Object, L""s);
			}
			else if (next[0] == '-' || (next[0] >= '0' && next[0] <= '9')) {
				bool has_decimal = false;
				for (int i = 0; i < next.size(); ++i) {
					if (i == 0) {
						continue;
					}
					if (next[i] == L'.' && has_decimal) {
						throw DataFileException {L"A number may not possess two decimal points."s, line};
					}
					else if (next[i] == L'.') {
						has_decimal = true;
					}
					else if (next[i] == L',' || next[i] == L'_') {
						next.erase(next.begin() + i);
						--i;
					}
					else if (next[i] < L'0' || next[i] > L'9') {
						throw DataFileException {L"An invalid character was encountered in a numeric literal."s, line};
					}
				}
				return std::make_pair(TokenTypes::Number, next);
			}
			return std::make_pair(TokenTypes::Identifier, next);
		}

		void skipComments(std::wistream& is) {
			std::wstring buffer {};
			do {
				is >> buffer;
			} while (!is.eof() && buffer[buffer.size() - 1] != '\n');
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
						next.erase(next.begin() + i + 1, next.end());
						break;
					}
					else if (lookahead_char == L'"') {
						next.erase(next.begin() + i);
					}
					else if (lookahead_char == L'\n') {
						// Illegal character found!
						throw DataFileException {L"Newlines cannot appear in quoted strings."s, line};
					}
				}
				if (is_first) {
					next.erase(next.begin());
					string_token = next;
					is_first = false;
				}
				else {
					string_token += L" "s + next;
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