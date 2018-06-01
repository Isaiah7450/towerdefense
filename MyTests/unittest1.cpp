#include "stdafx.h"
#include "./globals.hpp"
#include "./file_util.hpp"
#include "./pathfinding/graph_node.hpp"
#include "./pathfinding/grid.hpp"
#include "./pathfinding/pathfinder.hpp"

namespace ih = hoffman::isaiah;
using namespace std::literals::string_literals;
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
			Assert::AreEqual(size_t {4}, neighbor_set_a.size());
			auto neighbor_set_b = my_graph.getNeighbors(3, 3, filter_graph, true);
			Assert::AreEqual(size_t {8}, neighbor_set_b.size());
			try {
				auto neighbor_set_c = my_graph.getNeighbors(0, 3, filter_graph, true);
				Assert::AreEqual(size_t {2}, neighbor_set_c.size());
				auto neighbor_set_d = my_graph.getNeighbors(9, 1, filter_graph, false);
				Assert::AreEqual(size_t {1}, neighbor_set_d.size());
			}
			catch (...) {
				Assert::Fail(L"Exception caught: invalid bounds (x-dimension)!");
			}
			try {
				auto neighbor_set_e = my_graph.getNeighbors(5, 0, filter_graph, true);
				Assert::AreEqual(size_t {3}, neighbor_set_e.size());
				auto neighbor_set_f = my_graph.getNeighbors(4, 7, filter_graph, true);
				Assert::AreEqual(size_t {1}, neighbor_set_f.size());
			}
			catch (...) {
				Assert::Fail(L"Exception caught: invalid bounds (y-dimension)!");
			}
			auto neighbor_set_g = my_graph.getNeighbors(5, 3, filter_graph, true);
			Assert::AreEqual(size_t {4}, neighbor_set_g.size());
		}
		catch (...) {
			Assert::Fail(L"An unexpected exception occurred.");
		}
	}

	// Tests if a grid clears correctly.
	// It also tests that the number of rows and number of columns are correct.
	TEST_METHOD(Pathfinder_Grid_Clear) {
		auto new_graph = pathfinding::Grid {25, 15};
		Assert::AreEqual(25, new_graph.getRows());
		Assert::AreEqual(15, new_graph.getColumns());
		new_graph.getNode(3, 7).setWeight(5);
		new_graph.clearGrid(0);
		Assert::AreEqual(25, new_graph.getRows());
		Assert::AreEqual(15, new_graph.getColumns());
		Assert::AreEqual(0, new_graph.getNode(3, 7).getWeight());
		new_graph.clearGrid(12, 8, 3);
		Assert::AreEqual(12, new_graph.getRows());
		Assert::AreEqual(8, new_graph.getColumns());
		Assert::AreEqual(3, new_graph.getNode(0, 1).getWeight());
	}

	// Tests Pathfinder::findPath
	TEST_METHOD(Pathfinder_Pathfinder_Find_Path) {
		auto terrain_graph_a = pathfinding::Grid {0, 0, 4, 4, {
			{   1,   1,   1, 100,   1},
			{   1, 100, 100,   1,   1},
			{   1, 100,   1,  10,   1},
			{   1,   1,   1, 100,   1},
			{ 100, 100,   1,   1,   1}
		}};
		terrain_graph_a.setStartNode(0, 0);
		terrain_graph_a.setGoalNode(4, 0);
		auto filter_graph_a = pathfinding::Grid {5, 5};
		auto influence_graph = pathfinding::Grid {5, 5};
		auto pathfinder_a = std::make_unique<pathfinding::Pathfinder>(terrain_graph_a, filter_graph_a, influence_graph, false,
			pathfinding::HeuristicStrategies::Manhattan);
		try {
			auto path = pathfinder_a->findPath();
			Assert::AreEqual(size_t {13}, path.size());
			path = pathfinder_a->findPath(5.0);
			Assert::AreEqual(size_t {11}, path.size());
			path = pathfinder_a->findPath(1.0, -1, -1, 4, 4);
			Assert::AreEqual(size_t {9}, path.size());
			path = pathfinder_a->findPath(1.0, 3, 1);
			Assert::AreEqual(size_t {3}, path.size());
			path = pathfinder_a->findPath(1.0, 3, 1, 4, 4);
			Assert::AreEqual(size_t {5}, path.size());
		}
		catch (...) {
			Assert::Fail(L"An exception was thrown.");
		}
	}

	// Tests Pathfinder::checkPathExists() method of pathfinder
	TEST_METHOD(Pathfinder_Pathfinder_Path_Exists) {
		auto terrain_graph_a = pathfinding::Grid {0, 0, 4, 4, {
			{  1, 100, 100, 100, 100},
			{  1, 100, 100, 100, 100},
			{  1,   1,   1,   1, 100},
			{100, 100, 100,   1, 100},
			{100, 100, 100,   1,   1}
		}};
		auto filter_graph_a = pathfinding::Grid {5, 5};
		auto influence_graph = pathfinding::Grid {5, 5};
		auto pathfinder_nd_a = std::make_unique<pathfinding::Pathfinder>(terrain_graph_a, filter_graph_a, influence_graph, false,
			pathfinding::HeuristicStrategies::Manhattan);
		Assert::IsTrue(pathfinder_nd_a->checkPathExists());
		auto pathfinder_d_a = std::make_unique<pathfinding::Pathfinder>(terrain_graph_a, filter_graph_a, influence_graph, true,
			pathfinding::HeuristicStrategies::Manhattan);
		Assert::IsTrue(pathfinder_d_a->checkPathExists());
		auto terrain_graph_b = terrain_graph_a;
		terrain_graph_b.getNode(0, 1).setBlockage(true);
		terrain_graph_b.getNode(1, 1).setWeight(1);
		auto filter_graph_b = filter_graph_a;
		auto pathfinder_nd_b = std::make_unique<pathfinding::Pathfinder>(terrain_graph_b, filter_graph_b, influence_graph, false,
			pathfinding::HeuristicStrategies::Manhattan);
		Assert::IsFalse(pathfinder_nd_b->checkPathExists());
		auto pathfinder_d_b = std::make_unique<pathfinding::Pathfinder>(terrain_graph_b, filter_graph_b, influence_graph, true,
			pathfinding::HeuristicStrategies::Manhattan);
		Assert::IsTrue(pathfinder_d_b->checkPathExists());
	}
		};

		TEST_CLASS(Datafiles) {
		public:
			TEST_METHOD(Datafile_Quoted_String) {
				try {
					// TODO: Rewrite
					/*
					std::wistringstream my_string {L"\"My Quoted Input\""s};
					int line_number = 1;
					auto result = ih::util::file::getNextToken(my_string, line_number).second;
					Assert::AreEqual(L"My Quoted Input"s, result);
					std::wistringstream my_second_str {L"\"My \\\"Escaped\\\" Quoted Input!\""s};
					result = ih::util::file::getNextToken(my_second_str, line_number).second;
					Assert::AreEqual(L"My \"Escaped\" Quoted Input!"s, result);
					std::wistringstream my_fourth_str {L"\"My hack string cut\"off by a quote.\""s};
					result = ih::util::file::getNextToken(my_fourth_str, line_number).second;
					Assert::AreEqual(L"My hack string cut"s, result);
					std::wistringstream my_fifth_str {L"\"My perfectly normal string with a \\ in it.\""s};
					result = ih::util::file::getNextToken(my_fifth_str, line_number).second;
					Assert::AreEqual(L"My perfectly normal string with a \\ in it."s, result);
					std::wistringstream my_sixth_str {L"\"My \\\"\\\"double quoted\\\"\\\" string\""s};
					result = ih::util::file::getNextToken(my_sixth_str, line_number).second;
					*/
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}
		};
	}
}