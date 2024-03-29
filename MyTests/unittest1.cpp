#include "stdafx.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "./../TowerDefense/file_util.hpp"
#include "./../TowerDefense/globals.hpp"
#include "./../TowerDefense/resource.h"
#include "./../TowerDefense/game/game_level.hpp"
#include "./../TowerDefense/game/my_game.hpp"
#include "./../TowerDefense/game/shot.hpp"
#include "./../TowerDefense/game/tower.hpp"
#include "./../TowerDefense/pathfinding/graph_node.hpp"
#include "./../TowerDefense/pathfinding/grid.hpp"
#include "./../TowerDefense/pathfinding/pathfinder.hpp"

namespace ih = hoffman_isaiah;
using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace hoffman_isaiah {
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
		auto pathfinder_a = pathfinding::Pathfinder {terrain_graph_a, filter_graph_a, influence_graph, false,
			pathfinding::HeuristicStrategies::Manhattan};
		auto pathfinder_b = pathfinding::Pathfinder {terrain_graph_a, filter_graph_a, influence_graph, true,
			pathfinding::HeuristicStrategies::Diagonal};
		try {
			auto path = pathfinder_a.findPath(1.0);
			Assert::AreEqual(size_t {13}, path.size());
			path = pathfinder_a.findPath(1.0, -1, -1, -1, -1, 5.0);
			Assert::AreEqual(size_t {11}, path.size());
			path = pathfinder_a.findPath(1.0, -1, -1, 4, 4);
			Assert::AreEqual(size_t {9}, path.size());
			path = pathfinder_a.findPath(1.0, 3, 1);
			Assert::AreEqual(size_t {3}, path.size());
			path = pathfinder_a.findPath(1.0, 3, 1, 4, 4);
			Assert::AreEqual(size_t {5}, path.size());
			path = pathfinder_b.findPath(1.0, 2, 0, 2, 2);
			Assert::AreEqual(size_t {3}, path.size());
			path = pathfinder_b.findPath(1.0, 0, 0, 0, 0);
			Assert::AreEqual(size_t {1}, path.size());
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
			TEST_METHOD(Datafile_EOF_Detection) {
				try {
					std::wistringstream str_one {L"Hello 123 EOF"};
					ih::util::file::DataFileParser p1 {str_one};
					Assert::IsTrue(p1.getNext());
					Assert::IsFalse(p1.getNext());
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Datafile_Token_Classification) {
				namespace ih_file = ih::util::file;
				try {
					std::wistringstream str_one {L"Hello [Hello] \"Hello\" 1 <abc> <\"a\", \"b\">"};
					ih_file::DataFileParser parser_one {str_one};
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::Identifier));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::Section));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::String));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::Number));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::List));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::Identifier));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::List));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::List));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::String));
					Assert::IsTrue(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::String));
					Assert::IsFalse(parser_one.getNext());
					Assert::IsTrue(parser_one.matchTokenType(ih_file::TokenTypes::List));
					std::wistringstream str_two {L"{xyz = 123} {}"s};
					ih::util::file::DataFileParser parser_two {str_two};
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Object));
					parser_two.getNext();
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Identifier));
					parser_two.getNext();
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Identifier));
					parser_two.getNext();
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Number));
					parser_two.getNext();
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Object));
					parser_two.getNext();
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Object));
					Assert::IsFalse(parser_two.getNext());
					Assert::IsTrue(parser_two.matchTokenType(ih_file::TokenTypes::Object));
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Datafile_Quoted_String) {
				try {
					std::wistringstream my_string {L"\"My Quoted Input\""s};
					ih::util::file::DataFileParser my_parser {my_string};
					Assert::AreEqual(L"My Quoted Input"s, my_parser.getToken());
					std::wistringstream my_second_str {L"\"My \\\"Escaped\\\" Quoted Input!\""s};
					ih::util::file::DataFileParser my_parser2 {my_second_str};
					Assert::AreEqual(L"My \"Escaped\" Quoted Input!"s, my_parser2.getToken());
					std::wistringstream my_third_str {L"\"My hack string cut\"off by a quote.\""s};
					ih::util::file::DataFileParser my_parser3 {my_third_str};
					Assert::AreEqual(L"My hack string cut"s, my_parser3.getToken());
					std::wistringstream my_fourth_str {L"\"My perfectly normal string with a \\ in it.\""s};
					ih::util::file::DataFileParser my_parser4 {my_fourth_str};
					Assert::AreEqual(L"My perfectly normal string with a \\ in it."s, my_parser4.getToken());
					std::wistringstream my_fifth_str {L"\"My \\\"\\\"double quoted\\\"\\\" string\""s};
					ih::util::file::DataFileParser my_parser5 {my_fifth_str};
					Assert::AreEqual(L"My \"\"double quoted\"\" string"s, my_parser5.getToken());
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Datafile_Reading) {
				try {
					namespace ih_file = ih::util::file;
					std::wistringstream s1 {L"[hello] abc = 123 <3, 17, 12, 6>"};
					ih_file::DataFileParser p1 {s1};
					Assert::IsTrue(p1.matchToken(ih_file::TokenTypes::Section, L"hello"));
					const auto r1 = p1.readKeyValue(L"abc");
					Assert::IsTrue(r1.first == ih_file::TokenTypes::Number);
					Assert::IsTrue(r1.second == L"123"s);
					p1.getNext();
					const auto r2 = p1.readList<int>();
					Assert::AreEqual(static_cast<size_t>(4), r2.size());
					Assert::AreEqual(3, r2[0]);
					Assert::AreEqual(17, r2[1]);
					Assert::AreEqual(12, r2[2]);
					Assert::AreEqual(6, r2[3]);
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Datafile_Comment_Skipping) {
				try {
					namespace ih_file = ih::util::file;
					std::wistringstream s1 {L"# This is a comment.\nHello ; Another comment.\n123\t\t\t\"abc\""};
					ih_file::DataFileParser p1 {s1};
					Assert::IsTrue(p1.matchTokenValue(L"Hello"s));
					Assert::IsTrue(p1.getNext());
					Assert::IsTrue(p1.matchTokenType(ih_file::TokenTypes::Number));
					Assert::IsFalse(p1.getNext());
					Assert::IsTrue(p1.matchToken(ih_file::TokenTypes::String, L"abc"));
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}
		};

		TEST_CLASS(Main_Game) {
		public:
			TEST_METHOD(Main_Game_Construction) {
				try {
					const ih::game::MyGame my_game {nullptr};
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Main_Game_Initialization) {
				// Note that it is very possible for this to fail simply because the data files
				// have some issue. Perhaps not great for a unit test, but it should work.
				try {
					ih::game::MyGame my_game {nullptr};
					initGame(my_game);
					Assert::IsTrue(my_game.getAllTowerTypes().size() > 0);
					Assert::IsTrue(my_game.getAllShotTypes().size() > 0);
					Assert::IsTrue(my_game.getAllEnemyTypes().size() > 0);
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}
			
			TEST_METHOD(Main_Game_Save_Load) {
				try {
					ih::game::MyGame my_game {nullptr};
					this->initGame(my_game);
					this->initGame2(my_game);
					my_game.buyHealth();
					const double old_cash = my_game.getPlayerCash();
					const double old_diffi = my_game.getDifficulty();
					const int old_health_buy_cost = my_game.getHealthBuyCost();
					std::wstringstream save_file {};
					my_game.saveGame(save_file);
					my_game.changePlayerCash(100);
					my_game.buyHealth();
					my_game.loadGame(save_file);
					Assert::AreEqual(old_cash, my_game.getPlayerCash());
					Assert::AreEqual(old_diffi, my_game.getDifficulty());
					Assert::AreEqual(old_health_buy_cost, my_game.getHealthBuyCost());
					// @TODO: Add second test case. Probably going to be by some kind
					// of external file (although that's liable to break with a save
					// format update.)
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}

			TEST_METHOD(Main_Game_Reset_State) {
				try {
					constexpr const auto tol = 0.0000005;
					ih::game::MyGame my_game {nullptr};
					initGame(my_game);
					initGame2(my_game);
					Assert::AreEqual(1, my_game.getChallengeLevel());
					Assert::AreEqual(1, my_game.getLevelNumber());
					Assert::IsTrue(std::abs(my_game.getDifficulty() - 1.0) <= tol);
					Assert::IsTrue(my_game.getEnemies().size() == 0);
					Assert::IsTrue(my_game.getTowers().size() == 0);
					const auto is_seen = [](const auto& e) {
						return e.second;
					};
					const auto& seen_enemies = my_game.getSeenEnemies();
					Assert::IsFalse(std::any_of(seen_enemies.cbegin(), seen_enemies.cend(),
						is_seen));
					Assert::IsFalse(my_game.isPaused());
					Assert::IsFalse(my_game.isInLevel());
					Assert::AreEqual(L"intermediate"s, my_game.getMapBaseName());
					my_game.resetState(2, L"expert"s);
					my_game.setEnemyTypeAsSeen(L"Red Scout");
					// @TODO: Figure out how to make enemies without needing graphics.
					// It's probably an easy fix, but I will have to investigate.
					// my_game.startWave();
					while (my_game.isInLevel()) {
						my_game.update();
					}
					// my_game.startWave();
					my_game.update();
					my_game.update();
					my_game.togglePause();
					// One missing piece is adding towers...
					this->initGame2(my_game);
					Assert::AreEqual(1, my_game.getChallengeLevel());
					Assert::AreEqual(1, my_game.getLevelNumber());
					Assert::IsTrue(std::abs(my_game.getDifficulty() - 1.0) <= tol);
					Assert::IsTrue(my_game.getEnemies().size() == 0);
					Assert::IsTrue(my_game.getTowers().size() == 0);
					Assert::IsFalse(std::any_of(seen_enemies.cbegin(), seen_enemies.cend(),
						is_seen));
					Assert::IsFalse(my_game.isPaused());
					Assert::IsFalse(my_game.isInLevel());
					Assert::AreEqual(L"intermediate"s, my_game.getMapBaseName());
				}
				catch (const ih::util::file::DataFileException& e) {
					Assert::Fail(e.what());
				}
				catch (...) {
					Assert::Fail(L"An exception occurred.");
				}
			}
		protected:
		private:
			void initGame(ih::game::MyGame& my_game) {
				my_game.load_config_data();
				my_game.init_enemy_types();
				my_game.init_shot_types();
				my_game.init_tower_types();
				my_game.load_tower_upgrades_data();
				my_game.load_global_level_data();
				my_game.load_global_misc_data();
			}

			void initGame2(ih::game::MyGame& my_game) {
				const std::wstring map_name = my_game.getDefaultMapName(ID_CHALLENGE_LEVEL_NORMAL);
				my_game.resetState(ID_CHALLENGE_LEVEL_NORMAL - ID_CHALLENGE_LEVEL_EASY, map_name);
			}
		};
	}
}