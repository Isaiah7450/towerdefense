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
	TEST_METHOD(Pathfinder_Graph_IO_A) {
		constexpr const auto file_name {L"./test_resources/graph_io_test.txt"};
		pathfinding::Grid my_grid {0, 0, 1, 1};
		std::wofstream output_file {file_name};
		if (!output_file.good()) {
			Assert::Fail(L"Could not open output file to perform test.");
		}
		try {
			output_file << my_grid;
		}
		catch (...) {
			output_file.close();
			Assert::Fail(L"Graph lacks a start and ending node.");
		}
		output_file.close();
		std::wifstream input_file {file_name};
		if (!input_file.good()) {
			Assert::Fail(L"Could not open input file to perform test.");
		}
		try {
			input_file >> my_grid;
		}
		catch (...) {
			Assert::Fail(L"The input file generated an invalid graph.");
		}
	}

	// Second Input/Output Test for graphs
	// This loads a premade graph and tests that the file representation
	// coincides with the internal data representation.
	TEST_METHOD(Pathfinder_Graph_IO_B) {
		constexpr const auto file_name {L"./test_resources/test_graph_io_b.txt"};
		std::wifstream input_file {file_name};
		if (!input_file.good()) {
			Assert::Fail(L"Could not open required input file for the test.");
		}
		pathfinding::Grid my_graph {input_file};
		Assert::AreEqual(2, my_graph.getNode(2, 3).getGameX());
		Assert::AreEqual(1, my_graph.getNode(7, 1).getGameY());
		Assert::AreEqual(false, my_graph.getNode(1, 3).isBlocked());
		Assert::AreEqual(1, my_graph.getNode(2, 1).getWeight());
		Assert::AreEqual(my_graph.getNode(0, 2).getWeight(), my_graph.getStartNode()->getWeight());
		Assert::AreEqual(my_graph.getNode(7, 3).getWeight(), my_graph.getGoalNode()->getWeight());
		Assert::AreEqual(true, my_graph.getNode(3, 0).isBlocked());
		Assert::AreEqual(0, my_graph.getStartNode()->getGameX());
		Assert::AreEqual(3, my_graph.getGoalNode()->getGameY());
	}
		};
	}
}