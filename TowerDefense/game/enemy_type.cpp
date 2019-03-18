// Author: Isaiah Hoffman
// Created: March 8, 2019
#include <vector>
#include "./enemy_type.hpp"
#include "./game_formulas.hpp"
#include "./my_game.hpp"
namespace hoffman::isaiah::game {

	double BuffBase::getAverageInfluenceRating() const noexcept {
		// Maybe not the wisest use of global state but definitely convenient.
		return enemy_buffs::buff_base::getAverageInfluenceRating(this->getTargetNames(), game::g_my_game->getAllEnemyTypes());
	}
}
