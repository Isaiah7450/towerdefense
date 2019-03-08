#pragma once
// File Author: Isaiah Hoffman
// File Created: March 20, 2018
#include <string>
#include <memory>
#include <iosfwd>
#include <vector>
#include <map>
#include "./../globals.hpp"

namespace hoffman::isaiah {
	namespace pathfinding {
		// Forward declaration
		class Pathfinder;
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
			MyGame(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res, int clevel,
				std::wistream& ground_terrain_file, std::wistream& air_terrain_file);
			~MyGame() noexcept;
			/// <summary>Resets the game's state.</summary>
			void resetState();
			/// <summary>Updates the state of the game by one tick.</summary>
			void update();
			/// <summary>Updates the state of the game in some way for debugging reasons.</summary>
			/// <param name="cause">The reason to update the game state.</param>
			void debugUpdate(DebugUpdateStates cause);
			// Note: Defined in data_loading.cpp
			/// <summary>Initializes the list of enemy types in this game.</summary>
			void init_enemy_types();
			/// <summary>Initializes the list of shot types in this game.</summary>
			void init_shot_types();
			/// <summary>Initializes the list of tower types in this game.</summary>
			void init_tower_types();
			/// <summary>Loads miscellaneous data global to the application.</summary>
			void load_global_misc_data();
			/// <summary>Loads data that applies to all levels.</summary>
			void load_global_level_data();
			/// <summary>Loads the level data for the current level.</summary>
			void load_level_data();
			/// <summary>Saves the game state.</summary>
			/// <param name="save_file">The file to save the game's state to.</param>
			void saveGame(std::wostream& save_file) const;
			/// <summary>Loads a previously saved game state.</summary>
			/// <param name="save_file">The file to load the game's state from.</param>
			void loadGame(std::wistream& save_file);
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
			std::shared_ptr<EnemyType> getEnemyType(std::wstring name) {
				return this->enemy_types.at(name);
			}
			std::shared_ptr<ShotBaseType> getShotType(std::wstring name) {
				return this->shot_types.at(name);
			}
			std::shared_ptr<TowerType> getTowerType(int i) {
				return this->tower_types.at(i);
			}
			const std::vector<std::shared_ptr<TowerType>>& getAllTowerTypes() const noexcept {
				return this->tower_types;
			}
			std::vector<std::unique_ptr<Enemy>>& getEnemies() noexcept {
				return this->enemies;
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
			int getChallengeLevel() const noexcept {
				return this->challenge_level;
			}
			int getHealthBuyCost() const noexcept {
				return static_cast<int>(std::ceil(this->hp_buy_cost));
			}
		protected:
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
				if (this->win_streak > 1) {
					// Increase the game's difficulty some
					this->difficulty += 0.005 * this->getChallengeLevel() * (this->win_streak - 1);
				}
				else if (this->lose_streak > 0) {
					// Decrease the game's difficulty
					this->difficulty -= 0.015 * this->getChallengeLevel() * this->lose_streak
						+ this->level * 0.001;
				}
				// Else do nothing
			}
		private:
			/// <summary>Shared pointer to the device resources.</summary>
			std::shared_ptr<graphics::DX::DeviceResources2D> device_resources;
			/// <summary>The game map being used by the program.</summary>
			std::shared_ptr<GameMap> map {nullptr};
			/// <summary>The list of enemy template types.</summary>
			std::map<std::wstring, std::shared_ptr<game::EnemyType>> enemy_types;
			/// <summary>The list of enemies that are currently alive.</summary>
			std::vector<std::unique_ptr<game::Enemy>> enemies;
			/// <summary>The list of shot template types.</summary>
			std::map<std::wstring, std::shared_ptr<game::ShotBaseType>> shot_types;
			/// <summary>The list of projectiles that are currently active.</summary>
			std::vector<std::unique_ptr<game::Shot>> shots;
			/// <summary>The list of tower template types.</summary>
			std::vector<std::shared_ptr<game::TowerType>> tower_types;
			/// <summary>The list of towers currently in the game.</summary>
			std::vector<std::unique_ptr<game::Tower>> towers;
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
			int challenge_level;
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
			// Testing things
			std::shared_ptr<pathfinding::Pathfinder> ground_test_pf {nullptr};
			std::shared_ptr<pathfinding::Pathfinder> air_test_pf {nullptr};
			bool show_test_paths {false};
		};
	}
}