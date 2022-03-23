// File Author: Isaiah Hoffman
// File Created: April 14, 2018
#include <random>
#include "./game_util.hpp"
namespace hoffman_isaiah {
	namespace game {
		namespace rng {
			thread_local std::random_device rd {};
#ifdef _WIN64
			thread_local std::mt19937_64 gen {rd()};
#else
			thread_local std::mt19937 gen {rd()};
#endif
			thread_local std::uniform_real_distribution<double> distro_uniform {};
		}
	}
}