// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <string>
#include <limits>
#include <utility>
#include <vector>
#include "./file_util.hpp"
#include "./graphics/graphics.hpp"
#include "./graphics/shapes.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace util::file {
		std::pair<TokenTypes, std::wstring> getNextToken(std::wistream& is, int& line) {
			std::wstring next {};
			is >> next;
			stripLeadingWhitespace(next, line);
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
				return std::make_pair(TokenTypes::List,
					next.at(0) == L'"' ? getQuotedToken(is, next, line) : next);
			}
			else if (next[0] == L'{' || next[next.size() - 1] == L'}') {
				if (next.size() == 2 && next[0] == L'{' && next[1] == L'}') {
					// The empty object
					return std::make_pair(TokenTypes::Object, L"{}"s);
				}
				else if (next.size() != 1) {
					// This makes parsing and handling tokens easier
					throw DataFileException {L"Braces must stand alone; you cannot have extra characters"
						L" immediately before or after them."s, line};
				}
				return std::make_pair(TokenTypes::Object, next);
			}
			else if (next[0] == '-' || (next[0] >= '0' && next[0] <= '9')) {
				bool has_decimal = false;
				for (unsigned int i = 0; i < next.size(); ++i) {
					if (i == 0U) {
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

		void stripLeadingWhitespace(std::wstring& buffer, int& line) {
			for (unsigned int i = 0; i < buffer.size(); ++i) {
				if (buffer[i] == L'\r' || buffer[i] == L'\n'
					|| buffer[i] == L' ' || buffer[i] == L'\t') {
					if (buffer[i] == L'\n') {
						++line;
					}
					buffer.erase(buffer.begin() + i);
					--i;
				}
				else {
					break;
				}
			}
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

		bool matchToken(TokenTypes expected_type, std::wstring expected_input,
			std::pair<TokenTypes, std::wstring> actual_token) noexcept {
			return matchTokenType(expected_type, actual_token.first)
				&& matchTokenValue(expected_input, actual_token.second);
		}

		bool matchTokenType(TokenTypes expected_type, TokenTypes actual_type) noexcept {
			return expected_type == actual_type;
		}

		bool matchTokenValue(std::wstring expected_value, std::wstring actual_value) noexcept {
			return expected_value == actual_value;
		}

		std::pair<TokenTypes, std::wstring> getKeyValue(std::wstring expected_key, std::wistream& is, int& line) {
			auto my_token = getNextToken(is, line);
			if (!matchToken(TokenTypes::Identifier, expected_key, my_token)) {
				if (!matchTokenType(TokenTypes::Identifier, my_token.first)) {
					throw DataFileException {L"Expected a key name."s, line};
				}
				else {
					throw DataFileException {L"An invalid key name was specified; expected: "s + expected_key, line};
				}
			}
			my_token = getNextToken(is, line);
			if (!matchToken(TokenTypes::Identifier, L"="s, my_token)) {
				throw DataFileException {L"Expected an equal sign after the key's name."s, line};
			}
			return getNextToken(is, line);
		}

		bool verifyObjectStart(std::wstring expected_object_key, std::wistream& is, int& line) {
			auto my_token = getNextToken(is, line);
			if (!matchToken(TokenTypes::Identifier, expected_object_key, my_token)) {
				if (!matchTokenType(TokenTypes::Identifier, my_token.first)) {
					throw DataFileException {L"Expected a key name."s, line};
				}
				else {
					throw DataFileException {L"An invalid key name was specified; expected: "s + expected_object_key, line};
				}
			}
			my_token = getNextToken(is, line);
			if (!matchToken(TokenTypes::Identifier, L"="s, my_token)) {
				throw DataFileException {L"Expected an equal sign after the key's name."s, line};
			}
			my_token = getNextToken(is, line);
			if (!matchToken(TokenTypes::Object, L"{"s, my_token)) {
				throw DataFileException {L"Expected an opening brace following the equal sign."s, line};
			}
			return !matchTokenValue(L"{}"s, my_token.second);
		}

		std::wstring parseString(std::pair<TokenTypes, std::wstring> token, int line) {
			if (!matchTokenType(TokenTypes::String, token.first)) {
				throw DataFileException {L"Expected a quoted string."s, line};
			}
			return token.second;
		}

		double parseNumber(std::pair<TokenTypes, std::wstring> token, int line) {
			if (!matchTokenType(TokenTypes::Number, token.first)) {
				throw DataFileException {L"Expected a quoted string."s, line};
			}
			return std::stod(token.second);
		}

		bool parseBoolean(std::pair<TokenTypes, std::wstring> token, int line) {
			return (token.second == L"True"s || token.second == L"true"s) ? true
				: (token.second == L"False"s || token.second == L"false"s) ? false
				: throw DataFileException {L"Invalid boolean constant specified."s
					L" Expected one of: {True, False}."s, line};
		}

		std::vector<std::wstring> parseList(std::pair<TokenTypes, std::wstring> token, std::wistream& is, int& line) {
			std::vector<std::wstring> list_items {};
			do {
				list_items.emplace_back(token.second);
				token = util::file::getNextToken(is, line);
			} while (token.first != util::file::TokenTypes::List);
			return list_items;
		}

		graphics::Color readColor(std::wistream& is, int& line) {
			auto my_token = getKeyValue(L"color"s, is, line);
			auto my_list = parseList(my_token, is, line);
			if (my_list.size() != 4) {
				throw DataFileException {L"The color property takes a list of 4 values: Red, Green, Blue, and Alpha."s, line};
			}
			return graphics::Color {std::stof(my_list[0]), std::stof(my_list[1]), std::stof(my_list[2]),
				std::stof(my_list[3])};
		}

		graphics::shapes::ShapeTypes readShape(std::wistream& is, int& line) {
			auto my_token = getKeyValue(L"shape"s, is, line);
			if (!matchTokenType(TokenTypes::Identifier, my_token.first)) {
				throw DataFileException {L"Expected a shape constant."s, line};
			}
			const auto& my_value = my_token.second;
			if (my_value == L"Ellipse"s) {
				return graphics::shapes::ShapeTypes::Ellipse;
			}
			else if (my_value == L"Triangle"s) {
				return graphics::shapes::ShapeTypes::Triangle;
			}
			else if (my_value == L"Rectangle"s) {
				return graphics::shapes::ShapeTypes::Rectangle;
			}
			else if (my_value == L"Diamond"s) {
				return graphics::shapes::ShapeTypes::Diamond;
			}
			else if (my_value == L"Star"s) {
				return graphics::shapes::ShapeTypes::Star;
			}
			else {
				throw DataFileException {L"Invalid shape constant specified."s
					L" Expected one of: Ellipse, Triangle, Rectangle, Diamond, Star."s, line};
			}
		}
	}
}