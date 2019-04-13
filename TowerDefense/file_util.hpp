#pragma once
// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <string>
#include <iosfwd>
#include <type_traits>
#include <utility>
#include <vector>
#include "./graphics/graphics.hpp"
#include "./graphics/shapes.hpp"

namespace hoffman::isaiah {
	namespace util::file {
		/// <summary>Exception class for data file parsing errors.</summary>
		class DataFileException {
		public:
			DataFileException(std::wstring msg, int line) :
				message {L"On line " + std::to_wstring(line) + L": " + msg} {
			}
			/// <returns>The error message associated with the exception.</returns>
			const wchar_t* what() const noexcept {
				return this->message.c_str();
			}
		private:
			/// <summary>The message to output.</summary>
			std::wstring message;
		};

		/// <summary>The various types of tokens that can be returned by the parser.</summary>
		enum class TokenTypes {
			// Identifier --> Raw text, Section --> [bracketed_text]
			// String --> "Quoted text", Number --> [0-9]{1,}
			// List --> <Bracketed List>, Object --> {Object}
			Identifier, Section, String, Number, List, Object
		};

		/// <summary>Lexical and syntactical analyzer that reads data files.</summary>
		class DataFileParser {
		public:
			DataFileParser(std::wistream& is);
			/// <summary>Reads the next token and stores the value in the lexer.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool getNext();

			// Utility functions
			/// <summary>Determines if the current token matches the supplied values.</summary>
			/// <param name="expected_type">The token type that is expected.</param>
			/// <param name="expected_value">The token value that is expected.</param> 
			/// <returns>True if the current token and type match the supplied expected values.</returns>
			bool matchToken(TokenTypes expected_type, std::wstring expected_value) const noexcept {
				return this->matchTokenType(expected_type) && this->matchTokenValue(expected_value);
			}
			/// <summary>Checks if the current token matches the supplied values, and throws an
			/// exception if they do not.</summary>
			/// <param name="expected_type">The token type that is expected.</param>
			/// <param name="expected_value">The token value that is expected.</param> 
			void expectToken(TokenTypes expected_type, std::wstring expected_value) const {
				using namespace std::literals::string_literals;
				if (!this->matchToken(expected_type, expected_value)) {
					switch (expected_type) {
					case TokenTypes::Identifier:
						throw DataFileException {L"Expected the following key/operator/identifier: "s + expected_value
							+ L"."s, this->getLine()};
					case TokenTypes::Number:
						throw DataFileException {L"Expected the following number: "s + expected_value
							+ L"."s, this->getLine()};
					case TokenTypes::String:
						throw DataFileException {L"Expected the following string: \""s + expected_value
							+ L"\"."s, this->getLine()};
					case TokenTypes::Section:
						throw DataFileException {L"Expected the following section: ["s + expected_value
							+ L"]."s, this->getLine()};
					case TokenTypes::List:
					case TokenTypes::Object:
						throw DataFileException {L"Expected the following symbol: "s + expected_value
							+ L"."s, this->getLine()};
					}
				}
			}
			/// <summary>Determines if the current token matches the supplied values.</summary>
			/// <param name="expected_value">The token value that is expected.</param> 
			/// <returns>True if the current token matches the supplied value.</returns>
			bool matchTokenValue(std::wstring expected_value) const noexcept {
				return this->getToken() == expected_value;
			}
			/// <summary>Determines if the current token matches the supplied values.</summary>
			/// <param name="expected_type">The token type that is expected.</param>
			/// <returns>True if the current token type matches the supplied value.</returns>
			bool matchTokenType(TokenTypes expected_type) const noexcept {
				return this->getTokenType() == expected_type;
			}
			/// <summary>Attempts to read a key-value pair (throwing an exception if
			/// this fails), and returns the value part of the key-value pair if successful.</summary>
			/// <param name="expected_key">The expected value for the key-part of the key-value pair.</param>
			/// <returns>The value part of the key-value pair.</returns>
			std::pair<TokenTypes, std::wstring> readKeyValue(std::wstring expected_key) {
				using namespace std::literals::string_literals;
				if (!this->getNext()) {
					throw DataFileException {L"Data file stream is an invalid state. Maybe the file"s
						L" ended early?"s, this->getLine()};
				}
				this->expectToken(TokenTypes::Identifier, expected_key);
				if (!this->getNext()) {
					throw DataFileException {L"Data file stream is an invalid state. Maybe the file"s
						L" ended early?"s, this->getLine()};
				}
				this->expectToken(TokenTypes::Identifier, L"="s);
				if (!this->getNext()) {
					throw DataFileException {L"Data file stream is an invalid state. Maybe the file"s
						L" ended early?"s, this->getLine()};
				}
				return std::make_pair<TokenTypes, std::wstring>(this->getTokenType(), this->getToken());
			}
			/// <summary>Attempts to parse the current input as a string. (Throws an exception
			/// if not a string.)</summary>
			/// <returns>The parsed string.</returns>
			std::wstring parseString() const {
				if (!this->matchTokenType(TokenTypes::String)) {
					throw DataFileException {L"The current token is not a string.", this->getLine()};
				}
				return this->getToken();
			}
			/// <summary>Attempts to parse the current input as a number. (Throws an exception
			/// if not a number.)</summary>
			/// <returns>The parsed number.</returns>
			template <typename T = double>
			T parseNumber() const {
				if (!this->matchTokenType(TokenTypes::Number)) {
					throw DataFileException {L"The current token is not a number.", this->getLine()};
				}
				if constexpr(std::is_same_v<T, double>) {
					return std::stod(this->getToken());
				}
				else if constexpr(std::is_same_v<T, int>) {
					return std::stoi(this->getToken());
				}
				else if constexpr (std::is_same_v<T, float>) {
					return std::stof(this->getToken());
				}
				else if constexpr (std::is_same_v<T, long long>) {
					return std::stoll(this->getToken());
				}
				else {
					static_assert(false, "An invalid type was specified for parseNumber().");
				}
			}
			/// <summary>Attempts to parse the current input as a boolean constant. (Throws an exception
			/// if not a boolean constant.)</summary>
			/// <returns>The parsed boolean constant.</returns>
			bool parseBoolean() const {
				if (!this->matchTokenType(TokenTypes::Identifier)) {
					throw DataFileException {L"Expected an identifier.", this->getLine()};
				}
				else if (this->getToken() != L"True" && this->getToken() != L"False"
					&& this->getToken() != L"true" && this->getToken() != L"false") {
					throw DataFileException {L"Expected a boolean literal (`True` or `False`).", this->getLine()};
				}
				return this->getToken() == L"true" || this->getToken() == L"True";
			}
			/// <summary>Attempts to read and parse a list from the input stream.</summary>
			/// <returns>The items in the list.</returns>
			template <typename T = std::wstring>
			std::vector<T> readList() {
				// Note that this call should have been preceded by this->readKeyValue() or this->getNext()
				this->expectToken(TokenTypes::List, L"<");
				const int start_line = this->getLine();
				std::vector<T> list_items {};
				while (true) {
					if (!this->getNext()) {
						throw DataFileException {L"Data file stream is in an invalid state."
							L" Maybe the file ended early?", start_line};
					}
					if (this->matchToken(TokenTypes::List, L">")) {
						return list_items;
					}
					if constexpr (std::is_same_v<T, std::wstring>) {
						if (this->matchTokenType(TokenTypes::String)) {
							list_items.emplace_back(this->parseString());
						}
						else if (this->matchTokenType(TokenTypes::Identifier)) {
							list_items.emplace_back(this->getToken());
						}
						else {
							throw DataFileException {L"Expected a string or an identifier within the list.", this->getLine()};
						}
					}
					else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>
						|| std::is_same_v<T, float> || std::is_same_v<T, long long>) {
						list_items.emplace_back(this->parseNumber<T>());
					}
					else {
						static_assert(false, "An invalid type was specified for readList.");
					}
				}
			}
			/// <summary>Attempts to read and parse a color value from the input, returning
			/// the read color on success and throwing an exception on failure.</summary>
			/// <returns>The parsed color.</returns>
			graphics::Color readColor() {
#pragma warning(push)
#pragma warning(disable: 26444) // ES84: Avoid unnamed objects with custom construction/destruction. (No Idea...)
				this->readKeyValue(L"color");
#pragma warning(pop)
				auto my_list = this->readList<float>();
				if (my_list.size() != 4) {
					throw DataFileException {L"The color property takes a list of four numbers that"
						L" indicate the levels of Red, Green, Blue, and Alpha respectively.", this->getLine()};
				}
				return graphics::Color {my_list[0], my_list[1], my_list[2], my_list[3]};
			}
			/// <summary>Attempts to read and parse a shape constant from the input, returning
			/// the shape type on success and throwing an exception on failure.</summary>
			/// <returns>The parsed shape.</returns>
			graphics::shapes::ShapeTypes readShape() {
				using namespace std::literals::string_literals;
#pragma warning(push)
#pragma warning(disable: 26444) // ES84: Avoid unnamed objects with custom construction/destruction. (No Idea...)
				this->readKeyValue(L"shape");
#pragma warning(pop)
				if (!this->matchTokenType(TokenTypes::Identifier)) {
					throw DataFileException {L"Expected a shape constant."s, this->getLine()};
				}
				constexpr const wchar_t* shape_names[5] = {
					L"Ellipse", L"Triangle", L"Rectangle", L"Diamond", L"Star"
				};
				const std::wstring all_shapes = {
					shape_names[0] + L", "s + shape_names[1] + L", "s + shape_names[2] + L", "s
						+ shape_names[3] + L", "s + shape_names[4]
				};
				return this->matchTokenValue(shape_names[0]) ? graphics::shapes::ShapeTypes::Ellipse :
					this->matchTokenValue(shape_names[1]) ? graphics::shapes::ShapeTypes::Triangle :
					this->matchTokenValue(shape_names[2]) ? graphics::shapes::ShapeTypes::Rectangle :
					this->matchTokenValue(shape_names[3]) ? graphics::shapes::ShapeTypes::Diamond :
					this->matchTokenValue(shape_names[4]) ? graphics::shapes::ShapeTypes::Star :
					throw DataFileException {L"Invalid shape constant given: "s + this->getToken()
						+ L" Expected one of: "s + all_shapes + L"!"s, this->getLine()};
			}

			// Validates a number.
			// value -> Value to check.
			// min_value -> Minimum valid value.
			// max_value -> Maximum valid value.
			// prompt -> What is being validated.
			// line -> Line number.
			// inclusive_min -> Is the minimum bound valid?
			// inclusive_max -> Is the maximum bound valid?
			template <typename T>
			static void validateNumber(T value, T min_value, T max_value, std::wstring prompt, int line,
				bool inclusive_min, bool inclusive_max) {
				if (inclusive_min && inclusive_max) {
					if (value < min_value || value > max_value) {
						throw DataFileException {prompt + L" should be between " + std::to_wstring(min_value)
							+ L" and " + std::to_wstring(max_value) + L" (inclusive).", line};
					}
				}
				else if (inclusive_min) {
					if (value < min_value || value >= max_value) {
						throw DataFileException {prompt + L" should be between " + std::to_wstring(min_value)
							+ L" and " + std::to_wstring(max_value) + L" (inclusive-exclusive).", line};
					}
				}
				else if (inclusive_max) {
					if (value <= min_value || value > max_value) {
						throw DataFileException {prompt + L" should be between " + std::to_wstring(min_value)
							+ L" and " + std::to_wstring(max_value) + L" (exclusive-inclusive).", line};
					}
				}
				else if (value <= min_value || value >= max_value) {
					throw DataFileException {prompt + L" should be between " + std::to_wstring(min_value)
						+ L" and " + std::to_wstring(max_value) + L" (exclusive).", line};
				}
			}

			// Validates a number that only has a minimum bound.
			// value -> Value to check.
			// min_bound -> Minimum valid value.
			// prompt -> What is being validated.
			// line -> Line number.
			// inclusive_min -> Is the minimum bound valid?
			template <typename T>
			static void validateNumberMinBound(T value, T min_bound, std::wstring prompt, int line, bool inclusive_min) {
				if (inclusive_min) {
					if (value < min_bound) {
						throw DataFileException {prompt + L" must be at least " + std::to_wstring(min_bound) + L".", line};
					}
				}
				else if (value <= min_bound) {
					throw DataFileException {prompt + L" must exceed " + std::to_wstring(min_bound) + L".", line};
				}
			}

			// Validates a number that only has a minimum bound.
			// value -> Value to check.
			// max_bound -> Maximum valid value.
			// prompt -> What is being validated.
			// line -> Line number.
			// inclusive_max -> Is the maximum bound valid?
			template <typename T>
			static void validateNumberMaxBound(T value, T max_bound, std::wstring prompt, int line, bool inclusive_max) {
				if (inclusive_max) {
					if (value > max_bound) {
						throw DataFileException {prompt + L" cannot exceed " + std::to_wstring(max_bound) + L".", line};
					}
				}
				else if (value >= max_bound) {
					throw DataFileException {prompt + L" must be under " + std::to_wstring(max_bound) + L".", line};
				}
			}

			// Getters
			std::wstring getToken() const noexcept {
				return this->token;
			}
			TokenTypes getTokenType() const noexcept {
				return this->token_type;
			}
			int getLine() const noexcept {
				return this->line_number;
			}
		protected:
			/// <summary>Skips past leading whitespace and comments in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool skipOptional() noexcept;
			/// <summary>Skips past comments in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool skipComments() noexcept;
			/// <summary>Parses a string in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool readString();
			/// <summary>Parses a section header in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool readSection();
			/// <summary>Parses a number in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool readNumber();
			/// <summary>Parses an identifier in the input stream.</summary>
			/// <returns>False if a stream error occurs; otherwise, true.</returns>
			bool readIdentifier() noexcept;
			/// <summary>Checks if the data stream is in a valid state.</summary>
			/// <returns>True if the input stream is in a valid state; otherwise, false.</returns>
			bool isValid() const noexcept;
		private:
			/// <summary>The input stream being read.</summary>
			std::wistream& data_file;
			/// <summary>The current lookahead token.</summary>
			wchar_t lookahead {L'\0'};
			/// <summary>The current token being read by the lexer.</summary>
			std::wstring token {L""};
			/// <summary>The type of token that is being read.</summary>
			TokenTypes token_type {TokenTypes::Identifier};
			/// <summary>The current line number the lexer is on.</summary>
			int line_number {1};
		};

		// Deprecated code that will eventually be removed.
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_type">The expected type of token.</param>
		/// <param name="expected_input">The expected content of the token.</param>
		/// <param name="actual_token">The actual token obtained from the input stream.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		[[deprecated]] bool matchToken(TokenTypes expected_type, std::wstring expected_input,
			std::pair<TokenTypes, std::wstring> actual_token) noexcept;
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_type">The expected type of the token.</param>
		/// <param name="actual_type">The actual type of the token.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		[[deprecated]] bool matchTokenType(TokenTypes expected_type, TokenTypes actual_type) noexcept;
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_value">The expected value for the token.</param>
		/// <param name="actual_value">The actual value associated with the token.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		[[deprecated]] bool matchTokenValue(std::wstring expected_value, std::wstring actual_value) noexcept;
		/// <summary>Attempts to parse the given token as a string. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a string.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed string.</returns>
		[[deprecated]] std::wstring parseString(std::pair<TokenTypes, std::wstring> token, int line);
		/// <summary>Attempts to parse the given token as a number. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a number.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed number.</returns>
		[[deprecated]] double parseNumber(std::pair<TokenTypes, std::wstring> token, int line);
		/// <summary>Attempts to parse the given token as a boolean. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a boolean.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed boolean.</returns>
		[[deprecated]] bool parseBoolean(std::pair<TokenTypes, std::wstring> token, int line);
	}
}