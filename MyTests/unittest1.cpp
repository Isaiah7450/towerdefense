#include "stdafx.h"
#include "./globals.hpp"
#include "./pathfinding/grid.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace hoffman::isaiah {
	namespace tests {
		TEST_CLASS(Pathfinding) {
public:
	// First Input/Output test for graphs:
	// Checks whether the game can write a fresh graph and then subsequently read
	// it back without failing.
	// It does not check for the correctness of the read in graph.
	TEST_METHOD(Graph_IO_A) {
		pathfinding::Grid my_grid {0, 0, 1, 1};
		std::wofstream output_file {L"graph_io_test.txt"};
		if (output_file.fail() || output_file.bad()) {
			Assert::Fail(L"Could not open output file to perform test.");
		}
		try {
			output_file << my_grid;
		}
		catch (...) {
			Assert::Fail(L"Graph lacks a start and ending node.");
		}
		output_file.close();
		std::wifstream input_file {L"graph_io_test.txt"};
		try {
			input_file >> my_grid;
		}
		catch (...) {
			Assert::Fail(L"The input file generated an invalid graph.");
		}
		input_file.close();
		// Remove generated file
		std::remove("graph_io_test.txt");
	}
		};
	}
}