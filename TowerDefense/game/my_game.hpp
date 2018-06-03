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

		/// <summary>Class that represents an instance of the game itself.</summary>
		class MyGame {
			friend class graphics::Renderer2D;
		public:
			MyGame(std::shared_ptr<graphics::DX::DeviceResources2D> dev_res,
				std::wistream& ground_terrain_file, std::wistream& air_terrain_file);
			~MyGame() noexcept;
			/// <summary>Resets the game's state.</summary>
			void resetState() {
				// @TODO: Define
			}
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
			std::vector<std::unique_ptr<Enemy>>& getEnemies() noexcept {
				return this->enemies;
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
			/// <summary>The list of tower template types.</summary>
			std::vector<std::shared_ptr<game::TowerType>> tower_types;
			// Testing things
			std::shared_ptr<pathfinding::Pathfinder> ground_test_pf {nullptr};
			std::shared_ptr<pathfinding::Pathfinder> air_test_pf {nullptr};
		};
	}
}