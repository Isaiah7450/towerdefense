// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./enemy_type.hpp"
#include "./my_game.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace game {
		void MyGame::init_enemy_types() {
			std::wifstream data_file {L"./resources/enemies.ini"s};
			if (data_file.fail() || data_file.bad()) {
				throw util::file::DataFileException {L"Could not load the enemy data file (enemies.ini)."s, 0};
			}
			std::wstring buffer {};
			int line {1};
			// Globals section
			auto my_token = util::file::getNextToken(data_file, line);
			if (!util::file::matchToken(util::file::TokenTypes::Section, L"global"s, my_token)) {
				throw util::file::DataFileException {L"The enemy data file must begin with the"s
					L" [global] section."s, line};
			}
			my_token = util::file::getKeyValue(L"version"s, data_file, line);
			if (!util::file::matchToken(util::file::TokenTypes::Number, L"1"s, my_token)) {
				throw util::file::DataFileException {L"Expected the value 1 for the version number."s, line};
			}
			// Buff Targets section
			my_token = util::file::getNextToken(data_file, line);
			if (!util::file::matchToken(util::file::TokenTypes::Section, L"buff_targets"s, my_token)) {
				throw util::file::DataFileException {L"Expected the [buff_targets] section."s, line};
			}
			if (util::file::verifyObjectStart(L"targets"s, data_file, line)) {
				// Key => Group Name, Value => List of enemy names associated with that group name
				std::map<std::wstring, std::vector<std::wstring>> buff_target_groups {};
				while (true) {
					my_token = util::file::getNextToken(data_file, line);
					if (util::file::matchToken(util::file::TokenTypes::Object, L"}"s, my_token)
						|| util::file::matchToken(util::file::TokenTypes::Object, L"{}"s, my_token)) {
						my_token = util::file::getNextToken(data_file, line);
						break;
					}
					else if (!util::file::matchTokenType(util::file::TokenTypes::Object, my_token.first)) {
						throw util::file::DataFileException {L"Expected either an opening or closing brace."s, line};
					}
					// (Yes, this means the quotes are optional in some cases...)
					std::wstring group_name = util::file::getKeyValue(L"group_name"s, data_file, line).second;
					my_token = util::file::getKeyValue(L"target_names"s, data_file, line);
					if (!util::file::matchTokenType(util::file::TokenTypes::List, my_token.first)) {
						throw util::file::DataFileException {L"Expected a list of names following the equal signs."s
							L" (Start the list with < and end it with >.)"s, line};
					}
					std::vector<std::wstring> target_names = util::file::parseList(my_token, data_file, line);
					buff_target_groups.emplace(group_name, target_names);
				}
			} // End Buff Targets Section
			do {
				// Enemy Sections
				if (!util::file::matchToken(util::file::TokenTypes::Section, L"enemy"s, my_token)) {
					throw util::file::DataFileException {L"Expected either the end of the file"s
						L" or another [enemy] section."s, line};
				}
				my_token = util::file::getKeyValue(L"name"s, data_file, line);
				std::wstring n = util::file::parseString(my_token, line);
				my_token = util::file::getKeyValue(L"desc"s, data_file, line);
				std::wstring d = util::file::parseString(my_token, line);
				graphics::Color c = util::file::readColor(data_file, line);
				graphics::shapes::ShapeTypes st = util::file::readShape(data_file, line);
				my_token = util::file::getKeyValue(L"damage"s, data_file, line);
				int dmg = static_cast<int>(util::file::parseNumber(my_token, line));
				if (dmg <= 0) {
					throw util::file::DataFileException {L"Damage must be positive."s, line};
				}
				my_token = util::file::getKeyValue(L"health"s, data_file, line);
				double hp = util::file::parseNumber(my_token, line);
				if (hp <= 0.0) {
					throw util::file::DataFileException {L"Health must be positive."s, line};
				}
				my_token = util::file::getKeyValue(L"armor_health"s, data_file, line);
				double ahp = util::file::parseNumber(my_token, line);
				if (ahp < 0.0) {
					throw util::file::DataFileException {L"Armor health must be non-negative."s, line};
				}
				my_token = util::file::getKeyValue(L"armor_reduce"s, data_file, line);
				double ar = util::file::parseNumber(my_token, line);
				if (ar < 0.0 || ar > 1.0) {
					throw util::file::DataFileException {L"Armor reduction must be between 0 and 1 inclusive."s, line};
				}
				my_token = util::file::getKeyValue(L"pain_tolerance"s, data_file, line);
				double pt = util::file::parseNumber(my_token, line);
				if (pt <= 0.0 || pt >= 1.0) {
					throw util::file::DataFileException {L"Pain tolerance must be between 0 and 1 exclusive."s, line};
				}
				my_token = util::file::getKeyValue(L"walking_speed"s, data_file, line);
				double wspd = util::file::parseNumber(my_token, line);
				if (wspd < 0.5 || wspd > 25.0) {
					throw util::file::DataFileException {L"Walking speed must be between 0.5 and 25.0 inclusive."s, line};
				}
				my_token = util::file::getKeyValue(L"running_speed"s, data_file, line);
				double rspd = util::file::parseNumber(my_token, line);
				if (rspd < wspd) {
					throw util::file::DataFileException {L"An enemy's running speed cannot be less than"s
						L" their walking speed."s, line};
				}
				else if (rspd > 25.0) {
					throw util::file::DataFileException {L"Running speed must be between 0.5 and 25.0 inclusive."s, line};
				}
				my_token = util::file::getKeyValue(L"injured_speed"s, data_file, line);
				double ispd = util::file::parseNumber(my_token, line);
				if (ispd > wspd) {
					throw util::file::DataFileException {L"An enemy's injured speed cannot exceed thier"s
						L" walking speed."s, line};
				}
				else if (ispd < 0.5) {
					throw util::file::DataFileException {L"Injured speed must be between 0.5 and 25.0 inclusive."s, line};
				}
				my_token = util::file::getKeyValue(L"strategy"s, data_file, line);
				pathfinding::HeuristicStrategies strat =
					my_token.second == L"Manhattan"s ? pathfinding::HeuristicStrategies::Manhattan
					: my_token.second == L"Euclidean"s ? pathfinding::HeuristicStrategies::Euclidean
					: my_token.second == L"Diagonal"s ? pathfinding::HeuristicStrategies::Diagonal
					: my_token.second == L"Maximum"s ? pathfinding::HeuristicStrategies::Max_Dx_Dy
					: throw util::file::DataFileException {L"Invalid strategy constant specified."s, line};
				my_token = util::file::getKeyValue(L"can_move_diagonally"s, data_file, line);
				bool move_diag = util::file::parseBoolean(my_token, line);
				my_token = util::file::getKeyValue(L"is_flying"s, data_file, line);
				bool fly = util::file::parseBoolean(my_token, line);
				if (util::file::verifyObjectStart(L"buffs", data_file, line)) {
					// TODO: Buffs
				}
				else {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly);
					this->enemy_types.emplace_back(std::move(my_type));
				}
				my_token = util::file::getNextToken(data_file, line);
			} while (!util::file::matchTokenType(util::file::TokenTypes::End_Of_File, my_token.first));
		}
	}
}