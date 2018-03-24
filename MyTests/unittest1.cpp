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
		try {
			constexpr const auto file_name {L"./test_resources/graph_io_test.txt"};
			pathfinding::Grid my_grid {0, 1, 3, 3, {
				{100, 100, 100, 100, 100},
				{100, 20, 10, 20, 100},
				{100, 15, 12, 15, 100},
				{100, 25, 15, 20, 100},
				{100, 100, 100, 100, 100}
			}};
			std::wofstream output_file {file_name};
			if (!output_file.good()) {
				Assert::Fail(L"Could not open output file to perform test.");
			}
			try {
				output_file << my_grid;
			}
			catch (std::runtime_error) {
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
			catch (std::runtime_error) {
				Assert::Fail(L"The input file generated an invalid graph.");
			}
		}
		catch (...) {
			Assert::Fail(L"An unexpected exception occurred.");
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
		try {
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
		catch (...) {
			Assert::Fail(L"An unexpected exception occurred.");
		}
	}

	// Get neighbors test for graphs
	TEST_METHOD(Pathfinder_Graph_Neighbors) {
		try {
			pathfinding::Grid my_graph {0, 1, 8, 7, {
				// 10x8 graph
				{100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
				{100,   1,   1,   1,   1,   1,   1,   1,   1, 100},
				{100,   1,   2,   1,   1,   1,   2,   1,   1, 100},
				{100, 100,   2,   2,   2,   1,   2,   2, 100, 100},
				{100,   2,   2,   1,   2,   2, 100,   1,   1, 100},
				{100,   1,   1,   1, 100, 100, 100,   1, 100, 100},
				{100,   1,   1,   1, 100, 100, 100,   1,   1, 100},
				{100, 100, 100, 100, 100, 100, 100, 100, 100, 100}
			}};
			pathfinding::Grid filter_graph {-1, -1, -1, -1, {
				// 10x8 graph
				{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0, 100,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0,   0, 100,   0,   0,   0},
				{  0,   0,   0,   0,   0, 100,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
				{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
			}};

			auto neighbor_set_a = my_graph.getNeighbors(3, 3, filter_graph, false);
			Assert::AreEqual(4U, neighbor_set_a.size());
			auto neighbor_set_b = my_graph.getNeighbors(3, 3, filter_graph, true);
			Assert::AreEqual(8U, neighbor_set_b.size());
			try {
				auto neighbor_set_c = my_graph.getNeighbors(0, 3, filter_graph, true);
				Assert::AreEqual(2U, neighbor_set_c.size());
				auto neighbor_set_d = my_graph.getNeighbors(9, 1, filter_graph, false);
				Assert::AreEqual(1U, neighbor_set_d.size());
			}
			catch (...) {
				Assert::Fail(L"Exception caught: invalid bounds (x-dimension)!");
			}
			try {
				auto neighbor_set_e = my_graph.getNeighbors(5, 0, filter_graph, true);
				Assert::AreEqual(3U, neighbor_set_e.size());
				auto neighbor_set_f = my_graph.getNeighbors(4, 7, filter_graph, true);
				Assert::AreEqual(1U, neighbor_set_f.size());
			}
			catch (...) {
				Assert::Fail(L"Exception caught: invalid bounds (y-dimension)!");
			}
			auto neighbor_set_g = my_graph.getNeighbors(5, 3, filter_graph, true);
			Assert::AreEqual(4U, neighbor_set_g.size());
		}
		catch (...) {
			Assert::Fail(L"An unexpected exception occurred.");
		}
	}
		};
	}
}