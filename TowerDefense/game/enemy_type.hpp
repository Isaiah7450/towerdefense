#pragma once
// File Author: Isaiah Hoffman
// File Created: April 18, 2018
#include <string>
#include "./../globals.hpp"
#include "./game_object_type.hpp"

namespace hoffman::isaiah {
	namespace game {
		/// <summary>Template type used to create new enemies.</summary>
		class EnemyType : public GameObjectType {
		public:
		private:
			/// <summary>The amount of damage that the enemy can withstand before perishing.</summary>
			double base_health;
			/// <summary>The amount of damage that the enemy's armor can withstand before breaking.</summary>
			double base_armor_hp;
			/// <summary>The percentage of damage that the enemy's armor blocks.</summary>
			double armor_reduce;
			/// <summary>The percentage of damage (to the enemy's health) can receive before being
			/// considered 'injured'.</summary>
			double pain_tolerance;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while wearing armor.</summary>
			double walking_speed;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while not wearing armor.</summary>
			double running_speed;
			/// <summary>The speed in game coordinate squares per second that the enemy moves while injured.</summary>
			double injured_speed;
			/// <summary>The pathfinding heuristic the enemy uses.</summary>
			pathfinding::HeuristicStrategies default_strategy;
			/// <summary>Can the enemy move diagonally?</summary>
			bool move_diag;
			/// <summary>Is the enemy a flying enemy?</summary>
			bool flying;
			// Buff Parameters --> To do later!

		};
	}
}