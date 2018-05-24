#pragma once
// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <string>
#include <iosfwd>
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
			Identifier, Section, String, Number, List, Object, End_Of_File
		};

		/// <summary>Obtains the next token in the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">The current line number. (This may be modified
		/// by the parser.)</param>
		/// <returns>The parsed token, and its corresponding type.</returns>
		std::pair<TokenTypes, std::wstring> getNextToken(std::wistream& is, int& line);
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">The current line number. (This may be modified
		/// by the parser.)</param>
		void updateLineCount(std::wistream& is, int& line);
		/// <summary>Skips past comments in the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		void skipComments(std::wistream& is);
		/// <summary>Obtains a token surrounded in quotes from the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="buffer">The buffer containing the initial input.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The token that was surrounded in quotes with the \" escape sequence
		/// expanded.</returns>
		std::wstring getQuotedToken(std::wistream& is, std::wstring buffer, int& line);
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_type">The expected type of token.</param>
		/// <param name="expected_input">The expected content of the token.</param>
		/// <param name="actual_token">The actual token obtained from the input stream.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		bool matchToken(TokenTypes expected_type, std::wstring expected_input,
			std::pair<TokenTypes, std::wstring> actual_token) noexcept;
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_type">The expected type of the token.</param>
		/// <param name="actual_type">The actual type of the token.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		bool matchTokenType(TokenTypes expected_type, TokenTypes actual_type) noexcept;
		/// <summary>Determines if the provided token matches what is expected.</summary>
		/// <param name="expected_value">The expected value for the token.</param>
		/// <param name="actual_value">The actual value associated with the token.</param>
		/// <returns>True if it matches, otherwise, false.</returns>
		bool matchTokenValue(std::wstring expected_value, std::wstring actual_value) noexcept;
		/// <summary>Obtains the value of a key-value pair from the data file.
		/// An exception is thrown if the provided input is not a key-value pair
		/// or if the provided key does not match the expected key.</summary>
		/// <param name="expected_key">The expected value of the key part of the key-value pair.</param>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">A reference to the variable holding the current line number.</param>
		/// <returns>The value part of the key-value pair.</returns>
		std::pair<TokenTypes, std::wstring> getKeyValue(std::wstring expected_key, std::wistream& is, int& line);
		/// <summary>Verifies the starting line of an object definition.</summary>
		/// <param name="expected_object_key">The key name that immediately precedes the object's definition.</param>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">A reference to the variable holding the current line number.</param>
		/// <returns>True if this is a real object definition, false if the empty object was found.</returns>
		bool verifyObjectStart(std::wstring expected_object_key, std::wistream& is, int& line);
		/// <summary>Attempts to parse the given token as a string. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a string.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed string.</returns>
		std::wstring parseString(std::pair<TokenTypes, std::wstring> token, int line);
		/// <summary>Attempts to parse the given token as a number. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a number.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed number.</returns>
		double parseNumber(std::pair<TokenTypes, std::wstring> token, int line);
		/// <summary>Attempts to parse the given token as a boolean. If this fails, an exception will
		/// be thrown.</summary>
		/// <param name="token">The token to parse as a boolean.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The parsed boolean.</returns>
		bool parseBoolean(std::pair<TokenTypes, std::wstring> token, int line);
		/// <param name="token">The initial token in the list.</param>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">Reference to the variable holding the current line number.</param>
		/// <returns>The parsed list.</returns>
		std::vector<std::wstring> parseList(std::pair<TokenTypes, std::wstring> token, std::wistream& is, int& line);
		/// <summary>Attempts to read a color from the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">Reference to the variable holding the current line number.</param>
		/// <returns>The parsed color.</returns>
		graphics::Color readColor(std::wistream& is, int& line);
		/// <summary>Attempts to read a shape constant from the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">Reference to the variable holding the current line number.</param>
		/// <returns>The parsed shape constant.</returns>
		graphics::shapes::ShapeTypes readShape(std::wistream& is, int& line);
	}
}