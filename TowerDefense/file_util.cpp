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

namespace hoffman_isaiah {
	namespace util::file {
		DataFileParser::DataFileParser(std::wistream& is) :
			data_file {is} {
			this->lookahead = this->data_file.get();
			if (!this->isValid()) {
				throw DataFileException {L"Could not read data file."s, 0};
			}
			this->getNext();
		}

		bool DataFileParser::getNext() {
			if (!this->skipOptional()) {
				return false;
			}
			// Special case: optional commas
			if (this->lookahead == L',') {
				this->lookahead = this->data_file.get();
				if (!this->isValid() || !this->skipOptional()) {
					return false;
				}
			}
			if (this->lookahead == L'<' || this->lookahead == L'>') {
				this->token = this->lookahead;
				this->token_type = TokenTypes::List;
			}
			else if (this->lookahead == L'{' || this->lookahead == L'}') {
				this->token = this->lookahead;
				this->token_type = TokenTypes::Object;
			}
			else if (this->lookahead == L'"') {
				this->token_type = TokenTypes::String;
				return this->readString();
			}
			else if (this->lookahead == L'[') {
				this->token_type = TokenTypes::Section;
				return this->readSection();
			}
			else if ((this->lookahead >= L'0' && this->lookahead <= L'9')
				|| (this->lookahead == L'+' || this->lookahead == L'-')) {
				this->token_type = TokenTypes::Number;
				return this->readNumber();
			}
			else {
				this->token_type = TokenTypes::Identifier;
				return this->readIdentifier();
			}
			this->lookahead = this->data_file.get();
			return this->isValid();
		}

		bool DataFileParser::skipOptional() noexcept {
			while (this->lookahead == L' ' || this->lookahead == L'\t'
				|| this->lookahead == L'\r' || this->lookahead == L'\n'
				|| this->lookahead == L'#' || this->lookahead == L';') {
				while (this->lookahead == L'#' || this->lookahead == L';') {
					// Skip comments
					while (this->lookahead != L'\n') {
						this->lookahead = this->data_file.get();
						if (!this->isValid()) {
							return false;
						}
					}
				}
				while (this->lookahead == L' ' || this->lookahead == L'\t'
					|| this->lookahead == L'\r' || this->lookahead == L'\n') {
					// Skip white-space
					if (this->lookahead == L'\n') {
						++this->line_number;
					}
					this->lookahead = this->data_file.get();
					if (!this->isValid()) {
						return false;
					}
				}
			}
			return true;
		}

		bool DataFileParser::readString() {
			this->token = L"";
			wchar_t prev = this->lookahead;
			while (true) {
				prev = this->lookahead;
				this->lookahead = this->data_file.get();
				if (!this->isValid()) {
					return false;
				}
				if (prev == L'\\') {
					if (this->lookahead == L'\\' || this->lookahead == L'"'
						|| this->lookahead == L'n') {
						this->token.erase(this->token.end() - 1);
					}
					if (this->lookahead == L'n') {
						this->token += L'\n';
						continue;
					}
				}
				else if (this->lookahead == L'"') {
					this->lookahead = this->data_file.get();
					return this->isValid();
				}
				else if (this->lookahead == L'\n' || this->lookahead == L'\t') {
					throw DataFileException {L"Newlines and tabs are unallowed inside strings.", this->getLine()};
				}
				this->token += this->lookahead;
			}
		}

		bool DataFileParser::readSection() {
			this->token = L"";
			while ((this->lookahead = data_file.get()) != L']') {
				if (!this->isValid()) {
					return false;
				}
				if (this->lookahead != L'_' && !((this->lookahead >= L'a' && this->lookahead <= L'z')
					|| (this->lookahead >= L'A' && this->lookahead <= L'Z')
					|| (this->lookahead >= L'0' && this->lookahead <= L'9'))) {
					throw DataFileException {L"Invalid character encountered: "s + this->lookahead
						+ L" in section header!"s, this->getLine()};
				}
				this->token += this->lookahead;
			}
			this->lookahead = this->data_file.get();
			return this->isValid();
		}

		bool DataFileParser::readNumber() {
			this->token = this->lookahead;
			bool has_decimal_point = false;
			while (true) {
				this->lookahead = this->data_file.get();
				if (!this->isValid()) {
					return false;
				}
				if (this->lookahead == L'_' || this->lookahead == L',') {
					// Underscores and commas can be used as separators.
					continue;
				}
				else if (this->lookahead == L'.' && !has_decimal_point) {
					has_decimal_point = true;
				}
				else if (this->lookahead == L'.') {
					throw DataFileException {L"A decimal point (.) cannot appear twice in a number."s,
						this->getLine()};
				}
				else if (this->lookahead == L' ' || this->lookahead == L'\t'
					|| this->lookahead == L'\r' || this->lookahead == L'\n'
					|| this->lookahead == L'{' || this->lookahead == L'}'
					|| this->lookahead == L'<' || this->lookahead == L'>'
					|| this->lookahead == L'[' || this->lookahead == L']') {
					return true;
				}
				else if (this->lookahead < L'0' || this->lookahead > L'9') {
					throw DataFileException {L"Invalid character: "s + this->lookahead
						+ L" encountered in numeric literal!"s, this->getLine()};
				}
				this->token += this->lookahead;
			}
		}

		bool DataFileParser::readIdentifier() noexcept {
			this->token = this->lookahead;
			while (true) {
				this->lookahead = this->data_file.get();
				if (!this->isValid()) {
					return false;
				}
				if (!((this->lookahead >= L'a' && this->lookahead <= L'z')
					|| (this->lookahead >= L'A' && this->lookahead <= L'Z')
					|| (this->lookahead >= L'0' && this->lookahead <= L'9')
					|| this->lookahead == L'_' || this->lookahead == L'=')) {
					return true;
				}
				this->token += this->lookahead;
			}
		}

		bool DataFileParser::isValid() const noexcept {
			return !(this->data_file.bad() || this->data_file.fail());
		}
	}
}