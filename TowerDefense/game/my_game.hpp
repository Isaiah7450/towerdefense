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
			// Getters
			double getMoney() const noexcept {
				return this->money;
			}
			int getHealth() const noexcept {
				return this->health;
			}
			bool isAlive() const noexcept {
				return this->getHealth() > 0;
			}
			// Setters
			void changeMoney(double amt) noexcept {
				this->money += amt;
			}
			void changeHealth(int amt) noexcept {
				this->health += amt;
			}
		private:
			/// <summary>The amount of money the player possesses.</summary>
			double money {100.0};
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
			void debug_update(DebugUpdateStates cause);
			// Note: Defined in data_loading.cpp
			/// <summary>Initializes the list of enemy types in this game.</summary>
			void init_enemy_types();
			/// <summary>Initializes the list of shot types in this game.</summary>
			void init_shot_types();
			/// <summary>Initializes the list of tower types in this game.</summary>
			void init_tower_types();
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
			/// <param name="selection">The tower that was selected.</param>
			void selectTower(int selection) {
				this->selected_tower = selection;
			}
			void buyTower(int gx, int gy);
			void sellTower(int gx, int gy);
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
			int getSelectedTower() const noexcept {
				return this->selected_tower;
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
			/// <summary>The current level the player is on.</summary>
			int level {1};
			/// <summary>The current dynamic difficulty level the player is at.</summary>
			double difficulty {1.00};
			/// <summary>The current game difficulty level the player is at.</summary>
			int challenge_level;
			/// <summary>Is the game currently paused?</summary>
			bool is_paused {false};
			/// <summary>Is a level currently in progress?</summary>
			bool in_level {false};
			/// <summary>The currently selected tower.</summary>
			int selected_tower {-1};
			// Testing things
			std::shared_ptr<pathfinding::Pathfinder> ground_test_pf {nullptr};
			std::shared_ptr<pathfinding::Pathfinder> air_test_pf {nullptr};
		};
	}
}