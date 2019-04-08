// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include "./../targetver.hpp"
#include <shlobj.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <map>
#include <vector>
#include <set>
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

		void MyGame::load_config_data() {
			constexpr const auto config_file_name = L"./config/config.ini";
			[[maybe_unused]] std::wifstream config_file {config_file_name};
			if (config_file.fail() || config_file.bad()) {
				// Fail silently.
				return;
			}
#if defined(_WIN32) || defined(_WIN64)
			util::file::DataFileParser my_parser {config_file};
			// Globals section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"version");
			my_parser.expectToken(util::file::TokenTypes::Number, L"1");
			my_parser.readKeyValue(L"use_appdata_folder");
			const bool use_appdata_folder = my_parser.parseBoolean();
			if (use_appdata_folder) {
				wchar_t* appdata_folder_path = nullptr;
				if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appdata_folder_path))) {
					CreateDirectory((appdata_folder_path + L"/Isaiah Hoffman/"s).c_str(), nullptr);
					std::wstring my_path = appdata_folder_path + L"/Isaiah Hoffman/tower_defense/"s;
					CoTaskMemFree(appdata_folder_path);
					CreateDirectory(my_path.c_str(), nullptr);
					this->resources_folder_path = my_path + L"resources/"s;
					CreateDirectory(this->resources_folder_path.c_str(), nullptr);
					CreateDirectory((this->resources_folder_path + L"levels/").c_str(), nullptr);
					CreateDirectory((this->resources_folder_path + L"graphs/").c_str(), nullptr);
					this->userdata_folder_path = my_path + L"userdata/"s;
					CreateDirectory(this->userdata_folder_path.c_str(), nullptr);
					my_parser.readKeyValue(L"do_copy");
					if (my_parser.parseBoolean()) {
						// Copy needed files.
						// This is really lazy code, but no real harm done.
						for (int i = 1; i < 9999; ++i) {
							const std::wstring level_str = L"levels/level" + std::to_wstring(i) + L".ini";
							CopyFile((L"./resources/" + level_str).c_str(), (this->resources_folder_path + level_str).c_str(), FALSE);
						}
						const std::wstring my_resources[] = {
							L"enemies.ini", L"enemies.ini.format", L"shots.ini", L"shots.ini.format", L"towers.ini",
							L"towers.ini.format", L"tower_upgrades.ini", L"tower_upgrades.ini.format", L"other.ini",
							L"levels/global.ini", L"levels/level0.ini.format", L"levels/levels.xlsx",
							L"graphs/air_graph_beginner.txt", L"graphs/air_graph_intermediate.txt",
							L"graphs/air_graph_experienced.txt", L"graphs/air_graph_expert.txt",
							L"graphs/ground_graph_beginner.txt", L"graphs/ground_graph_intermediate.txt",
							L"graphs/ground_graph_experienced.txt", L"graphs/ground_graph_expert.txt"
						};
						for (const auto res_str : my_resources) {
							CopyFile((L"./resources/" + res_str).c_str(), (this->resources_folder_path + res_str).c_str(), FALSE);
						}
						config_file.close();
						std::wofstream my_config_writer {config_file_name, std::ios_base::out | std::ios_base::trunc};
						if (my_config_writer.fail() || my_config_writer.bad()) {
							// Fail silently.
							return;
						}
						my_config_writer << L"# Stores information about where other resources are located.\n"
							<< L"[global]\n" << L"version = 1\n"
							<< L"# Whether or not to resolve file paths to the execution directory\n"
							<< L"# or to the Windows-specific application data folder.\n"
							<< L"use_appdata_folder = true\n"
							<< L"# Whether or not to copy the files in program files to the application\n"
							<< L"# data folder.\n" << L"do_copy = false\n";
					}
				}
			}
#endif // _WIN32 || _WIN64
		}

		void MyGame::load_global_misc_data() {
			std::wifstream data_file {this->resources_folder_path + L"other.ini"s};
			if (data_file.fail() || data_file.bad()) {
				throw util::file::DataFileException {L"Could not load the enemy data file (enemies.ini)."s, 0};
			}
			util::file::DataFileParser my_parser {data_file};
			// Globals section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.getNext();
			// Health buying section.
			my_parser.expectToken(util::file::TokenTypes::Section, L"health_buying"s);
			my_parser.readKeyValue(L"amount_gained");
			this->hp_gained_per_buy = static_cast<int>(my_parser.parseNumber());
			util::file::DataFileParser::validateNumberMinBound(this->hp_gained_per_buy, 0, L"Amount gained", my_parser.getLine(), true);
			my_parser.readKeyValue(L"initial_cost");
			this->hp_buy_cost = my_parser.parseNumber();
			util::file::DataFileParser::validateNumberMinBound(this->hp_buy_cost, 0., L"Initial cost", my_parser.getLine(), false);
			my_parser.readKeyValue(L"cost_multiplier");
			this->hp_buy_multiplier = my_parser.parseNumber();
			util::file::DataFileParser::validateNumberMinBound(this->hp_buy_multiplier, 1., L"Cost multiplier", my_parser.getLine(), true);
		}

#pragma warning(push)
#pragma warning(disable: 4996) // I presume this is deprecated warning? Too bad I didn't document this originally.
		void MyGame::init_enemy_types() {
			std::wifstream data_file {this->resources_folder_path + L"enemies.ini"s};
			if (data_file.fail() || data_file.bad()) {
				throw util::file::DataFileException {L"Could not load the enemy data file (enemies.ini)."s, 0};
			}
			// (I might eventually rewrite this so that all the legacy code is gone...)
			// (But this will do for the time being.)
			util::file::DataFileParser my_parser {data_file};
			// Globals section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			auto my_token = my_parser.readKeyValue(L"version"s);
			my_parser.expectToken(util::file::TokenTypes::Number, L"1"s);
			// Buff Targets section
			my_parser.getNext();
			my_parser.expectToken(util::file::TokenTypes::Section, L"buff_targets"s);
			my_parser.readKeyValue(L"targets"s);
			my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
			// Key => Group Name, Value => List of enemy names associated with that group name
			std::map<std::wstring, std::vector<std::wstring>> buff_target_groups {};
			while (true) {
				my_parser.getNext();
				if (my_parser.matchToken(util::file::TokenTypes::Object, L"}"s)) {
					my_parser.getNext();
					break;
				}
				else if (!my_parser.matchTokenType(util::file::TokenTypes::Object)) {
					throw util::file::DataFileException {L"Expected either an opening or closing brace."s,
						my_parser.getLine()};
				}
				// (Yes, this means the quotes are optional in some cases...)
				const std::wstring group_name = my_parser.readKeyValue(L"group_name"s).second;
				my_token = my_parser.readKeyValue(L"target_names"s);
				if (!util::file::matchTokenType(util::file::TokenTypes::List, my_token.first)) {
					throw util::file::DataFileException {L"Expected a list of names following the equal signs."s
						L" (Start the list with < and end it with >.)"s, my_parser.getLine()};
				}
				const std::vector<std::wstring> target_names = my_parser.readList();
				buff_target_groups.emplace(group_name, target_names);
				my_parser.getNext();
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
			}
			do {
				// Enemy Sections
				my_parser.expectToken(util::file::TokenTypes::Section, L"enemy"s);
				my_token = my_parser.readKeyValue(L"name"s);
				const std::wstring n = util::file::parseString(my_token, my_parser.getLine());
				my_token = my_parser.readKeyValue(L"desc"s);
				const std::wstring d = util::file::parseString(my_token, my_parser.getLine());
				const graphics::Color c = my_parser.readColor();
				const graphics::shapes::ShapeTypes st = my_parser.readShape();
				my_token = my_parser.readKeyValue(L"damage"s);
				const int dmg = static_cast<int>(util::file::parseNumber(my_token, my_parser.getLine()));
				util::file::DataFileParser::validateNumberMinBound(dmg, 1, L"Damage", my_parser.getLine(), true);
				my_token = my_parser.readKeyValue(L"health"s);
				const double hp = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumberMinBound(hp, 0.0, L"Health", my_parser.getLine(), false);
				my_token = my_parser.readKeyValue(L"armor_health"s);
				const double ahp = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumberMinBound(ahp, 0.0, L"Armor health", my_parser.getLine(), true);
				my_token = my_parser.readKeyValue(L"armor_reduce"s);
				// Allowing ar == 1.0 is not good design.
				const double ar = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumber(ar, 0.0, 1.0, L"Armor reduction", my_parser.getLine(), true, false);
				my_token = my_parser.readKeyValue(L"pain_tolerance"s);
				const double pt = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumber(pt, 0.0, 1.0, L"Pain tolerance", my_parser.getLine(), true, true);
				my_token = my_parser.readKeyValue(L"walking_speed"s);
				const double wspd = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumber(wspd, 0.500, 25.000, L"Walking speed", my_parser.getLine(), true, true);
				// Not enforcing ispd <= wspd <= rspd because of interesting effects
				// when that rule is not followed.
				my_token = my_parser.readKeyValue(L"running_speed"s);
				const double rspd = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumber(rspd, 0.500, 25.000, L"Running speed", my_parser.getLine(), true, true);
				my_token = my_parser.readKeyValue(L"injured_speed"s);
				const double ispd = util::file::parseNumber(my_token, my_parser.getLine());
				util::file::DataFileParser::validateNumber(ispd, 0.500, 25.000, L"Injured speed", my_parser.getLine(), true, true);
				my_token = my_parser.readKeyValue(L"strategy"s);
				const pathfinding::HeuristicStrategies strat =
					my_token.second == L"Manhattan"s ? pathfinding::HeuristicStrategies::Manhattan
					: my_token.second == L"Euclidean"s ? pathfinding::HeuristicStrategies::Euclidean
					: my_token.second == L"Diagonal"s ? pathfinding::HeuristicStrategies::Diagonal
					: my_token.second == L"Maximum"s ? pathfinding::HeuristicStrategies::Max_Dx_Dy
					: throw util::file::DataFileException {L"Invalid strategy constant specified."s,
						my_parser.getLine()};
				my_parser.readKeyValue(L"can_move_diagonally"s);
				const bool move_diag = my_parser.parseBoolean();
				my_parser.readKeyValue(L"is_flying"s);
				const bool fly = my_parser.parseBoolean();
				my_parser.readKeyValue(L"is_unique"s);
				const bool unique = my_parser.parseBoolean();
				my_parser.readKeyValue(L"buffs"s);
				if (!my_parser.matchToken(util::file::TokenTypes::Object, L"{"s)) {
					throw util::file::DataFileException {L"Expected the start of an object definition."s,
						my_parser.getLine()};
				}
				my_parser.getNext();
				bool insert_succeeded = false;
				std::set<std::wstring> enemy_names {};
				// Case when we have {} next to buffs
				if (my_parser.matchToken(util::file::TokenTypes::Object, L"}"s)) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly, unique);
					this->enemy_types.emplace_back(std::move(my_type));
					if (enemy_names.find(n) == enemy_names.end()) {
						enemy_names.emplace(n);
						insert_succeeded = true;
					}
				}
				std::vector<std::shared_ptr<BuffBase>> my_buffs {};
				while (!my_parser.matchToken(util::file::TokenTypes::Object, L"}"s)) {
					if (!my_parser.matchToken(util::file::TokenTypes::Object, L"{"s)) {
						throw util::file::DataFileException {L"Expected opening brace ({)."s, my_parser.getLine()};
					}
					my_token = my_parser.readKeyValue(L"type"s);
					const BuffTypes buff_type = my_token.second == L"Intelligence"s ? BuffTypes::Intelligence
						: my_token.second == L"Speed"s ? BuffTypes::Speed
						: my_token.second == L"Healer"s ? BuffTypes::Healer
						: my_token.second == L"Purify"s ? BuffTypes::Purify
						: my_token.second == L"Repair"s ? BuffTypes::Repair
						: my_token.second == L"Forcefield"s ? BuffTypes::Forcefield
						: throw util::file::DataFileException {L"Expected the type of the buff."s
							L" Valid values include: Intelligence, Speed, Healer, Purify, Repair"s, my_parser.getLine()};
					my_token = my_parser.readKeyValue(L"targets"s);
					const std::wstring buff_group = util::file::parseString(my_token, my_parser.getLine());
					my_token = my_parser.readKeyValue(L"radius"s);
					try {
						// Gotta find some way to tell the compiler NOT
						// to optimize away this statement.
						(void)buff_target_groups.at(buff_group);
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"Invalid target group name specified."s,
							my_parser.getLine()};
					}
					const double buff_radius = util::file::parseNumber(my_token, my_parser.getLine());
					util::file::DataFileParser::validateNumberMinBound(buff_radius, 0.0, L"Buff radius", my_parser.getLine(), false);
					my_token = my_parser.readKeyValue(L"delay"s);
					const int buff_delay = static_cast<int>(util::file::parseNumber(my_token, my_parser.getLine()));
					util::file::DataFileParser::validateNumber(buff_delay, 10, 60'000, L"Buff delay (ms)", my_parser.getLine(), true, true);
					[[maybe_unused]] int buff_duration {};
					// Duration is common to many buffs, so I don't switch on it.
					if (buff_type == BuffTypes::Intelligence || buff_type == BuffTypes::Speed
						|| buff_type == BuffTypes::Forcefield) {
						my_token = my_parser.readKeyValue(L"duration"s);
						buff_duration = static_cast<int>(util::file::parseNumber(my_token, my_parser.getLine()));
						util::file::DataFileParser::validateNumberMinBound(buff_duration, 10, L"Buff duration (ms)", my_parser.getLine(), true);
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
						my_token = my_parser.readKeyValue(L"walking_speed_boost"s);
						const double buff_wspd = util::file::parseNumber(my_token, my_parser.getLine());
						util::file::DataFileParser::validateNumberMinBound(buff_wspd, 0.0, L"Walking speed boost", my_parser.getLine(), true);
						my_token = my_parser.readKeyValue(L"running_speed_boost"s);
						const double buff_rspd = util::file::parseNumber(my_token, my_parser.getLine());
						util::file::DataFileParser::validateNumberMinBound(buff_rspd, 0.0, L"Running speed boost", my_parser.getLine(), true);
						my_token = my_parser.readKeyValue(L"injured_speed_boost"s);
						const double buff_ispd = util::file::parseNumber(my_token, my_parser.getLine());
						util::file::DataFileParser::validateNumberMinBound(buff_ispd, 0.0, L"Injured speed boost", my_parser.getLine(), true);
						if (buff_ispd == 0.0 && buff_rspd == 0. && buff_wspd == 0.) {
							throw util::file::DataFileException {L"You should set at least one of the speed"s
								L" boosts to a positive value!"s, my_parser.getLine()};
						}
						auto speed_buff = std::make_shared<SpeedBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_duration, buff_wspd, buff_rspd, buff_ispd);
						my_buffs.emplace_back(std::move(speed_buff));
						break;
					}
					case BuffTypes::Healer:
					{
						my_token = my_parser.readKeyValue(L"heal_amount"s);
						const double buff_heal = util::file::parseNumber(my_token, my_parser.getLine());
						util::file::DataFileParser::validateNumberMinBound(buff_heal, 0.0, L"Heal amount", my_parser.getLine(), false);
						auto heal_buff = std::make_shared<HealerBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_heal);
						my_buffs.emplace_back(std::move(heal_buff));
						break;
					}
					case BuffTypes::Purify:
					{
						my_token = my_parser.readKeyValue(L"purify_max_effects"s);
						const int buff_cure_max = static_cast<int>(util::file::parseNumber(my_token,
							my_parser.getLine()));
						util::file::DataFileParser::validateNumberMinBound(buff_cure_max, 1, L"Purify max effects", my_parser.getLine(), true);
						auto purify_buff = std::make_shared<PurifyBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_cure_max);
						my_buffs.emplace_back(std::move(purify_buff));
						break;
					}
					case BuffTypes::Repair:
					{
						my_token = my_parser.readKeyValue(L"repair_amount"s);
						const double buff_repair = util::file::parseNumber(my_token, my_parser.getLine());
						util::file::DataFileParser::validateNumberMinBound(buff_repair, 0.0, L"Repair amount", my_parser.getLine(), false);
						auto repair_buff = std::make_shared<HealerBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_repair);
						my_buffs.emplace_back(std::move(repair_buff));
						break;
					}
					case BuffTypes::Forcefield:
					{
						my_parser.readKeyValue(L"shield_health");
						const double buff_shield_hp = my_parser.parseNumber();
						util::file::DataFileParser::validateNumberMinBound(buff_shield_hp, 0.0, L"Shield health", my_parser.getLine(), false);
						my_parser.readKeyValue(L"shield_absorb");
						const double buff_shield_absorb = my_parser.parseNumber();
						util::file::DataFileParser::validateNumberMinBound(buff_shield_absorb, 0.0, L"Shield absorb", my_parser.getLine(), false);
						auto shield_buff = std::make_shared<ForcefieldBuff>(buff_target_groups.at(buff_group),
							buff_radius, buff_delay, buff_duration, buff_shield_hp, buff_shield_absorb);
						my_buffs.emplace_back(std::move(shield_buff));
						break;
					}
					} // End switch, this looks weird but is correct
					my_parser.getNext();
					if (!my_parser.matchToken(util::file::TokenTypes::Object, L"}"s)) {
						throw util::file::DataFileException {L"Expected closing brace (})."s,
							my_parser.getLine()};
					}
					my_parser.getNext();
					if (!my_parser.matchTokenType(util::file::TokenTypes::Object)) {
						throw util::file::DataFileException {L"Expected an opening or closing brace."s,
							my_parser.getLine()};
					}
				}
				if (my_buffs.size() > 0) {
					auto my_type = std::make_unique<EnemyType>(n, d, c, st, dmg, hp, ahp, ar, pt,
						wspd, rspd, ispd, strat, move_diag, fly, unique, std::move(my_buffs));
					if (enemy_names.find(n) == enemy_names.end()) {
						this->enemy_types.emplace_back(std::move(my_type));
						insert_succeeded = true;
					}
				}
				if (!insert_succeeded) {
					throw util::file::DataFileException {L"Duplicate enemy name: "s + n + L"."s, my_parser.getLine()};
				}
			} while (my_parser.getNext());
			// Add listing for "seen before".
			for (const auto& etype : this->enemy_types) {
				this->enemies_seen.emplace(etype->getName(), false);
				this->enemy_kill_count.emplace(etype->getName(), 0);
			}
#pragma warning(pop)
		}

		void MyGame::init_shot_types() {
			std::wifstream data_file {this->resources_folder_path + L"shots.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/shots.ini for reading."s, 0};
			}
			util::file::DataFileParser my_parser {data_file};
			// Global section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
#pragma warning(push)
#pragma warning(disable: 26444) // ES84: Avoid unnamed objects with custom construction/destruction. (No Idea...)
			my_parser.readKeyValue(L"version");
			my_parser.expectToken(util::file::TokenTypes::Number, L"1"s);
			while (my_parser.getNext()) {
				// Shot sections
				my_parser.expectToken(util::file::TokenTypes::Section, L"shot"s);
				my_parser.readKeyValue(L"name"s);
				const std::wstring n = my_parser.parseString();
				my_parser.readKeyValue(L"desc"s);
				const std::wstring d = my_parser.parseString();
				const graphics::Color c = my_parser.readColor();
				const graphics::shapes::ShapeTypes st = my_parser.readShape();
				my_parser.readKeyValue(L"damage"s);
				const double dmg = my_parser.parseNumber();
				util::file::DataFileParser::validateNumberMinBound(dmg, 0.0, L"Damage", my_parser.getLine(), true);
				my_parser.readKeyValue(L"piercing"s);
				const double wap = my_parser.parseNumber();
				util::file::DataFileParser::validateNumber(wap, 0.0, 1.0, L"Piercing", my_parser.getLine(), true, true);
				my_parser.readKeyValue(L"move_speed"s);
				const double ms = my_parser.parseNumber();
				util::file::DataFileParser::validateNumber(ms, 10.00, 60.00, L"Movement speed", my_parser.getLine(), true, true);
				my_parser.readKeyValue(L"impact_radius"s);
				const double ir = my_parser.parseNumber();
				util::file::DataFileParser::validateNumberMinBound(ir, 0.0, L"Impact radius", my_parser.getLine(), true);
				my_parser.readKeyValue(L"splash_damage"s);
				const double sdmg = my_parser.parseNumber();
				util::file::DataFileParser::validateNumberMinBound(sdmg, 0.0, L"Splash damage", my_parser.getLine(), true);
				// Don't need to check the other cuz there may be a special effect. (At least not now.)
				if (sdmg > 0.0 && ir <= 0.0) {
					throw util::file::DataFileException {L"Splash damage should be zero if impact"s
						L" radius is zero."s, my_parser.getLine()};
				}
				my_parser.readKeyValue(L"ground_multiplier"s);
				const double gm = my_parser.parseNumber();
				util::file::DataFileParser::validateNumberMinBound(gm, 0.0, L"Ground multiplier", my_parser.getLine(), true);
				my_parser.readKeyValue(L"air_multiplier"s);
				const double am = my_parser.parseNumber();
				util::file::DataFileParser::validateNumberMinBound(am, 0.0, L"Air multiplier", my_parser.getLine(), true);
				if (am <= 0.0 && gm <= 0.0 && (dmg > 0.0 || sdmg > 0.0)) {
					throw util::file::DataFileException {L"Either the ground multiplier or the air multiplier"s
						L" must be positive."s, my_parser.getLine()};
				}
				const std::wstring my_type_str = my_parser.readKeyValue(L"type"s).second;
				const ShotTypes my_type = my_type_str == L"Standard"s ? ShotTypes::Standard :
					my_type_str == L"Damage_Over_Time"s ? ShotTypes::DoT :
					my_type_str == L"Slow"s ? ShotTypes::Slow :
					my_type_str == L"Stun"s ? ShotTypes::Stun :
					throw util::file::DataFileException {L"Invalid type specified."s, my_parser.getLine()};
				if (my_type == ShotTypes::Standard && dmg <= 0.0 && sdmg <= 0.0) {
					throw util::file::DataFileException {L"The shot should deal some kind of damage or"s
						L" have some kind of special effect."s, my_parser.getLine()};
				}
				[[maybe_unused]] bool affect_splash {false};
				if (my_type != ShotTypes::Standard) {
					my_parser.readKeyValue(L"apply_effect_on_splash"s);
					affect_splash = my_parser.parseBoolean();
					if (!affect_splash && ir > 0.0 && sdmg <= 0.0) {
						throw util::file::DataFileException {L"It is pointless to set impact radius to a"s
							L" value greater than zero if splash damage is zero, and there is no special effect"s
							L" applied on splash."s, my_parser.getLine()};
					}
				}
#pragma warning(pop)
				bool insert_succeeded = false;
				switch (my_type) {
				case ShotTypes::Standard:
				{
					auto my_shot = std::make_shared<NormalShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am);
					const auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::DoT:
				{
					const auto dot_type_str = my_parser.readKeyValue(L"dot_damage_type"s).second;
					const DoTDamageTypes dot_type = dot_type_str == L"Poison"s ? DoTDamageTypes::Poison :
						dot_type_str == L"Fire"s ? DoTDamageTypes::Fire :
						dot_type_str == L"Heal"s ? DoTDamageTypes::Heal :
						throw util::file::DataFileException {L"Invalid DoT damage type specified."s,
							my_parser.getLine()};
					my_parser.readKeyValue(L"dot_damage_per_tick"s);
					const double dot_tick_dmg = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(dot_tick_dmg, 0.0, L"DoT damage per tick", my_parser.getLine(), false);
					my_parser.readKeyValue(L"dot_time_between_ticks"s);
					const int dot_tick_time = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumberMinBound(dot_tick_time, 10, L"DoT Time Between Ticks (ms)", my_parser.getLine(), true);
					my_parser.readKeyValue(L"dot_total_ticks"s);
					const int dot_total_ticks = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumberMinBound(dot_total_ticks, 1, L"DoT Total Ticks", my_parser.getLine(), true);
					auto my_shot = std::make_shared<DoTShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, dot_type, dot_tick_dmg, dot_tick_time, dot_total_ticks);
					const auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::Slow:
				{
					my_parser.readKeyValue(L"slow_factor"s);
					const double slow_factor = my_parser.parseNumber();
					util::file::DataFileParser::validateNumber(slow_factor, 0.0, 1.0, L"Slow factor", my_parser.getLine(), false, false);
					my_parser.readKeyValue(L"slow_duration"s);
					const int slow_duration = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumberMinBound(slow_duration, 10, L"Slow duration (ms)", my_parser.getLine(), true);
					my_parser.readKeyValue(L"slow_multi_chance"s);
					const double slow_mchance = my_parser.parseNumber();
					util::file::DataFileParser::validateNumber(slow_mchance, 0.0, 1.0, L"Slow multi-chance", my_parser.getLine(), true, false);
					auto my_shot = std::make_shared<SlowShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, slow_factor, slow_duration, slow_mchance);
					const auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				case ShotTypes::Stun:
				{
					my_parser.readKeyValue(L"stun_chance"s);
					const double stun_chance = my_parser.parseNumber();
					util::file::DataFileParser::validateNumber(stun_chance, 0.0, 1.0, L"Stun chance", my_parser.getLine(), false, false);
					my_parser.readKeyValue(L"stun_duration"s);
					const int stun_duration = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumberMinBound(stun_duration, 10, L"Stun duration (ms)", my_parser.getLine(), true);
					my_parser.readKeyValue(L"stun_multi_chance"s);
					const double stun_mchance = my_parser.parseNumber();
					util::file::DataFileParser::validateNumber(stun_mchance, 0.0, stun_chance, L"Stun multi-chance", my_parser.getLine(), true, true);
					auto my_shot = std::make_shared<StunShotType>(n, d, c, st, dmg, wap, ms, ir, sdmg, gm, am,
						affect_splash, stun_chance, stun_duration, stun_mchance);
					const auto ret = this->shot_types.emplace(n, std::move(my_shot));
					insert_succeeded = ret.second;
					break;
				}
				} // End Switch
				if (!insert_succeeded) {
					throw util::file::DataFileException {L"Duplicate shot name found: "s + n + L"."s, my_parser.getLine()};
				}
			}
		}

		void MyGame::init_tower_types() {
			std::wifstream data_file {this->resources_folder_path + L"towers.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/towers.ini for reading."s, 0};
			}
			util::file::DataFileParser my_parser {data_file};
			// Global section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"version"s);
			my_parser.expectToken(util::file::TokenTypes::Number, L"1"s);
			// Firing section
			my_parser.getNext();
			my_parser.expectToken(util::file::TokenTypes::Section, L"firing"s);
			my_parser.readKeyValue(L"firing_methods"s);
			my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
			std::map<std::wstring, std::shared_ptr<FiringMethod>> my_firing_methods {};
			my_parser.getNext();
			while (my_parser.matchToken(util::file::TokenTypes::Object, L"{"s)) {
				my_parser.readKeyValue(L"name"s);
				const std::wstring fm_name = my_parser.parseString();
				my_parser.readKeyValue(L"method"s);
				if (my_parser.getToken() == L"Default"s) {
					auto my_fmethod = std::make_shared<FiringMethod>(fm_name, FiringMethodTypes::Default);
					my_firing_methods.emplace(fm_name, std::move(my_fmethod));
				}
				else {
					const std::wstring fmethod_type_str = my_parser.getToken();
					my_parser.readKeyValue(L"angles"s);
					auto fm_angle_strs = my_parser.readList();
					std::vector<double> fm_angles {};
					for (auto& a : fm_angle_strs) {
						double base_angle_degrees = std::stod(a);
						while (base_angle_degrees < -180.0 || base_angle_degrees > 180.0) {
							if (base_angle_degrees > 180.0) {
								base_angle_degrees -= 360.0;
							}
							else {
								base_angle_degrees += 360.0;
							}
						}
						fm_angles.emplace_back(base_angle_degrees * math::pi / 180.0);
					}
					std::sort(fm_angles.begin(), fm_angles.end());
					const auto my_iterator = std::unique(fm_angles.begin(), fm_angles.end());
					fm_angles.erase(my_iterator, fm_angles.end());

					if (fmethod_type_str == L"Static"s) {
						auto my_fmethod = std::make_shared<FiringMethod>(fm_name, FiringMethodTypes::Static, fm_angles);
						my_firing_methods.emplace(fm_name, std::move(my_fmethod));
					}
					else {
						my_parser.readKeyValue(L"duration"s);
						const int fm_duration = static_cast<int>(my_parser.parseNumber());
						util::file::DataFileParser::validateNumberMinBound(fm_duration, 10, L"Duration (Firing method)", my_parser.getLine(), true);
						auto my_fmethod = std::make_shared<FiringMethod>(fm_name, FiringMethodTypes::Pulse, fm_angles, fm_duration);
						my_firing_methods.emplace(fm_name, std::move(my_fmethod));
					}
				}
				my_parser.getNext();
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
				my_parser.getNext();
			}
			my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
			// Targeting section
			my_parser.getNext();
			my_parser.expectToken(util::file::TokenTypes::Section, L"targeting"s);
			my_parser.readKeyValue(L"targeting_methods"s);
			my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
			std::map<std::wstring, std::shared_ptr<TargetingStrategy>> my_targeting_strategies {};
			my_parser.getNext();
			while (my_parser.matchToken(util::file::TokenTypes::Object, L"{"s)) {
				my_parser.readKeyValue(L"name"s);
				const std::wstring ts_name = my_parser.parseString();
				my_parser.readKeyValue(L"strategy"s);
				const std::wstring ts_strategy_str = my_parser.getToken();
				const TargetingStrategyTypes ts_strategy = ts_strategy_str == L"Distances"s ? TargetingStrategyTypes::Distances :
					ts_strategy_str == L"Statistics"s ? TargetingStrategyTypes::Statistics :
					ts_strategy_str == L"Names"s ? TargetingStrategyTypes::Names :
					throw util::file::DataFileException {L"Expected a valid targeting strategy: "s
						L" Distances, Statistics, or Names."s, my_parser.getLine()};
				my_parser.readKeyValue(L"protocol"s);
				const std::wstring ts_protocol_str = my_parser.getToken();
				const TargetingStrategyProtocols ts_protocol = ts_protocol_str == L"Lowest"s ? TargetingStrategyProtocols::Lowest :
					ts_protocol_str == L"Highest"s ? TargetingStrategyProtocols::Highest :
					throw util::file::DataFileException {L"Expected a valid protocol: Lowest or Highest."s,
						my_parser.getLine()};
				switch (ts_strategy) {
				case TargetingStrategyTypes::Distances:
				{
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_name, ts_strategy, ts_protocol);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				case TargetingStrategyTypes::Statistics:
				{
					const std::wstring ts_stat_str = my_parser.readKeyValue(L"statistic"s).second;
					const TargetingStrategyStatistics ts_stat = ts_stat_str == L"Damage"s ? TargetingStrategyStatistics::Damage :
						ts_stat_str == L"Health"s ? TargetingStrategyStatistics::Health :
						ts_stat_str == L"Armor_Health"s ? TargetingStrategyStatistics::Armor_Health :
						ts_stat_str == L"Armor_Reduce"s ? TargetingStrategyStatistics::Armor_Reduce :
						ts_stat_str == L"Speed"s ? TargetingStrategyStatistics::Speed :
						ts_stat_str == L"Buffs"s ? TargetingStrategyStatistics::Buffs :
						throw util::file::DataFileException {L"Expected a valid test statistic:"s
						L" Damage, Health, Armor_Health, Armor_Reduce, Speed, Buffs."s, my_parser.getLine()};
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_name, ts_strategy, ts_protocol, ts_stat);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				case TargetingStrategyTypes::Names:
				{
					// Note: I probably should verify the given names, but I'm not...
					my_parser.readKeyValue(L"target_names");
					const auto ts_target_names = my_parser.readList();
					auto my_ts_strategy = std::make_shared<TargetingStrategy>(ts_name, ts_strategy, ts_protocol, ts_target_names);
					my_targeting_strategies.emplace(ts_name, std::move(my_ts_strategy));
					break;
				}
				} // End switch
				my_parser.getNext();
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
				my_parser.getNext();
			}
			my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
			// Wall section
			my_parser.getNext();
			my_parser.expectToken(util::file::TokenTypes::Section, L"wall"s);
			my_parser.readKeyValue(L"name"s);
			const std::wstring wall_name = my_parser.parseString();
			my_parser.readKeyValue(L"desc"s);
			const std::wstring wall_desc = my_parser.parseString();
			const graphics::Color wall_color = my_parser.readColor();
			const graphics::shapes::ShapeTypes wall_shape = my_parser.readShape();
			my_parser.readKeyValue(L"cost"s);
			const int wall_cost = static_cast<int>(my_parser.parseNumber());
			util::file::DataFileParser::validateNumberMinBound(wall_cost, 1, L"Cost (Wall)", my_parser.getLine(), true);
			auto my_wall = std::make_shared<WallType>(wall_name, wall_desc, wall_color, wall_shape, wall_cost);
			this->tower_types.emplace_back(std::move(my_wall));
			// Trap section(s)
			my_parser.getNext();
			while (my_parser.matchToken(util::file::TokenTypes::Section, L"trap"s)) {
				my_parser.getNext();
				// For now, this is ignored...
				// (This allows me to add new stuff to the game without breaking the
				// file format.)
				while (!my_parser.matchTokenType(util::file::TokenTypes::Section)) {
					my_parser.getNext();
				}
			}
			do {
				// Tower section(s)
				my_parser.expectToken(util::file::TokenTypes::Section, L"tower"s);
				my_parser.readKeyValue(L"name"s);
				const std::wstring n = my_parser.parseString();
				my_parser.readKeyValue(L"desc"s);
				const std::wstring d = my_parser.parseString();
				const graphics::Color c = my_parser.readColor();
				const graphics::shapes::ShapeTypes st = my_parser.readShape();
				std::shared_ptr<FiringMethod> fmethod {nullptr};
				try {
					my_parser.readKeyValue(L"firing_method"s);
					fmethod = my_firing_methods.at(my_parser.parseString());
				}
				catch (const std::out_of_range&) {
					throw util::file::DataFileException {L"Unknown firing method name: "s + my_parser.parseString(),
						my_parser.getLine()};
				}
				std::shared_ptr<TargetingStrategy> tstrategy {nullptr};
				try {
					my_parser.readKeyValue(L"targeting_strategy"s);
					tstrategy = my_targeting_strategies.at(my_parser.parseString());
				}
				catch (const std::out_of_range&) {
					throw util::file::DataFileException {L"Unknown targeting strategy name: "s + my_parser.parseString(),
						my_parser.getLine()};
				}
				my_parser.readKeyValue(L"shots");
				my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
				std::vector<std::pair<const ShotBaseType*, double>> my_tower_shots {};
				my_parser.getNext();
				my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
				double freq_total = 0.0;
				do {
					const ShotBaseType* my_tower_shot_type {nullptr};
					try {
						my_parser.readKeyValue(L"name"s);
						my_tower_shot_type = this->getShotType(my_parser.parseString());
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"No projectile exists with the following name: "s
							+ my_parser.parseString() + L"."s, my_parser.getLine()};
					}
					my_parser.readKeyValue(L"frequency"s);
					const double my_tower_shot_freq = my_parser.parseNumber();
					util::file::DataFileParser::validateNumber(my_tower_shot_freq, 0.0, 1.1, L"Frequency", my_parser.getLine(), false, false);
					freq_total += my_tower_shot_freq;
					my_tower_shots.emplace_back(std::move(my_tower_shot_type), my_tower_shot_freq);
					my_parser.getNext();
					my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
					my_parser.getNext();
				} while (my_parser.matchToken(util::file::TokenTypes::Object, L"{"s));
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
				// Not strictly equal due to floating-point inprecision.
				util::file::DataFileParser::validateNumber(freq_total, 0.999, 1.05, L"Combined frequency", my_parser.getLine(), false, false);
				my_parser.readKeyValue(L"max_level"s);
				const int max_lv = static_cast<int>(my_parser.parseNumber());
				util::file::DataFileParser::validateNumber(max_lv, 1, 31, L"Max level", my_parser.getLine(), true, true);
				my_parser.readKeyValue(L"firing_speed"s);
				const double fs = my_parser.parseNumber();
				util::file::DataFileParser::validateNumber(fs, 0.1, 60.0, L"Firing speed", my_parser.getLine(), true, true);
				my_parser.readKeyValue(L"firing_range"s);
				const double fr = my_parser.parseNumber();
				util::file::DataFileParser::validateNumber(fr, 0.5, 30.5, L"Firing range", my_parser.getLine(), true, true);
				my_parser.readKeyValue(L"volley_shots"s);
				const int vs = static_cast<int>(my_parser.parseNumber());
				util::file::DataFileParser::validateNumberMinBound(vs, 0, L"Volley shots", my_parser.getLine(), true);
				my_parser.readKeyValue(L"reload_delay"s);
				const int rd = static_cast<int>(my_parser.parseNumber());
				if (vs == 0 && rd != 0) {
					throw util::file::DataFileException {L"Reload delay must be zero if volley shots is zero."s,
						my_parser.getLine()};
				}
				else if (vs > 0) {
					util::file::DataFileParser::validateNumberMinBound(rd, 10, L"Reload delay (ms)", my_parser.getLine(), true);
				}
				my_parser.readKeyValue(L"cost_adjust"s);
				const int cost_adj = static_cast<int>(my_parser.parseNumber());
				auto my_tower_type = std::make_shared<TowerType>(n, d, c, st, fmethod, tstrategy,
					std::move(my_tower_shots), fs, fr, vs, rd, cost_adj, max_lv);
				util::file::DataFileParser::validateNumberMinBound(my_tower_type->getCost(), 0.0, L"Tower Cost", my_parser.getLine(), false);
				this->tower_types.emplace_back(std::move(my_tower_type));
			} while (my_parser.getNext());
		}

		void MyGame::load_tower_upgrades_data() {
			std::wifstream data_file {this->resources_folder_path + L"tower_upgrades.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/towers.ini for reading."s, 0};
			}
			util::file::DataFileParser my_parser {data_file};
			// Global section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"version"s);
			my_parser.expectToken(util::file::TokenTypes::Number, L"1"s);
			my_parser.getNext();
			do {
				// Upgrade section(s)
				my_parser.expectToken(util::file::TokenTypes::Section, L"upgrade"s);
				my_parser.readKeyValue(L"for");
				const std::wstring tower_name = my_parser.parseString();
				TowerType* my_ttype = nullptr;
				for (auto& ttype : this->getAllTowerTypes()) {
					if (ttype->getName() == tower_name) {
						my_ttype = ttype.get();
						break;
					}
				}
				if (my_ttype == nullptr) {
					throw util::file::DataFileException {L"Tower name not found: " + tower_name + L".", my_parser.getLine()};
				}
				my_parser.readKeyValue(L"upgrades");
				my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
				my_parser.getNext();
				while (my_parser.matchToken(util::file::TokenTypes::Object, L"{"s)) {
					my_parser.readKeyValue(L"upgrade_level");
					const int upgrade_level = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumber(upgrade_level, 2, my_ttype->getMaxLevel(),
						L"Upgrade level for " + tower_name, my_parser.getLine(), true, true);
					my_parser.readKeyValue(L"upgrade_option");
					const unsigned int upgrade_option_number = static_cast<unsigned int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumber(upgrade_option_number, 0U, 1U, L"Upgrade option", my_parser.getLine(), true, true);
					const TowerUpgradeOption upgrade_option = upgrade_option_number == 0
						? TowerUpgradeOption::One : TowerUpgradeOption::Two;
					my_parser.readKeyValue(L"upgrade_cost_percent");
					const double upgrade_cost_percent = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_cost_percent, 0.0, L"Upgrade cost percent", my_parser.getLine(), false);
					my_parser.readKeyValue(L"special_type");
					if (!my_parser.matchTokenType(util::file::TokenTypes::Identifier)) {
						throw util::file::DataFileException {L"Expected an identifier (no quotes).", my_parser.getLine()};
					}
					const std::wstring upgrade_special_str = my_parser.getToken();
					const TowerUpgradeSpecials upgrade_special = upgrade_special_str == L"None"
						? TowerUpgradeSpecials::None : upgrade_special_str == L"Extra_Cash"
						? TowerUpgradeSpecials::Extra_Cash : upgrade_special_str == L"Multishot"
						? TowerUpgradeSpecials::Multishot : upgrade_special_str == L"Mega_Missile"
						? TowerUpgradeSpecials::Mega_Missile : upgrade_special_str == L"Fast_Reload"
						? TowerUpgradeSpecials::Fast_Reload : throw util::file::DataFileException {L"Invalid upgrade special type provided.",
						my_parser.getLine()};
					my_parser.readKeyValue(L"special_chance");
					const double upgrade_special_chance = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_special_chance, 0.0, L"Special chance", my_parser.getLine(), true);
					my_parser.readKeyValue(L"special_power");
					const double upgrade_special_power = my_parser.parseNumber();
					// I should validate special power but I don't see a real reason to do it right now.
					my_parser.readKeyValue(L"damage_multi");
					const double upgrade_damage_multi = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_damage_multi, 0.0, L"Damage multi", my_parser.getLine(), false);
					my_parser.readKeyValue(L"speed_multi");
					const double upgrade_speed_multi = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_speed_multi, 0.0, L"Speed multi", my_parser.getLine(), false);
					my_parser.readKeyValue(L"range_multi");
					const double upgrade_range_multi = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_range_multi, 0.0, L"Range multi", my_parser.getLine(), false);
					my_parser.readKeyValue(L"ammo_multi");
					const double upgrade_ammo_multi = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_ammo_multi, 0.0, L"Ammo multi", my_parser.getLine(), false);
					my_parser.readKeyValue(L"delay_multi");
					const double upgrade_delay_multi = my_parser.parseNumber();
					util::file::DataFileParser::validateNumberMinBound(upgrade_delay_multi, 0.0, L"Delay multi", my_parser.getLine(), false);
					my_ttype->addUpgradeInfo(TowerUpgradeInfo {upgrade_level, upgrade_option, upgrade_cost_percent,
						upgrade_damage_multi, upgrade_speed_multi, upgrade_range_multi, upgrade_ammo_multi,
						upgrade_delay_multi, upgrade_special, upgrade_special_chance, upgrade_special_power});
					my_parser.getNext();
					my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
					my_parser.getNext();
				}
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
			} while (my_parser.getNext());

		}

		void MyGame::load_global_level_data() {
			std::wifstream data_file {this->resources_folder_path + L"levels/global.ini"};
			if (data_file.bad() || data_file.fail()) {
				throw util::file::DataFileException {L"Could not open resources/levels/global.ini for reading."s, 0};
			}
			util::file::DataFileParser my_parser {data_file};
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"backup_level_if_load_fails"s);
			this->my_level_backup_number = static_cast<int>(my_parser.parseNumber());
			if (this->my_level_backup_number < 1) {
				this->my_level_backup_number = 1;
			}
		}

		void MyGame::load_level_data() {
			std::wifstream data_file {this->resources_folder_path + L"levels/level"s + std::to_wstring(this->level) + L".ini"s};
			if (data_file.bad() || data_file.fail()) {
				data_file.open(this->resources_folder_path + L"levels/level"s + std::to_wstring(this->my_level_backup_number) + L".ini"s);
				if (data_file.bad() || data_file.fail()) {
					throw util::file::DataFileException {L"Could not open resources/levels/level"s
						+ std::to_wstring(this->level) + L".ini for reading!"s, 0};
				}
			}
			util::file::DataFileParser my_parser {data_file};
			// Global section
			my_parser.expectToken(util::file::TokenTypes::Section, L"global"s);
			my_parser.readKeyValue(L"version"s);
			my_parser.expectToken(util::file::TokenTypes::Number, L"1"s);
			my_parser.readKeyValue(L"wave_spawn_delay"s);
			const int wave_spawn_delay = static_cast<int>(my_parser.parseNumber());
			util::file::DataFileParser::validateNumber(wave_spawn_delay, 200, 30'000, L"Wave spawn delay (ms)", my_parser.getLine(), true, true);
			my_parser.getNext();
			std::deque<std::unique_ptr<EnemyWave>> my_level_waves {};
			do {
				// [wave] sections
				my_parser.expectToken(util::file::TokenTypes::Section, L"wave"s);
				my_parser.readKeyValue(L"group_spawn_delay"s);
				const int group_spawn_delay = static_cast<int>(my_parser.parseNumber());
				util::file::DataFileParser::validateNumber(group_spawn_delay, 100, 15'000, L"Group spawn delay (ms)", my_parser.getLine(), true, true);
				std::deque<std::unique_ptr<EnemyGroup>> my_wave_groups {};
				my_parser.readKeyValue(L"groups"s);
				my_parser.expectToken(util::file::TokenTypes::Object, L"{"s);
				my_parser.getNext();
				do {
					my_parser.readKeyValue(L"enemy_name"s);
					const std::wstring enemy_name = my_parser.parseString();
					const EnemyType* etype = nullptr;
					// Check the enemy exists.
					try {
						etype = this->getEnemyType(enemy_name);
					}
					catch (const std::out_of_range&) {
						throw util::file::DataFileException {L"Enemy type not found: "s + enemy_name + L"."s,
							my_parser.getLine()};
					}
					my_parser.readKeyValue(L"extra_count"s);
					int enemy_count = static_cast<int>(my_parser.parseNumber()) + this->challenge_level + 2;
					util::file::DataFileParser::validateNumberMinBound(enemy_count - this->challenge_level - 2, 0,
						L"Extra count", my_parser.getLine(), true);
					my_parser.readKeyValue(L"enemy_spawn_delay"s);
					const int enemy_spawn_delay = static_cast<int>(my_parser.parseNumber());
					util::file::DataFileParser::validateNumber(enemy_spawn_delay, 10, 10'000,
						L"Enemy spawn delay (ms)", my_parser.getLine(), true, true);
					std::queue<std::unique_ptr<Enemy>> my_enemy_spawns {};
					if (etype->isUnique()) {
						enemy_count -= this->getChallengeLevel() + 1;
					}
					pathfinding::Pathfinder my_pathfinder {this->getMap(), etype->isFlying(), etype->canMoveDiagonally(),
						etype->getDefaultStrategy()};
					my_pathfinder.findPath(this->challenge_level / 10.0);
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
					my_parser.getNext();
					my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
					my_parser.getNext();
				} while (my_parser.matchToken(util::file::TokenTypes::Object, L"{"s));
				my_parser.expectToken(util::file::TokenTypes::Object, L"}"s);
				auto my_wave = std::make_unique<EnemyWave>(std::move(my_wave_groups), group_spawn_delay);
				my_level_waves.emplace_front(std::move(my_wave));
			} while (my_parser.getNext());
			this->my_level = std::make_unique<GameLevel>(this->level, std::move(my_level_waves), wave_spawn_delay);
			this->my_level_enemy_count = this->my_level->getEnemyCount();
		}

		void MyGame::saveGame(std::wostream& save_file) const {
			if (this->isInLevel() || !this->player.isAlive()) {
				// Can't save now...
				return;
			}
			save_file << L"V: " << 4 << L"\n";
			save_file << L"C: " << this->challenge_level << L" D: " << this->difficulty
				<< L" L: " << this->level << L"\n";
			save_file << L"H: " << this->player.getHealth() << L" K: " << this->hp_buy_cost
				<< L" M: " << this->player.getMoney() << L"\n";
			save_file << L"W: " << this->win_streak << L" L: " << this->lose_streak << L"\n";
			// Output map name (mainly for the terrain editor rather than the game itself.)
			save_file << L"MN: " << this->map_base_name << L"\n";
			// Output terrain map
			save_file << this->getMap().getTerrainGraph(false) << L"\n";
			save_file << this->getMap().getTerrainGraph(true) << L"\n";
			// Output influence map
			save_file << this->getMap().getInfluenceGraph(false) << L"\n";
			save_file << this->getMap().getInfluenceGraph(true) << L"\n";
			// Output towers as well.
			for (const auto& t : this->towers) {
				save_file << L"T: " << t->getBaseType()->getName() << L"\n\tT: " << t->getGameX() << L" "
					<< t->getGameY() << L" " << t->getLevel() << L" " << t->getUpgradePath() << L"\n";
			}
			// Output seen enemies.
			for (const auto& seen_e : this->enemies_seen) {
				if (seen_e.second) {
					save_file << L"E: " << seen_e.first << L"\n\tK: "
						<< std::hex << this->enemy_kill_count.at(seen_e.first) << std::dec << L"\n";
				}
			}
		}

		void MyGame::loadGame(std::wistream& save_file) {
			std::wstring buffer {};
			int version;
			save_file >> buffer >> version;
			if (version >= 1 && version <= 4) {
				save_file >> buffer >> this->challenge_level >> buffer >> this->difficulty
					>> buffer >> this->level;
				int player_health;
				double player_cash;
				if (version <= 2) {
					save_file >> buffer >> player_health >> buffer >> player_cash;
				}
				else {
					save_file >> buffer >> player_health >> buffer >> this->hp_buy_cost
						>> buffer >> player_cash;
				}
				this->player = Player {player_cash, player_health};
				save_file >> buffer >> this->win_streak >> buffer >> this->lose_streak;
				// Read or determine map name.
				if (version >= 4) {
					save_file >> buffer;
					std::getline(save_file, this->map_base_name);
					// Leading space is removed...
					this->map_base_name.erase(this->map_base_name.begin());
				}
				else {
					switch (this->getChallengeLevel()) {
					case 0:
						this->map_base_name = L"beginner";
						break;
					case 1:
						this->map_base_name = L"intermediate";
						break;
					case 2:
						this->map_base_name = L"experienced";
						break;
					case 3:
						this->map_base_name = L"expert";
						break;
					default:
						this->map_base_name = L"intermediate";
						this->challenge_level = 1;
					}
				}
				// Terrain map
				// (This is tricky --> Due to the way the start and end nodes are loaded,
				// I need to TRANSFER the ownership of these resources TO this->map.)
				auto my_gterrain = std::make_unique<pathfinding::Grid>();
				auto my_aterrain = std::make_unique<pathfinding::Grid>();
				save_file >> *my_gterrain >> *my_aterrain;
				this->map = std::make_shared<game::GameMap>(std::move(my_gterrain), std::move(my_aterrain));
				// Note that above move invalidates the test paths.
				this->ground_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), false,
					false, pathfinding::HeuristicStrategies::Manhattan);
				this->air_test_pf = std::make_shared<pathfinding::Pathfinder>(this->getMap(), true,
					false, pathfinding::HeuristicStrategies::Manhattan);
				// Influence map
				pathfinding::Grid ground_influence_map {};
				pathfinding::Grid air_influence_map {};
				save_file >> ground_influence_map >> air_influence_map;
				this->map->setInfluenceGraphs(ground_influence_map, air_influence_map);
				while (save_file >> buffer) {
					if (buffer == L"T:") {
						// Towers
#pragma warning(push)
#pragma warning(disable: 26494) // Code Analysis: type.5 --> Always initialize.
						std::wstring tower_name;
						double tower_gx;
						double tower_gy;
#pragma warning(pop)
						std::getline(save_file, tower_name);
						// Leading space is removed...
						tower_name.erase(tower_name.begin());
						save_file >> buffer >> tower_gx >> tower_gy;
						const TowerType* my_type {nullptr};
						for (const auto& tt : this->getAllTowerTypes()) {
							if (tt->getName() == tower_name) {
								my_type = tt.get();
								break;
							}
						}
						if (!my_type) {
							throw std::runtime_error {"Error: Tower does not exist!"};
						}
						auto my_tower = std::make_unique<Tower>(this->getDeviceResources(), my_type,
							graphics::Color {0.f, 0.f, 0.f, 1.f}, tower_gx, tower_gy);
						if (version >= 2) {
							// Special code to handle upgrades (while not breaking old files.)
							int tower_lv;
							int tower_path;
							save_file >> tower_lv >> tower_path;
							my_tower->setTowerUpgradeStatus(tower_lv, tower_path);
						}
						this->addTower(std::move(my_tower));
						const auto my_floored_gx = static_cast<int>(std::floor(tower_gx));
						const auto my_floored_gy = static_cast<int>(std::floor(tower_gy));
						this->getMap().getFiterGraph(false).getNode(my_floored_gx, my_floored_gy).setBlockage(true);
						this->getMap().getFiterGraph(true).getNode(my_floored_gx, my_floored_gy).setBlockage(true);
					}
					else if (buffer == L"E:") {
						// Load seen enemies.
						std::wstring enemy_name {};
						std::getline(save_file, enemy_name);
						// Leading space is removed...
						enemy_name.erase(enemy_name.begin());
						long long kill_count {0};
						if (version >= 3) {
							save_file >> buffer >> std::hex >> kill_count >> std::dec;
						}
						try {
							this->enemies_seen.at(enemy_name) = true;
							this->enemy_kill_count.at(enemy_name) = kill_count;
						}
						catch (const std::out_of_range&) {
							// Ignore.
						}
					}
				}
			}
			else {
				throw util::file::DataFileException {L"Unrecognized save file version."s, 1};
			}
		}
	}
}