// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <stdexcept>
#include <queue>
#include <deque>
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./../graphics/graphics.hpp"
#include "./../graphics/shapes.hpp"
#include "./enemy_type.hpp"
#include "./game_level.hpp"
#include "./my_game.hpp"
#include "./status_effects.hpp"
#include "./shot_types.hpp"
#include "./tower_types.hpp"
#include "./tower.hpp"
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
			my_parser->expectToken(util::file::TokenTypes::Section, L"global"s);
			auto my_token = my_parser->readKeyValue(L"version"s);
			my_parser->expectToken(util::file::TokenTypes::Number, L"1"s);
			// Buff Targets section
			my_parser->getNext();
			my_parser->expectToken(util::file::TokenTypes::Section, L"buff_targets"s);
			my_parser->readKeyValue(L"targets"s);
			my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
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
						my_parser->getLine()};
				}
				// (Yes, this means the quotes are optional in some cases...)
				std::wstring group_name = my_parser->readKeyValue(L"group_name"s).second;
				my_token = my_parser->readKeyValue(L"target_names"s);
				if (!util::file::matchTokenType(util::file::TokenTypes::List, my_token.first)) {
					throw util::file::DataFileException {L"Expected a list of names following the equal signs."s
						L" (Start the list with < and end it with >.)"s, my_parser->getLine()};
				}
				std::vector<std::wstring> target_names = my_parser->readList();
				buff_target_groups.emplace(group_name, target_names);
				my_parser->getNext();
				my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
			}
			do {
				// Enemy Sections
				my_parser->expectToken(util::file::TokenTypes::Section, L"enemy"s);
				my_token = my_parser->readKeyValue(L"name"s);
				std::wstring n = util::file::parseString(my_token, my_parser->getLine());
				my_token = my_parser->readKeyValue(L"desc"s);
				std::wstring d = util::file::parseString(my_token, my_parser->getLine());
				graphics::Color c = my_parser->readColor();
				graphics::shapes::ShapeTypes st = my_parser->readShape();
				my_token = my_parser->readKeyValue(L"damage"s);
				int dmg = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLine()));
				if (dmg <= 0) {
					throw util::file::DataFileException {L"Damage must be positive."s, my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"health"s);
				double hp = util::file::parseNumber(my_token, my_parser->getLine());
				if (hp <= 0.0) {
					throw util::file::DataFileException {L"Health must be positive."s, my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"armor_health"s);
				double ahp = util::file::parseNumber(my_token, my_parser->getLine());
				if (ahp < 0.0) {
					throw util::file::DataFileException {L"Armor health must be non-negative."s, my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"armor_reduce"s);
				double ar = util::file::parseNumber(my_token, my_parser->getLine());
				if (ar < 0.0 || ar > 1.0) {
					throw util::file::DataFileException {L"Armor reduction must be between 0 and 1 inclusive."s,
						my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"pain_tolerance"s);
				double pt = util::file::parseNumber(my_token, my_parser->getLine());
				if (pt <= 0.0 || pt >= 1.0) {
					throw util::file::DataFileException {L"Pain tolerance must be between 0 and 1 exclusive."s,
						my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"walking_speed"s);
				double wspd = util::file::parseNumber(my_token, my_parser->getLine());
				if (wspd < 0.5 || wspd > 25.0) {
					throw util::file::DataFileException {L"Walking speed must be between 0.5 and 25.0 inclusive."s,
						my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"running_speed"s);
				double rspd = util::file::parseNumber(my_token, my_parser->getLine());
				if (rspd < wspd) {
					throw util::file::DataFileException {L"An enemy's running speed cannot be less than"s
						L" their walking speed."s, my_parser->getLine()};
				}
				else if (rspd > 25.0) {
					throw util::file::DataFileException {L"Running speed must be between 0.5 and 25.0 inclusive."s,
						my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"injured_speed"s);
				double ispd = util::file::parseNumber(my_token, my_parser->getLine());
				if (ispd > wspd) {
					throw util::file::DataFileException {L"An enemy's injured speed cannot exceed thier"s
						L" walking speed."s, my_parser->getLine()};
				}
				else if (ispd < 0.5) {
					throw util::file::DataFileException {L"Injured speed must be between 0.5 and 25.0 inclusive."s,
					my_parser->getLine()};
				}
				my_token = my_parser->readKeyValue(L"strategy"s);
				pathfinding::HeuristicStrategies strat =
					my_token.second == L"Manhattan"s ? pathfinding::HeuristicStrategies::Manhattan
					: my_token.second == L"Euclidean"s ? pathfinding::HeuristicStrategies::Euclidean
					: my_token.second == L"Diagonal"s ? pathfinding::HeuristicStrategies::Diagonal
					: my_token.second == L"Maximum"s ? pathfinding::HeuristicStrategies::Max_Dx_Dy
					: throw util::file::DataFileException {L"Invalid strategy constant specified."s,
						my_parser->getLine()};
				my_parser->readKeyValue(L"can_move_diagonally"s);
				bool move_diag = my_parser->parseBoolean();
				my_parser->readKeyValue(L"is_flying"s);
				bool fly = my_parser->parseBoolean();
				my_parser->readKeyValue(L"is_unique"s);
				bool unique = my_parser->parseBoolean();
				my_parser->readKeyValue(L"buffs"s);
				if (!my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
					throw util::file::DataFileException {L"Expected the start of an object definition."s,
						my_parser->getLine()};
				}
				my_parser->getNext();
				bool insert_succeeded = false;
				// Case when we have {} next to buffs
				if (my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly, unique);
					auto ret = this->enemy_types.emplace(n, std::move(my_type));
					insert_succeeded = ret.second;
				}
				std::vector<std::shared_ptr<BuffBase>> my_buffs {};
				while (!my_parser->matchToken(util::file::TokenTypes::Object, L"}"s)) {
					if (!my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
						throw util::file::DataFileException {L"Expected opening brace ({)."s, my_parser->getLine()};
					}
					my_token = my_parser->readKeyValue(L"type"s);
					BuffTypes buff_type = my_token.second == L"Intelligence"s ? BuffTypes::Intelligence
						: my_token.second == L"Speed"s ? BuffTypes::Speed
						: my_token.second == L"Healer"s ? BuffTypes::Healer
						: my_token.second == L"Purify"s ? BuffTypes::Purify
						: throw util::file::DataFileException {L"Expected the type of the buff."s
							L" Valid values include: Intelligence, Speed, Healer, Purify"s, my_parser->getLine()};
					my_token = my_parser->readKeyValue(L"targets"s);
					std::wstring buff_group = util::file::parseString(my_token, my_parser->getLine());
					my_token = my_parser->readKeyValue(L"radius"s);
					try {
						// Gotta find some way to tell the compiler NOT
						// to optimize away this statement.
						(void)buff_target_groups.at(buff_group);
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"Invalid target group name specified."s,
							my_parser->getLine()};
					}
					double buff_radius = util::file::parseNumber(my_token, my_parser->getLine());
					if (buff_radius <= 0.0) {
						throw util::file::DataFileException {L"Buff radius must be positive."s, my_parser->getLine()};
					}
					my_token = my_parser->readKeyValue(L"delay"s);
					int buff_delay = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLine()));
					if (buff_delay < 10 || buff_delay > 60000) {
						throw util::file::DataFileException {L"Buff delay must be between 10 and 60,000 inclusive."s,
							my_parser->getLine()};
					}
					[[maybe_unused]] int buff_duration {};
					// Duration is common to many buffs, so I don't switch on it.
					if (buff_type == BuffTypes::Intelligence || buff_type == BuffTypes::Speed) {
						my_token = my_parser->readKeyValue(L"duration"s);
						buff_duration = static_cast<int>(util::file::parseNumber(my_token, my_parser->getLine()));
						if (buff_duration < 10) {
							throw util::file::DataFileException {L"Buff duration must be >= 10ms."s,
								my_parser->getLine()};
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
						double buff_wspd = util::file::parseNumber(my_token, my_parser->getLine());
						if (buff_wspd < 0.0) {
							throw util::file::DataFileException {L"Walking speed boost should be non-negative."s,
								my_parser->getLine()};
						}
						my_token = my_parser->readKeyValue(L"running_speed_boost"s);
						double buff_rspd = util::file::parseNumber(my_token, my_parser->getLine());
						if (buff_rspd < 0.0) {
							throw util::file::DataFileException {L"Running speed boost should be non-negative."s,
								my_parser->getLine()};
						}
						my_token = my_parser->readKeyValue(L"injured_speed_boost"s);
						double buff_ispd = util::file::parseNumber(my_token, my_parser->getLine());
						if (buff_ispd < 0.0) {
							throw util::file::DataFileException {L"Injured speed boost should be non-negative."s,
								my_parser->getLine()};
						}
						if (buff_ispd == 0.0 && buff_rspd == 0. && buff_wspd == 0.) {
							throw util::file::DataFileException {L"You should set at least one of the speed"s
								L" boosts to a positive value!"s, my_parser->getLine()};
						}
						auto speed_buff = std::make_shared<SpeedBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_duration, buff_wspd, buff_rspd, buff_ispd);
						my_buffs.emplace_back(std::move(speed_buff));
						break;
					}
					case BuffTypes::Healer:
					{
						my_token = my_parser->readKeyValue(L"heal_amount"s);
						double buff_heal = util::file::parseNumber(my_token, my_parser->getLine());
						if (buff_heal <= 0.0) {
							throw util::file::DataFileException {L"Heal amount should be positive."s,
								my_parser->getLine()};
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
							my_parser->getLine()));
						if (buff_cure_max < 1) {
							throw util::file::DataFileException {L"Purify max effects must be positive."s,
								my_parser->getLine()};
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
							my_parser->getLine()};
					}
					my_parser->getNext();
					if (!my_parser->matchTokenType(util::file::TokenTypes::Object)) {
						throw util::file::DataFileException {L"Expected an opening or closing brace."s,
							my_parser->getLine()};
					}
				}
				if (my_buffs.size() > 0) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly, unique, std::move(my_buffs));
					insert_succeeded = this->enemy_types.emplace(n, std::move(my_type)).second;
				}
				if (!insert_succeeded) {
					throw util::file::DataFileException {L"Duplicate enemy name: "s + n + L"."s, my_parser->getLine()};
				}
			} while (my_parser->getNext());
#pragma warning(error: 4996)
		}

		void MyGame::init_shot_types() {
			std::wifstream data_file {L"./resources/shots.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/shots.ini for reading."s, 0};
			}
			auto my_parser {std::make_unique<util::file::DataFileParser>(data_file)};
			// Global section
			my_parser->expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser->readKeyValue(L"version");
			my_parser->expectToken(util::file::TokenTypes::Number, L"1"s);
			while (my_parser->getNext()) {
				// Shot sections
				my_parser->expectToken(util::file::TokenTypes::Section, L"shot"s);
				my_parser->readKeyValue(L"name"s);
				std::wstring n = my_parser->parseString();
				my_parser->readKeyValue(L"desc"s);
				std::wstring d = my_parser->parseString();
				graphics::Color c = my_parser->readColor();
				graphics::shapes::ShapeTypes st = my_parser->readShape();
				my_parser->readKeyValue(L"damage"s);
				double dmg = my_parser->parseNumber();
				if (dmg < 0.0) {
					throw util::file::DataFileException {L"Damage must be non-negative."s, my_parser->getLine()};
				}
				my_parser->readKeyValue(L"piercing"s);
				double wap = my_parser->parseNumber();
				if (wap < 0.0 || wap > 1.0) {
					throw util::file::DataFileException {L"Piercing must be between 0 and 1 inclusive."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"move_speed"s);
				double ms = my_parser->parseNumber();
				if (ms < 20.00 || ms > 60.00) {
					throw util::file::DataFileException {L"Movement speed must between 20 and 60 inclusive."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"impact_radius"s);
				double ir = my_parser->parseNumber();
				if (ir < 0.0) {
					throw util::file::DataFileException {L"Impact radius must be non-negative."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"splash_damage"s);
				double sdmg = my_parser->parseNumber();
				if (sdmg < 0.0) {
					throw util::file::DataFileException {L"Splash damage must be non-negative."s,
						my_parser->getLine()};
				}
				else if (sdmg > 0.0 && ir <= 0.0) {
					throw util::file::DataFileException {L"Splash damage should be zero if impact"s
						L" radius is zero."s, my_parser->getLine()};
				}
				my_parser->readKeyValue(L"ground_multiplier"s);
				double gm = my_parser->parseNumber();
				if (gm < 0.0) {
					throw util::file::DataFileException {L"Ground mulitplier must be non-negative."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"air_multiplier"s);
				double am = my_parser->parseNumber();
				if (am < 0.0) {
					throw util::file::DataFileException {L"Air multiplier must be non-negative."s,
						my_parser->getLine()};
				}
				if (am <= 0.0 && gm <= 0.0 && (dmg > 0.0 || sdmg > 0.0)) {
					throw util::file::DataFileException {L"Either the ground multiplier or the air multiplier"s
						L" must be positive."s, my_parser->getLine()};
				}
				auto my_type_str = my_parser->readKeyValue(L"type"s).second;
				ShotTypes my_type = my_type_str == L"Standard"s ? ShotTypes::Standard :
					my_type_str == L"Damage_Over_Time"s ? ShotTypes::DoT :
					my_type_str == L"Slow"s ? ShotTypes::Slow :
					my_type_str == L"Stun"s ? ShotTypes::Stun :
					throw util::file::DataFileException {L"Invalid type specified."s, my_parser->getLine()};
				if (my_type == ShotTypes::Standard && dmg <= 0.0 && sdmg <= 0.0) {
					throw util::file::DataFileException {L"The shot should deal some kind of damage or"s
						L" have some kind of special effect."s, my_parser->getLine()};
				}
				[[maybe_unused]] bool affect_splash {false};
				if (my_type != ShotTypes::Standard) {
					my_parser->readKeyValue(L"apply_effect_on_splash"s);
					affect_splash = my_parser->parseBoolean();
					if (!affect_splash && ir > 0.0 && sdmg <= 0.0) {
						throw util::file::DataFileException {L"It is pointless to set impact radius to a"s
							L" value greater than zero if splash damage is zero, and there is no special effect"s
							L" applied on splash."s, my_parser->getLine()};
					}
				}
				bool insert_succeeded = false;
				switch (my_type) {
				case ShotTypes::Standard:
				{
					auto my_shot = std::make_shared<NormalShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am);
					auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::DoT:
				{
					auto dot_type_str = my_parser->readKeyValue(L"dot_damage_type"s).second;
					DoTDamageTypes dot_type = dot_type_str == L"Poison"s ? DoTDamageTypes::Poison :
						dot_type_str == L"Fire"s ? DoTDamageTypes::Fire :
						throw util::file::DataFileException {L"Invalid DoT damage type specified."s,
							my_parser->getLine()};
					my_parser->readKeyValue(L"dot_damage_per_tick"s);
					double dot_tick_dmg = my_parser->parseNumber();
					if (dot_tick_dmg <= 0.0) {
						throw util::file::DataFileException {L"DoT damage per tick must be positive."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"dot_time_between_ticks"s);
					int dot_tick_time = static_cast<int>(my_parser->parseNumber());
					if (dot_tick_time <= 10) {
						throw util::file::DataFileException {L"Time between DoT ticks must be >= 10 ms."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"dot_total_ticks"s);
					int dot_total_ticks = static_cast<int>(my_parser->parseNumber());
					if (dot_total_ticks < 1) {
						throw util::file::DataFileException {L"Total number of DoT ticks must be positive."s,
							my_parser->getLine()};
					}
					auto my_shot = std::make_shared<DoTShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, dot_type, dot_tick_dmg, dot_tick_time, dot_total_ticks);
					auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::Slow:
				{
					my_parser->readKeyValue(L"slow_factor"s);
					double slow_factor = my_parser->parseNumber();
					if (slow_factor <= 0.0 || slow_factor >= 1.0) {
						throw util::file::DataFileException {L"Slow factor must be between 0 and 1 exclusive."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"slow_duration"s);
					int slow_duration = static_cast<int>(my_parser->parseNumber());
					if (slow_duration < 10) {
						throw util::file::DataFileException {L"Slow duration must be >= 10ms."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"slow_multi_chance"s);
					double slow_mchance = my_parser->parseNumber();
					if (slow_mchance < 0.0 || slow_mchance > 1.0) {
						throw util::file::DataFileException {L"Slow multi-chance must be between 0 and 1 inclusive."s,
							my_parser->getLine()};
					}
					auto my_shot = std::make_shared<SlowShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, slow_factor, slow_duration, slow_mchance);
					auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::Stun:
				{
					my_parser->readKeyValue(L"stun_chance"s);
					double stun_chance = my_parser->parseNumber();
					if (stun_chance <= 0.0 || stun_chance > 1.0) {
						throw util::file::DataFileException {L"Stun chance should be positive and less than 1.0."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"stun_duration"s);
					int stun_duration = static_cast<int>(my_parser->parseNumber());
					if (stun_duration < 10) {
						throw util::file::DataFileException {L"Stun duration must be >= 10ms."s,
							my_parser->getLine()};
					}
					my_parser->readKeyValue(L"stun_multi_chance"s);
					double stun_mchance = my_parser->parseNumber();
					if (stun_mchance < 0.0) {
						throw util::file::DataFileException {L"Stun multi-chance must be non-negative."s,
							my_parser->getLine()};
					}
					else if (stun_mchance > stun_chance) {
						throw util::file::DataFileException {L"Stun multi-chance must not exceed stun chance."s,
							my_parser->getLine()};
					}
					auto my_shot = std::make_shared<StunShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, stun_chance, stun_duration, stun_mchance);
					auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				} // End Switch
				if (!insert_succeeded) {
					throw util::file::DataFileException {L"Duplicate shot name found: "s + n + L"."s, my_parser->getLine()};
				}
			}
		}

		void MyGame::init_tower_types() {
			std::wifstream data_file {L"./resources/towers.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/towers.ini for reading."s, 0};
			}
			auto my_parser {std::make_unique<util::file::DataFileParser>(data_file)};
			// Global section
			my_parser->expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser->readKeyValue(L"version"s);
			my_parser->expectToken(util::file::TokenTypes::Number, L"1"s);
			// Firing section
			my_parser->getNext();
			my_parser->expectToken(util::file::TokenTypes::Section, L"firing"s);
			my_parser->readKeyValue(L"firing_methods"s);
			my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
			std::map<std::wstring, std::shared_ptr<FiringMethod>> my_firing_methods {};
			my_parser->getNext();
			while (my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
				my_parser->readKeyValue(L"name"s);
				std::wstring fm_name = my_parser->parseString();
				my_parser->readKeyValue(L"method"s);
				if (my_parser->getToken() == L"Default"s) {
					auto my_fmethod = std::make_shared<FiringMethod>(FiringMethodTypes::Default);
					my_firing_methods.emplace(fm_name, std::move(my_fmethod));
				}
				else {
					std::wstring fmethod_type_str = my_parser->getToken();
					my_parser->readKeyValue(L"angles"s);
					auto fm_angle_strs = my_parser->readList();
					std::vector<double> fm_angles {};
					for (auto& a : fm_angle_strs) {
						fm_angles.emplace_back(std::stod(a) * math::pi / 180.0);
					}
					std::sort(fm_angles.begin(), fm_angles.end());
					// So weird... Why can't we just do the duplicate removal in-place?
					// (With any luck, this will never be an issue... There is unique(),
					// but I apparently have to make use of the returned iterator which is
					// not favorable either.)
					auto fm_angles2 = fm_angles;
					std::unique_copy(fm_angles.begin(), fm_angles.end(), std::back_inserter(fm_angles2));
					fm_angles = fm_angles2;

					if (fmethod_type_str == L"Static"s) {
						auto my_fmethod = std::make_shared<FiringMethod>(FiringMethodTypes::Static, fm_angles);
						my_firing_methods.emplace(fm_name, std::move(my_fmethod));
					}
					else {
						my_parser->readKeyValue(L"duration"s);
						int fm_duration = static_cast<int>(my_parser->parseNumber());
						auto my_fmethod = std::make_shared<FiringMethod>(FiringMethodTypes::Pulse, fm_angles, fm_duration);
						my_firing_methods.emplace(fm_name, std::move(my_fmethod));
					}
				}
				my_parser->getNext();
				my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
				my_parser->getNext();
			}
			my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
			// Targeting section
			my_parser->getNext();
			my_parser->expectToken(util::file::TokenTypes::Section, L"targeting"s);
			my_parser->readKeyValue(L"targeting_methods"s);
			my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
			std::map<std::wstring, std::shared_ptr<TargetingStrategy>> my_targeting_strategies {};
			my_parser->getNext();
			while (my_parser->matchToken(util::file::TokenTypes::Object, L"{"s)) {
				my_parser->readKeyValue(L"name"s);
				std::wstring ts_name = my_parser->parseString();
				my_parser->readKeyValue(L"strategy"s);
				std::wstring ts_strategy_str = my_parser->getToken();
				TargetingStrategyTypes ts_strategy = ts_strategy_str == L"Distances"s ? TargetingStrategyTypes::Distances :
					ts_strategy_str == L"Statistics"s ? TargetingStrategyTypes::Statistics :
					ts_strategy_str == L"Names"s ? TargetingStrategyTypes::Names :
					throw util::file::DataFileException {L"Expected a valid targeting strategy: "s
						L" Distances, Statistics, or Names."s, my_parser->getLine()};
				my_parser->readKeyValue(L"protocol"s);
				std::wstring ts_protocol_str = my_parser->getToken();
				TargetingStrategyProtocols ts_protocol = ts_protocol_str == L"Lowest"s ? TargetingStrategyProtocols::Lowest :
					ts_protocol_str == L"Highest"s ? TargetingStrategyProtocols::Highest :
					throw util::file::DataFileException {L"Expected a valid protocol: Lowest or Highest."s,
						my_parser->getLine()};
				switch (ts_strategy) {
				case TargetingStrategyTypes::Distances:
				{
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_strategy, ts_protocol);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				case TargetingStrategyTypes::Statistics:
				{
					my_parser->readKeyValue(L"statistic"s);
					std::wstring ts_stat_str = my_parser->getToken();
					TargetingStrategyStatistics ts_stat = ts_stat_str == L"Damage"s ? TargetingStrategyStatistics::Damage :
						ts_stat_str == L"Health"s ? TargetingStrategyStatistics::Health :
						ts_stat_str == L"Armor_Health"s ? TargetingStrategyStatistics::Armor_Health :
						ts_stat_str == L"Armor_Reduce"s ? TargetingStrategyStatistics::Armor_Reduce :
						ts_stat_str == L"Speed"s ? TargetingStrategyStatistics::Speed :
						ts_stat_str == L"Buffs"s ? TargetingStrategyStatistics::Buffs :
						throw util::file::DataFileException {L"Expected a valid test statistic:"s
						L" Damage, Health, Armor_Health, Armor_Reduce, Speed, Buffs."s, my_parser->getLine()};
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_strategy, ts_protocol, ts_stat);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				case TargetingStrategyTypes::Names:
				{
					// Note: I probably should verify the given names, but I'm not...
					my_parser->readKeyValue(L"target_names");
					auto ts_target_names = my_parser->readList();
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_strategy, ts_protocol, ts_target_names);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				} // End switch
				my_parser->getNext();
				my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
				my_parser->getNext();
			}
			my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
			// Wall section
			my_parser->getNext();
			my_parser->expectToken(util::file::TokenTypes::Section, L"wall"s);
			my_parser->readKeyValue(L"name"s);
			std::wstring wall_name = my_parser->parseString();
			my_parser->readKeyValue(L"desc"s);
			std::wstring wall_desc = my_parser->parseString();
			graphics::Color wall_color = my_parser->readColor();
			graphics::shapes::ShapeTypes wall_shape = my_parser->readShape();
			my_parser->readKeyValue(L"cost"s);
			int wall_cost = static_cast<int>(my_parser->parseNumber());
			if (wall_cost <= 0) {
				throw util::file::DataFileException {L"The cost to build walls should be positive."s, my_parser->getLine()};
			}
			auto my_wall = std::make_shared<WallType>(wall_name, wall_desc, wall_color, wall_shape, wall_cost);
			this->tower_types.emplace_back(std::move(my_wall));
			// Trap section(s)
			my_parser->getNext();
			while (my_parser->matchToken(util::file::TokenTypes::Section, L"trap"s)) {
				my_parser->getNext();
				// For now, this is ignored...
				// (This allows me to add new stuff to the game without breaking the
				// file format.)
				while (!my_parser->matchTokenType(util::file::TokenTypes::Section)) {
					my_parser->getNext();
				}
			}
			do {
				// Tower section(s)
				my_parser->expectToken(util::file::TokenTypes::Section, L"tower"s);
				my_parser->readKeyValue(L"name"s);
				std::wstring n = my_parser->parseString();
				my_parser->readKeyValue(L"desc"s);
				std::wstring d = my_parser->parseString();
				graphics::Color c = my_parser->readColor();
				graphics::shapes::ShapeTypes st = my_parser->readShape();
				std::shared_ptr<FiringMethod> fmethod {nullptr};
				try {
					my_parser->readKeyValue(L"firing_method"s);
					fmethod = my_firing_methods.at(my_parser->parseString());
				}
				catch (const std::out_of_range&) {
					throw util::file::DataFileException {L"Unknown firing method name: "s + my_parser->parseString(),
						my_parser->getLine()};
				}
				std::shared_ptr<TargetingStrategy> tstrategy {nullptr};
				try {
					my_parser->readKeyValue(L"targeting_strategy"s);
					tstrategy = my_targeting_strategies.at(my_parser->parseString());
				}
				catch (const std::out_of_range&) {
					throw util::file::DataFileException {L"Unknown targeting strategy name: "s + my_parser->parseString(),
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"shots");
				my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
				std::vector<std::pair<std::shared_ptr<ShotBaseType>, double>> my_tower_shots {};
				my_parser->getNext();
				my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
				double freq_total = 0.0;
				do {
					std::shared_ptr<ShotBaseType> my_tower_shot_type {nullptr};
					try {
						my_parser->readKeyValue(L"name"s);
						my_tower_shot_type = this->getShotType(my_parser->parseString());
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"No projectile exists with the following name: "s
							+ my_parser->parseString() + L"."s, my_parser->getLine()};
					}
					my_parser->readKeyValue(L"frequency"s);
					double my_tower_shot_freq = my_parser->parseNumber();
					freq_total += my_tower_shot_freq;
					if (my_tower_shot_freq <= 0.0) {
						throw util::file::DataFileException {L"Shot frequency must be positive."s, my_parser->getLine()};
					}
					else if (freq_total > 1.02) {
						// (1.02 was chosen due to precision shenanigans)
						throw util::file::DataFileException {L"The frequencies for all shots must not exceed 1.0."s,
							my_parser->getLine()};
					}
					my_tower_shots.emplace_back(std::move(my_tower_shot_type), my_tower_shot_freq);
					my_parser->getNext();
					my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
					my_parser->getNext();
				} while (my_parser->matchToken(util::file::TokenTypes::Object, L"{"s));
				my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
				if (freq_total < 1.00 || freq_total > 1.02) {
					throw util::file::DataFileException {L"The frequencies for all shots combined must not exceed 1.0."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"firing_speed"s);
				double fs = my_parser->parseNumber();
				if (fs < 0.1 || fs > 90.0) {
					throw util::file::DataFileException {L"Firing speed must be between 0.1 and 90.0 inclusive."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"firing_range"s);
				double fr = my_parser->parseNumber();
				if (fr < 0.5) {
					throw util::file::DataFileException {L"Firing range must be at least 0.5."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"volley_shots"s);
				int vs = static_cast<int>(my_parser->parseNumber());
				if (vs < 0) {
					throw util::file::DataFileException {L"Volley shots must be non-negative."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"reload_delay"s);
				int rd = static_cast<int>(my_parser->parseNumber());
				if (vs == 0 && rd != 0) {
					throw util::file::DataFileException {L"Reload delay must be zero if volley shots is zero."s,
						my_parser->getLine()};
				}
				else if (vs > 0 && rd < 10) {
					throw util::file::DataFileException {L"Reload delay must be at least 10ms if volley shots is positive."s,
						my_parser->getLine()};
				}
				my_parser->readKeyValue(L"cost_adjust"s);
				int cost_adj = static_cast<int>(my_parser->parseNumber());
				auto my_tower_type = std::make_shared<TowerType>(n, d, c, st, fmethod, tstrategy,
					std::move(my_tower_shots), fs, fr, vs, rd, cost_adj);
				this->tower_types.emplace_back(std::move(my_tower_type));
			} while (my_parser->getNext());
		}

		void MyGame::load_global_level_data() {
			std::wifstream data_file {L"./resources/levels/global.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/levels/global.ini for reading."s, 0};
			}
			auto my_parser = util::file::DataFileParser {data_file};
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"backup_level_if_load_fails"s);
			this->my_level_backup_number = static_cast<int>(my_parser.parseNumber());
			if (this->my_level_backup_number < 1) {
				this->my_level_backup_number = 1;
			}
		}

		void MyGame::load_level_data() {
			std::wifstream data_file {L"./resources/levels/level"s + std::to_wstring(this->level) + L".ini"s};
			if (data_file.bad() || data_file.fail()) {
				data_file.open(L"./resources/levels/level"s + std::to_wstring(this->my_level_backup_number) + L".ini"s);
				if (data_file.bad() || data_file.fail()) {
					throw util::file::DataFileException {L"Could not open ./resources/levels/level"s
						+ std::to_wstring(this->level) + L".ini for reading!"s, 0};
				}
			}
			auto my_parser = std::make_unique<util::file::DataFileParser>(data_file);
			// Global section
			my_parser->expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser->readKeyValue(L"version"s);
			my_parser->expectToken(util::file::TokenTypes::Number, L"1"s);
			my_parser->readKeyValue(L"wave_spawn_delay"s);
			int wave_spawn_delay = static_cast<int>(my_parser->parseNumber());
			if (wave_spawn_delay < 200 || wave_spawn_delay > 20000) {
				throw util::file::DataFileException {L"Wave spawn delay should be between 200ms and 20000ms inclusive."s,
					my_parser->getLine()};
			}
			my_parser->getNext();
			std::deque<std::unique_ptr<EnemyWave>> my_level_waves {};
			do {
				// [wave] sections
				my_parser->expectToken(util::file::TokenTypes::Section, L"wave"s);
				my_parser->readKeyValue(L"group_spawn_delay"s);
				int group_spawn_delay = static_cast<int>(my_parser->parseNumber());
				if (group_spawn_delay < 100 || group_spawn_delay > 7500) {
					throw util::file::DataFileException {L"Group spawn delay should be between 100ms and 7500ms inclusive."s,
						my_parser->getLine()};
				}
				std::deque<std::unique_ptr<EnemyGroup>> my_wave_groups {};
				my_parser->readKeyValue(L"groups"s);
				my_parser->expectToken(util::file::TokenTypes::Object, L"{"s);
				my_parser->getNext();
				do {
					my_parser->readKeyValue(L"enemy_name"s);
					std::wstring enemy_name = my_parser->parseString();
					if (this->enemy_types.find(enemy_name) == this->enemy_types.end()) {
						throw util::file::DataFileException {L"Enemy type not found: "s + enemy_name + L"."s,
							my_parser->getLine()};
					}
					std::shared_ptr<EnemyType> etype = this->getEnemyType(enemy_name);
					my_parser->readKeyValue(L"extra_count"s);
					int enemy_count = static_cast<int>(my_parser->parseNumber()) + this->challenge_level + 2;
					if (enemy_count - this->challenge_level - 2 < 0) {
						throw util::file::DataFileException {L"Extra count should be non-negative."s, my_parser->getLine()};
					}
					my_parser->readKeyValue(L"enemy_spawn_delay"s);
					int enemy_spawn_delay = static_cast<int>(my_parser->parseNumber());
					if (enemy_spawn_delay < 10 || enemy_spawn_delay > 5000) {
						throw util::file::DataFileException {L"Enemy spawn delay should be between 10ms and 5000ms inclusive."s,
							my_parser->getLine()};
					}
					std::queue<std::unique_ptr<Enemy>> my_enemy_spawns {};
					if (etype->isUnique()) {
						enemy_count -= this->getChallengeLevel() + 1;
					}
					pathfinding::Pathfinder my_pathfinder {this->getMap(), etype->isFlying(), etype->canMoveDiagonally(),
						etype->getDefaultStrategy()};
					my_pathfinder.findPath(this->challenge_level);
					for (int i = 0; i < enemy_count; ++i) {
						auto my_enemy = std::make_unique<Enemy>(this->getDeviceResources(), etype,
							graphics::Color {0.f, 0.f, 0.f, 1.f}, my_pathfinder,
							this->getMap().getTerrainGraph(etype->isFlying()).getStartNode()->getGameX() + 0.5,
							this->getMap().getTerrainGraph(etype->isFlying()).getStartNode()->getGameY() + 0.5,
							this->level, this->difficulty, this->challenge_level);
						my_enemy_spawns.emplace(std::move(my_enemy));
					}
					auto my_group = std::make_unique<EnemyGroup>(std::move(my_enemy_spawns), enemy_spawn_delay);
					my_wave_groups.emplace_front(std::move(my_group));
					my_parser->getNext();
					my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
					my_parser->getNext();
				} while (my_parser->matchToken(util::file::TokenTypes::Object, L"{"s));
				my_parser->expectToken(util::file::TokenTypes::Object, L"}"s);
				auto my_wave = std::make_unique<EnemyWave>(std::move(my_wave_groups), group_spawn_delay);
				my_level_waves.emplace_front(std::move(my_wave));
			} while (my_parser->getNext());
			this->my_level = std::make_unique<GameLevel>(this->level, std::move(my_level_waves), wave_spawn_delay);
			this->my_level_enemy_count = this->my_level->getEnemyCount();
		}

		void MyGame::save_game(std::wostream& save_file) const {
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can't save now...
				return;
			}
			save_file << L"V: " << 1 << L"\n";
			save_file << L"C: " << this->challenge_level << L" D: " << this->difficulty
				<< L" L: " << this->level << L"\n";
			save_file << L"H: " << this->player.getHealth() << L" M: " << this->player.getMoney() << L"\n";
			save_file << L"W: " << this->win_streak << L" L: " << this->lose_streak << L"\n";
			// Output terrain map
			save_file << this->getMap().getTerrainGraph(false) << L"\n";
			save_file << this->getMap().getTerrainGraph(true) << L"\n";
			// Output influence map
			save_file << this->getMap().getInfluenceGraph(false) << L"\n";
			save_file << this->getMap().getInfluenceGraph(true) << L"\n";
			// Output towers as well.
			for (const auto& t : this->towers) {
				save_file << L"T: " << t->getBaseType()->getName() << L"\n\tT: " << t->getGameX() << L" "
					<< t->getGameY() << L"\n";
			}
		}

		void MyGame::load_game(std::wistream& save_file) {
			std::wstring buffer {};
			int version;
			save_file >> buffer >> version;
			if (version == 1) {
				save_file >> buffer >> this->challenge_level >> buffer >> this->difficulty
					>> buffer >> this->level;
				int player_health;
				double player_cash;
				save_file >> buffer >> player_health >> buffer >> player_cash;
				this->player = Player {player_cash, player_health};
				save_file >> buffer >> this->win_streak >> buffer >> this->lose_streak;
				// Terrain map
				// (This is tricky --> Due to the way the start and end nodes are loaded,
				// I need to TRANSFER the ownership of these resources TO this->map.)
				auto my_gterrain = std::make_unique<pathfinding::Grid>();
				auto my_aterrain = std::make_unique<pathfinding::Grid>();
				save_file >> *my_gterrain >> *my_aterrain;
				this->map = std::make_shared<game::GameMap>(std::move(my_gterrain), std::move(my_aterrain));
				// Influence map
				pathfinding::Grid ground_influence_map {};
				pathfinding::Grid air_influence_map {};
				save_file >> ground_influence_map >> air_influence_map;
				this->map->setInfluenceGraphs(ground_influence_map, air_influence_map);
				// Towers
				std::wstring tower_name;
				double tower_gx;
				double tower_gy;
				while (save_file >> buffer) {
					std::getline(save_file, tower_name);
					// Leading space is removed...
					tower_name.erase(tower_name.begin());
					save_file >> buffer >> tower_gx >> tower_gy;
					std::shared_ptr<TowerType> my_type {nullptr};
					for (const auto& tt : this->getAllTowerTypes()) {
						if (tt->getName() == tower_name) {
							my_type = tt;
							break;
						}
					}
					auto my_tower = std::make_unique<Tower>(this->getDeviceResources(), my_type,
						graphics::Color {0.f, 0.f, 0.f, 1.f}, tower_gx, tower_gy);
					this->addTower(std::move(my_tower));
					auto my_floored_gx = static_cast<int>(std::floor(tower_gx));
					auto my_floored_gy = static_cast<int>(std::floor(tower_gy));
					this->getMap().getFiterGraph(false).getNode(my_floored_gx, my_floored_gy).setBlockage(true);
					this->getMap().getFiterGraph(true).getNode(my_floored_gx, my_floored_gy).setBlockage(true);
				}
			}
			else {
				throw util::file::DataFileException {L"Unrecognized save file version."s, 1};
			}
		}
	}
}