#pragma once
// File Author: Isaiah Hoffman
// File Created: March 20, 2018
#include <cmath>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "./../globals.hpp"
#include "./../ih_math.hpp"

namespace hoffman_isaiah {
	namespace pathfinding {
		// Forward declaration
		class Pathfinder;
	}

	namespace graphics {
		// Forward declarations
		class Renderer2D;
		namespace DX {
			class DeviceResources2D;
		}
	}

	namespace game {
		// Forward declarations
		class GameMap;
		class EnemyType;
		class Enemy;
		class ShotBaseType;
		class Shot;
		class TowerType;
		class Tower;
		class GameLevel;
		class LevelGenerator;

		// Debug-related update states
		enum class DebugUpdateStates {
			Terrain_Changed
		};

		// Global variables
		/// <summary>Global pointer to the my_game instance.</summary>
		extern std::shared_ptr<MyGame> g_my_game;

		/// <summary>Class that represents the player.</summary>
		class Player {
		public:
			constexpr Player() = default;
			constexpr Player(double cash, int hp) :
				money {cash},
				health {hp} {
			}
			// Getters
			constexpr double getMoney() const noexcept {
				return this->money;
			}
			constexpr int getHealth() const noexcept {
				return this->health;
			}
			constexpr bool isAlive() const noexcept {
				return this->getHealth() > 0;
			}
			// Setters and Changers
			constexpr void changeMoney(double amt) noexcept {
				this->money += amt;
			}
			constexpr void changeHealth(int amt) noexcept {
				this->health += amt;
			}
		private:
			/// <summary>The amount of money the player possesses.</summary>
			double money {150.0};
			/// <summary>The amount of health the player possesses.</summary>
			int health {20};
		};

		/// <summary>Class that represents an instance of the game itself.</summary>
		class MyGame {
			friend class graphics::Renderer2D;
		public:
			// Other stuff
			MyGame(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res);
			~MyGame() noexcept;
			// Rule of 5:
			MyGame(const MyGame&) = delete;
			MyGame(MyGame&&);
			MyGame& operator=(const MyGame&) = delete;
			MyGame& operator=(MyGame&&);
			/// <summary>Resets the game's state.</summary>
			/// <param name="new_clevel">The new challenge level to set.</param>
			/// <param name="map_name">The base name of the new map to use.</param>
			/// <param name="is_custom">Is the new game a custom game?</param>
			void resetState(int new_clevel, std::wstring map_name, bool is_custom = false);
			/// <summary>Updates the state of the game by one tick.</summary>
			void update();
			/// <summary>Updates the state of the game in some way for debugging reasons.</summary>
			/// <param name="cause">The reason to update the game state.</param>
			void debugUpdate(DebugUpdateStates cause);
			// Note: Defined in data_loading.cpp
			/// <summary>Loads application data.</summary>
			/// <param name="ran_once">Has this method been called before?</param>
			void load_config_data(bool ran_once = false);
			/// <summary>Initializes the list of enemy types in this game.</summary>
			void init_enemy_types();
			/// <summary>Initializes the list of shot types in this game.</summary>
			void init_shot_types();
			/// <summary>Initializes the list of tower types in this game.</summary>
			void init_tower_types();
			/// <summary>Loads the upgrade data for towers.</summary>
			void load_tower_upgrades_data();
			/// <summary>Loads miscellaneous data global to the application.</summary>
			void load_global_misc_data();
			/// <summary>Loads data that applies to all levels.</summary>
			void load_global_level_data();
			/// <summary>Loads the level data for the current level.</summary>
			void load_level_data();
			// Other stuff:
			/// <summary>Saves the game state.</summary>
			/// <param name="save_file">The file to save the game's state to.</param>
			void saveGame(std::wostream& save_file) const;
			/// <summary>Loads a previously saved game state.</summary>
			/// <param name="save_file">The file to load the game's state from.</param>
			void loadGame(std::wistream& save_file);
			/// <summary>Saves global user data.</summary>
			void saveGlobalData() const;
			/// <summary>Loads global user data.</summary>
			void loadGlobalData();
			/// <summary>Adds an enemy to the game.</summary>
			/// <param name="e">The enemy to add.</param>
			void addEnemy(std::unique_ptr<Enemy>&& e);
			/// <summary>Adds a tower to the game.</summary>
			/// <param name="e">The tower to add.</param>
			void addTower(std::unique_ptr<Tower>&& t);
			// Player Actions:
			/// <summary>Starts the next wave.</summary>
			void startWave();
			/// <summary>Toggles the pause state of the game.</summary>
			void togglePause() noexcept {
				this->is_paused = !this->is_paused;
			}
			/// <summary>Selects a new tower type from the available choices.</summary>
			/// <param name="selection">The tower that was selected. -1 => None;
			/// otherwise, valid values are between 0 and tower_types.size() - 1.</param>
			void selectTower(int selection) {
				this->selected_tower = selection;
			}
			/// <summary>Attempts to buy health for the player.</summary>
			void buyHealth();
			/// <summary>Changes the speed at which the game is updated.</summary>
			void changeUpdateSpeed();
			/// <summary>Attempts to buy and place a tower at the given game coordinates.</summary>
			/// <param name="gx">The x-location of the place to build (in game coordinate squares).</param>
			/// <param name="gy">The y-location of the place to build (in game coordinate squares).</param>
			void buyTower(int gx, int gy);
			/// <summary>Attempts to sell and remove a tower at the given game coordinates.</summary>
			/// <param name="gx">The x-location of the tower to destroy (in game coordinate squares).</param>
			/// <param name="gy">The y-location of the tower to destroy (in game coordinate squares).</param>
			void sellTower(int gx, int gy);
			// Misc:
			/// <summary>Toggles the showing of paths (as an aid for debugging pathfinding).</summary>
			void toggleShowPaths() noexcept {
				this->show_test_paths = !this->show_test_paths;
			}
			/// <summary>Inverts the showing of a tower's shooting range for all towers.</summary>
			void toggleAllRadii() noexcept;
			/// <summary>Sets an enemy type as "seen" before.</summary>
			/// <param name="name">The name of the type seen.</param>
			void setEnemyTypeAsSeen(std::wstring name) noexcept {
				this->enemies_seen.at(name) = true;
			}
			/// <param name="amt">The amount of money to add (or remove) from the player.</param>
			void changePlayerCash(double amt) noexcept {
				this->player.changeMoney(amt);
			}
			/// <param name="is_custom">Is the current game a custom game?</param>
			void setGameType(bool is_custom) noexcept {
				this->in_custom_game = is_custom;
			}
			// Getters
			std::shared_ptr<graphics::DX::DeviceResources2D> getDeviceResources() const noexcept {
				return this->device_resources;
			}
			GameMap& getMap() noexcept {
				return *this->map;
			}
			const GameMap& getMap() const noexcept {
				return *this->map;
			}
			std::wstring getMapBaseName() const noexcept {
				return this->map_base_name;
			}
			/// <param name="new_challenge">The new challenge level to set (resource identifier).</param>
			/// <returns>The map base name of the associat
			std::wstring getDefaultMapName(int new_challenge) const;
			/// <param name="name">The name of the enemy type to obtain.</param>
			const EnemyType* getEnemyType(std::wstring name) const;
			/// <param name="index">The zero-based index of the enemy to retrieve (relative to the beginning of the file).</param>
			const EnemyType* getEnemyType(int index) const {
				return this->enemy_types.at(index).get();
			}
			const std::vector<std::unique_ptr<EnemyType>>& getAllEnemyTypes() const noexcept {
				return this->enemy_types;
			}
			std::map<std::wstring, bool> getSeenEnemies() const noexcept {
				return this->enemies_seen;
			}
			const ShotBaseType* getShotType(std::wstring name) const {
				return this->shot_types.at(name).get();
			}
			const std::map<std::wstring, std::unique_ptr<ShotBaseType>>& getAllShotTypes() const noexcept {
				return this->shot_types;
			}
			const TowerType* getTowerType(int i) const {
				return this->tower_types.at(i).get();
			}
			const std::vector<std::unique_ptr<TowerType>>& getAllTowerTypes() const noexcept {
				return this->tower_types;
			}
			std::vector<std::unique_ptr<Enemy>>& getEnemies() noexcept {
				return this->enemies;
			}
			std::vector<std::unique_ptr<Tower>>& getTowers() noexcept {
				return this->towers;
			}
			bool isPaused() const noexcept {
				return this->is_paused;
			}
			bool isInLevel() const noexcept {
				return this->in_level;
			}
			/// <returns>-1 indicates that "None" is selected. Otherwise, the number
			/// corresponds with the tower type's index in the list.</returns>
			int getSelectedTower() const noexcept {
				return this->selected_tower;
			}
			int getLevelNumber() const noexcept {
				return this->level;
			}
			double getDifficulty() const noexcept {
				return this->difficulty;
			}
			int getChallengeLevel() const noexcept {
				return this->challenge_level;
			}
			int getHealthBuyCost() const noexcept {
				return static_cast<int>(std::ceil(this->hp_buy_cost));
			}
			std::wstring getResourcesPath() const noexcept {
				return this->resources_folder_path;
			}
			std::wstring getUserDataPath() const noexcept {
				return this->userdata_folder_path;
			}
			/// <returns>The amount of money the player possesses.</returns>
			double getPlayerCash() const noexcept {
				return this->player.getMoney();
			}
			/// <returns>The new speed that the game will update at based on the current update speed.</returns>
			int getNextUpdateSpeed() const noexcept {
				switch (this->update_speed) {
				case 1:
					return 2;
				case 2:
					return 3;
				case 3:
					return 5;
				case 5:
					return 8;
				default:
					return 1;
				}
			}
			bool canStartCustomGames() const noexcept {
				return this->start_custom_games;
			}
			long long getHiscore() const noexcept {
				return this->highest_score;
			}
			const std::map<int, int>& getHighestLevels() const noexcept {
				return this->highest_levels;
			}
		protected:
			/// <summary>Calculates the player's final score.</summary>
			/// <returns>The calculated final score.</returns>
			long long calculateScore() const noexcept {
				const long long clevel_mod_numerator = (this->getChallengeLevel() + 1ll) * 2ll;
				constexpr const long long clevel_mod_denominator = 3ll;
				const long long level_score_component = this->getLevelNumber() <= 5
					? this->getLevelNumber() : this->getLevelNumber() <= 10
					? this->getLevelNumber() * 2ll : this->getLevelNumber() <= 25
					? this->getLevelNumber() * 5ll : this->getLevelNumber() <= 50
					? this->getLevelNumber() * 10ll : this->getLevelNumber() <= 75
					? this->getLevelNumber() * 15ll : this->getLevelNumber() <= 90
					? this->getLevelNumber() * 20ll : this->getLevelNumber() <= 95
					? this->getLevelNumber() * 25ll : this->getLevelNumber() <= 99
					? this->getLevelNumber() * 33ll : this->getLevelNumber() * 50ll;
				const long long adjusted_difficulty_numerator = static_cast<long long>(this->getDifficulty() * 1000.0l);
				constexpr const long long adjusted_difficulty_denominator = 1000ll;
				const long long difficulty_score_component = (this->getLevelNumber() <= 5
					? 0ll : this->getLevelNumber() <= 10
					? adjusted_difficulty_numerator * 13ll: this->getLevelNumber() <= 25
					? adjusted_difficulty_numerator * 31ll : this->getLevelNumber() <= 50
					? adjusted_difficulty_numerator * 63ll : this->getLevelNumber() <= 75
					? adjusted_difficulty_numerator * 94ll : this->getLevelNumber() <= 90
					? adjusted_difficulty_numerator * 113ll : this->getLevelNumber() <= 95
					? adjusted_difficulty_numerator * 119ll : this->getLevelNumber() <= 99
					? adjusted_difficulty_numerator * 124ll : adjusted_difficulty_numerator * 125ll) / adjusted_difficulty_denominator;
				const long long score_bonus = this->getLevelNumber() > 99 ? 25000ll
					: this->getLevelNumber() > 95 ? 20000ll
					: this->getLevelNumber() > 90 ? 15000ll
					: this->getLevelNumber() > 75 ? 10000ll
					: this->getLevelNumber() > 50 ? 7500ll
					: this->getLevelNumber() > 25 ? 5000ll
					: this->getLevelNumber() > 10 ? 2500ll : 0ll;
				return score_bonus + clevel_mod_numerator * (level_score_component + difficulty_score_component) / clevel_mod_denominator;
			}
			/// <summary>Updates the value of the dynamic difficulty variable.</summary>
			void updateDifficulty() noexcept {
				if (!this->did_lose_life) {
					++this->win_streak;
					this->lose_streak = 0;
				}
				else {
					++this->lose_streak;
					this->win_streak = 0;
				}
				const double adjusted_challenge_level = this->getChallengeLevel() == 0 ? 0.500
					: this->getChallengeLevel() == 1 ? 1.000
					: this->getChallengeLevel() == 2 ? 1.750
					: this->getChallengeLevel() == 3 ? 2.333 : 2.500;
				if (this->win_streak > 1) {
					// Increase the game's difficulty some
					this->difficulty += 0.005 * adjusted_challenge_level * math::get_min(this->win_streak - 1, 100);
				}
				else if (this->lose_streak > 0) {
					// Decrease the game's difficulty
					this->difficulty -= 0.015 * adjusted_challenge_level * math::get_min(this->lose_streak, 20)
						+ this->level * 0.001;
				}
				// Else do nothing
			}
		private:
			/// <summary>Shared pointer to the device resources.</summary>
			std::shared_ptr<graphics::DX::DeviceResources2D> device_resources;
			/// <summary>The base name of the current map.</summary>
			std::wstring map_base_name {L"intermediate"};
			/// <summary>The game map being used by the program.</summary>
			std::shared_ptr<GameMap> map {nullptr};
			/// <summary>The list of enemy template types.</summary>
			std::vector<std::unique_ptr<game::EnemyType>> enemy_types {};
			/// <summary>Stores which enemy types have been seen before.</summary>
			std::map<std::wstring, bool> enemies_seen {};
			/// <summary>Stores how many times each enemy type has been killed.</summary>
			std::map<std::wstring, long long> enemy_kill_count {};
			/// <summary>The list of enemies that are currently alive.</summary>
			std::vector<std::unique_ptr<game::Enemy>> enemies {};
			/// <summary>The list of shot template types.</summary>
			std::map<std::wstring, std::unique_ptr<game::ShotBaseType>> shot_types {};
			/// <summary>The list of projectiles that are currently active.</summary>
			std::vector<std::unique_ptr<game::Shot>> shots {};
			/// <summary>The list of tower template types.</summary>
			std::vector<std::unique_ptr<game::TowerType>> tower_types {};
			/// <summary>The list of towers currently in the game.</summary>
			std::vector<std::unique_ptr<game::Tower>> towers {};
			/// <summary>The player's health and cash.</summary>
			Player player {};
			/// <summary>The current level number the player is on.</summary>
			int level {1};
			/// <summary>The current game level the player is on.</summary>
			std::unique_ptr<GameLevel> my_level {nullptr};
			/// <summary>The total number of enemies that were created for a certain level.</summary>
			int my_level_enemy_count {0};
			/// <summary>The total number of enemies that were kill in a certain level.</summary>
			int my_level_enemy_killed {0};
			/// <summary>The game level to use if the loading of a level file fails.</summary>
			int my_level_backup_number {-1};
			/// <summary>The current dynamic difficulty level the player is at.</summary>
			double difficulty {1.00};
			/// <summary>The current game difficulty level the player is at.</summary>
			int challenge_level {1};
			/// <summary>The number of levels in a row that a player has lost life.</summary>
			int lose_streak {0};
			/// <summary>The number of levels in a row that a player has NOT lost any life.</summary>
			int win_streak {0};
			/// <summary>Whether or not the player has lost life this level.</summary>
			bool did_lose_life {false};
			/// <summary>Is the game currently paused?</summary>
			bool is_paused {false};
			/// <summary>Is a level currently in progress?</summary>
			bool in_level {false};
			/// <summary>The currently selected tower.</summary>
			int selected_tower {-1};
			/// <summary>The amount of health gained each time health is bought.</summary>
			int hp_gained_per_buy {0};
			/// <summary>The current cost to buy health.</summary>
			double hp_buy_cost {0};
			/// <summary>The cost to buy health is multiplied by this number each time.</summary>
			double hp_buy_multiplier {1.00};
			/// <summary>The number of times to update the game per logical frame.</summary>
			int update_speed {1};
			/// <summary>Stores the path to the resources folder.</summary>
			std::wstring resources_folder_path {L"./resources/"};
			/// <summary>Stores the path to the userdata folder.</summary>
			std::wstring userdata_folder_path {L"./userdata/"};
			/// <summary>Stores the automatic level generator.</summary>
			std::unique_ptr<LevelGenerator> my_level_generator {nullptr};
			/// <summary>Is the player allowed to load and play custom games and maps?</summary>
			bool start_custom_games {false};
			/// <summary>Is the player in a custom game?</summary>
			bool in_custom_game {false};
			/// <summary>Stores the highest score obtained by the player.</summary>
			long long highest_score {0};
			/// <summary>Is the current score a high score?</summary>
			bool is_hiscore {false};
			/// <summary>Maps the highest levels reached on each difficulty.</summary>
			std::map<int, int> highest_levels;
			// Testing things
			std::shared_ptr<pathfinding::Pathfinder> ground_test_pf {nullptr};
			std::shared_ptr<pathfinding::Pathfinder> air_test_pf {nullptr};
			bool show_test_paths {false};
		};
	}
}