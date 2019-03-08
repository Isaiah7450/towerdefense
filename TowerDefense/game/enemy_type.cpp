// Author: Isaiah Hoffman
// Created: March 8, 2019
#include <vector>
#include "./enemy_type.hpp"
#include "./my_game.hpp"
namespace hoffman::isaiah::game {

	double BuffBase::getAverageInfluenceRating() const noexcept {
		const auto etnames = this->getTargetNames();
		double total_rating = 0.0;
		for (const auto& ename : etnames) {
			// Maybe not the wisest use of global state but definitely convenient.
			total_rating += game::g_my_game->getEnemyType(ename)->getBaseRating();
		}
		return total_rating / static_cast<double>(etnames.size());
	}
}
