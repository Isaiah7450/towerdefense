#pragma once
// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <string>
#include <iosfwd>

namespace hoffman::isaiah {
	namespace utility::file {
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

		/// <summary>Obtains the next token in the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="line">The current line number. (This may be modified
		/// by the parser.)</param>
		/// <returns>The parsed token.</returns>
		std::wstring getNextToken(std::wistream& is, int& line);
		/// <summary>Obtains a token surrounded in quotes from the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="buffer">The buffer containing the initial input.</param>
		/// <param name="line">The current line number.</param>
		/// <returns>The token that was surrounded in quotes with the \" escape sequence
		/// expanded.</returns>
		std::wstring getQuotedToken(std::wistream& is, std::wstring buffer, const int& line);
		/// <summary>Skips past comments in the input stream.</summary>
		/// <param name="is">The input stream to parse.</param>
		void skipComments(std::wistream& is);
		/// <summary>Attempts to parse the next token as a section
		/// header in the configuration file, and checks if it matches
		/// the expected input.</summary>
		/// <param name="is">The input stream to parse.</param>
		/// <param name="expected_name">The section name that is expected to be found.</param>
		std::wstring matchSection(std::wistream& is, std::wstring expected_name);
	}
}