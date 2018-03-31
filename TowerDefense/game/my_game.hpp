#pragma once
// File Author: Isaiah Hoffman
// File Created: March 20, 2018
#include <string>
#include <memory>
#include <iosfwd>
#include "./../globals.hpp"

namespace hoffman::isaiah {
	namespace game {
		// Forward declarations
		class GameMap;
		// Global variables
		/// <summary>Global pointer to the my_game instance.</summary>
		extern std::shared_ptr<MyGame> g_my_game;


		/// <summary>Class that represents an instance of the game itself.</summary>
		class MyGame {
			friend class graphics::Renderer2D;
		public:
			MyGame(std::wistream& ground_terrain_file, std::wistream& air_terrain_file);
			/// <summary>Updates the state of the game by one tick.</summary>
			void update();
			// Getters
			GameMap& getMap() noexcept {
				return *this->map;
			}
			const GameMap& getMap() const noexcept {
				return *this->map;
			}
		private:
			/// <summary>The game map being used by the program.</summary>
			std::shared_ptr<GameMap> map {nullptr};
		};
	}
}