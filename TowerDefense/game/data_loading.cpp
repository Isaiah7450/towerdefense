// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <stdexcept>
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./enemy_type.hpp"
#include "./my_game.hpp"
#include "./shot_types.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace game {
		void MyGame::init_enemy_types() {
#pragma warning(disable: 4996)
			std::wifstream data_file {L"./resources/enemies.ini"s};
			if (data_file.fail() || data_file.bad()) {
				throw util::file::DataFileException {L"Could not load the enemy data file (enemies.ini)."s, 0};
			}
			// (I might eventually rewrite this so that all the legacy code is gone...)
			// (But this will do for the time being.)
			auto my_parser {std::make_unique<util::file::DataFileParser>(data_file)};
			// Globals section
			if (!my_parser->matchToken(util::file::TokenTypes::Section, L"global"s)) {
				throw util::file::DataFileException {L"The enemy data file must begin with the"s
					L" [global] section."s, my_parser->getLineNumber()};
			}
			auto my_token = my_parser->readKeyValue(L"version"s);
			if (!util::file::matchToken(util::file::TokenTypes::Number, L"1"s, my_token)) {
				throw util::file::DataFileException {L"Expected the value 1 for the version number."s,
					my_parser->getLineNumber()};
			}
			// Buff Targets section
			my_parser->getNext();
			if (!my_parser->matchToken(util::file::TokenTypes::Section, L"buff_targets"s)) {
				throw util::file::DataFileException {L"Expected the [buff_targets] section."s, my_parser->getLineNumber()};
			}
			my_parser->readKeyValue(L"targets"s);
			if (!my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
				throw util::file::DataFileException {L"Expected the start of an object ({)."s, my_parser->getLineNumber()};
			}
			// Key => Group Name, Value => List of enemy names associated with that group name
			std::map<std::wstring, std::vector<std::wstring>> buff_target_groups {};
			while (true) {
				my_parser->getNext();
				if (my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					my_parser->getNext();
					break;
				}
				else if (!my_parser->matchTokenType(util::file::TokenTypes::Object)) {
					throw util::file::DataFileException {L"Expected either an opening or closing brace."s,
						my_parser->getLineNumber()};
				}
				// (Yes, this means the quotes are optional in some cases...)
				std::wstring group_name = my_parser->readKeyValue(L"group_name"s).second;
				my_token = my_parser->readKeyValue(L"target_names"s);
				if (!util::file::matchTokenType(util::file::TokenTypes::List, my_token.first)) {
					throw util::file::DataFileException {L"Expected a list of names following the equal signs."s
						L" (Start the list with < and end it with >.)"s, my_parser->getLineNumber()};
				}
				std::vector<std::wstring> target_names = my_parser->readList();
				buff_target_groups.emplace(group_name, target_names);
				my_parser->getNext();
				if (!my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					throw util::file::DataFileException {L"Missing ending brace (})."s, my_parser->getLineNumber()};
				}
			}
			do {
				// Enemy Sections
				if (!my_parser->matchToken(util::file::TokenTypes::Section, L"enemy"s)) {
					throw util::file::DataFileException {L"Expected either the end of the file"s
						L" or another [enemy] section."s, my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"name"s);
				std::wstring n = util::file::parseString(my_token, my_parser->getLineNumber());
				my_token = my_parser->readKeyValue(L"desc"s);
				std::wstring d = util::file::parseString(my_token, my_parser->getLineNumber());
				graphics::Color c = my_parser->readColor();
				graphics::shapes::ShapeTypes st = my_parser->readShape();
				my_token = my_parser->readKeyValue(L"damage"s);
				int dmg = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLineNumber()));
				if (dmg <= 0) {
					throw util::file::DataFileException {L"Damage must be positive."s, my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"health"s);
				double hp = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (hp <= 0.0) {
					throw util::file::DataFileException {L"Health must be positive."s, my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"armor_health"s);
				double ahp = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (ahp < 0.0) {
					throw util::file::DataFileException {L"Armor health must be non-negative."s, my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"armor_reduce"s);
				double ar = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (ar < 0.0 || ar > 1.0) {
					throw util::file::DataFileException {L"Armor reduction must be between 0 and 1 inclusive."s,
						my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"pain_tolerance"s);
				double pt = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (pt <= 0.0 || pt >= 1.0) {
					throw util::file::DataFileException {L"Pain tolerance must be between 0 and 1 exclusive."s,
						my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"walking_speed"s);
				double wspd = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (wspd < 0.5 || wspd > 25.0) {
					throw util::file::DataFileException {L"Walking speed must be between 0.5 and 25.0 inclusive."s,
						my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"running_speed"s);
				double rspd = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (rspd < wspd) {
					throw util::file::DataFileException {L"An enemy's running speed cannot be less than"s
						L" their walking speed."s, my_parser->getLineNumber()};
				}
				else if (rspd > 25.0) {
					throw util::file::DataFileException {L"Running speed must be between 0.5 and 25.0 inclusive."s,
						my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"injured_speed"s);
				double ispd = util::file::parseNumber(my_token, my_parser->getLineNumber());
				if (ispd > wspd) {
					throw util::file::DataFileException {L"An enemy's injured speed cannot exceed thier"s
						L" walking speed."s, my_parser->getLineNumber()};
				}
				else if (ispd < 0.5) {
					throw util::file::DataFileException {L"Injured speed must be between 0.5 and 25.0 inclusive."s,
					my_parser->getLineNumber()};
				}
				my_token = my_parser->readKeyValue(L"strategy"s);
				pathfinding::HeuristicStrategies strat =
					my_token.second == L"Manhattan"s ? pathfinding::HeuristicStrategies::Manhattan
					: my_token.second == L"Euclidean"s ? pathfinding::HeuristicStrategies::Euclidean
					: my_token.second == L"Diagonal"s ? pathfinding::HeuristicStrategies::Diagonal
					: my_token.second == L"Maximum"s ? pathfinding::HeuristicStrategies::Max_Dx_Dy
					: throw util::file::DataFileException {L"Invalid strategy constant specified."s,
						my_parser->getLineNumber()};
				my_token = my_parser->readKeyValue(L"can_move_diagonally"s);
				bool move_diag = util::file::parseBoolean(my_token, my_parser->getLineNumber());
				my_token = my_parser->readKeyValue(L"is_flying"s);
				bool fly = util::file::parseBoolean(my_token, my_parser->getLineNumber());
				my_parser->readKeyValue(L"buffs"s);
				if (!my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
					throw util::file::DataFileException {L"Expected the start of an object definition."s,
						my_parser->getLineNumber()};
				}
				my_parser->getNext();
				// Case when we have {} next to buffs
				if (my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly);
					this->enemy_types.emplace_back(std::move(my_type));
				}
				std::vector<std::shared_ptr<BuffBase>> my_buffs {};
				while (!my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					if (!my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
						throw util::file::DataFileException {L"Expected opening brace ({)."s, my_parser->getLineNumber()};
					}
					my_token = my_parser->readKeyValue(L"type"s);
					BuffTypes buff_type = my_token.second == L"Intelligence"s ? BuffTypes::Intelligence
						: my_token.second == L"Speed"s ? BuffTypes::Speed
						: my_token.second == L"Healer"s ? BuffTypes::Healer
						: my_token.second == L"Purify"s ? BuffTypes::Purify
						: throw util::file::DataFileException {L"Expected the type of the buff."s
							L" Valid values include: Intelligence, Speed, Healer, Purify"s, my_parser->getLineNumber()};
					my_token = my_parser->readKeyValue(L"targets"s);
					std::wstring buff_group = util::file::parseString(my_token, my_parser->getLineNumber());
					my_token = my_parser->readKeyValue(L"radius"s);
					try {
						// Gotta find some way to tell the compiler NOT
						// to optimize away this statement.
						(void)buff_target_groups.at(buff_group);
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"Invalid target group name specified."s,
							my_parser->getLineNumber()};
					}
					double buff_radius = util::file::parseNumber(my_token, my_parser->getLineNumber());
					if (buff_radius <= 0.0) {
						throw util::file::DataFileException {L"Buff radius must be positive."s, my_parser->getLineNumber()};
					}
					my_token = my_parser->readKeyValue(L"delay"s);
					int buff_delay = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLineNumber()));
					if (buff_delay < 10 || buff_delay > 60000) {
						throw util::file::DataFileException {L"Buff delay must be between 10 and 60,000 inclusive."s,
							my_parser->getLineNumber()};
					}
					[[maybe_unused]] int buff_duration {};
					// Duration is common to many buffs, so I don't switch on it.
					if (buff_type == BuffTypes::Intelligence || buff_type == BuffTypes::Speed) {
						my_token = my_parser->readKeyValue(L"duration"s);
						buff_duration = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLineNumber()));
						if (buff_duration < 10) {
							throw util::file::DataFileException {L"Buff duration must be >= 10ms."s,
								my_parser->getLineNumber()};
						}
					}
					// Read buff-specific attributes
					switch (buff_type) {
					case BuffTypes::Intelligence:
					{
						auto smart_buff = std::make_shared<SmartBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_duration);
						my_buffs.emplace_back(std::move(smart_buff));
						break;
					}
					case BuffTypes::Speed:
					{
						my_token = my_parser->readKeyValue(L"walking_speed_boost"s);
						double buff_wspd = util::file::parseNumber(my_token, my_parser->getLineNumber());
						if (buff_wspd < 0.0) {
							throw util::file::DataFileException {L"Walking speed boost should be non-negative."s,
								my_parser->getLineNumber()};
						}
						my_token = my_parser->readKeyValue(L"running_speed_boost"s);
						double buff_rspd = util::file::parseNumber(my_token, my_parser->getLineNumber());
						if (buff_rspd < 0.0) {
							throw util::file::DataFileException {L"Running speed boost should be non-negative."s,
								my_parser->getLineNumber()};
						}
						my_token = my_parser->readKeyValue(L"injured_speed_boost"s);
						double buff_ispd = util::file::parseNumber(my_token, my_parser->getLineNumber());
						if (buff_ispd < 0.0) {
							throw util::file::DataFileException {L"Injured speed boost should be non-negative."s,
								my_parser->getLineNumber()};
						}
						if (buff_ispd == 0.0 && buff_rspd == 0. && buff_wspd == 0.) {
							throw util::file::DataFileException {L"You should set at least one of the speed"s
								L" boosts to a positive value!"s, my_parser->getLineNumber()};
						}
						auto speed_buff = std::make_shared<SpeedBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_duration, buff_wspd, buff_rspd, buff_ispd);
						my_buffs.emplace_back(std::move(speed_buff));
						break;
					}
					case BuffTypes::Healer:
					{
						my_token = my_parser->readKeyValue(L"heal_amount"s);
						double buff_heal = util::file::parseNumber(my_token, my_parser->getLineNumber());
						if (buff_heal <= 0.0) {
							throw util::file::DataFileException {L"Heal amount should be positive."s,
								my_parser->getLineNumber()};
						}
						auto heal_buff = std::make_shared<HealerBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_heal);
						my_buffs.emplace_back(std::move(heal_buff));
						break;
					}
					case BuffTypes::Purify:
					{
						my_token = my_parser->readKeyValue(L"purify_max_effects"s);
						int buff_cure_max = static_cast<int>(util::file::parseNumber(my_token,
							my_parser->getLineNumber()));
						if (buff_cure_max < 1) {
							throw util::file::DataFileException {L"Purify max effects must be positive."s,
								my_parser->getLineNumber()};
						}
						auto purify_buff = std::make_shared<PurifyBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_cure_max);
						my_buffs.emplace_back(std::move(purify_buff));
						break;
					}
					} // End switch, this looks weird but is correct
					my_parser->getNext();
					if (!my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
						throw util::file::DataFileException {L"Expected closing brace (})."s,
							my_parser->getLineNumber()};
					}
					my_parser->getNext();
					if (!my_parser->matchTokenType(util::file::TokenTypes::Object)) {
						throw util::file::DataFileException {L"Expected an opening or closing brace."s,
							my_parser->getLineNumber()};
					}
				}
				if (my_buffs.size() > 0) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly, std::move(my_buffs));
					this->enemy_types.emplace_back(std::move(my_type));
				}
			} while (my_parser->getNext());
#pragma warning(error: 4996)
		}
	}
}