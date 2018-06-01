#pragma once
// File Author: Isaiah Hoffman
// File Created: April 14, 2018
#include <random>

namespace hoffman::isaiah {
	namespace game {
		namespace rng {
			/// <summary>A non-deterministic random number generator used to seed another random number generator.</summary>
			extern thread_local std::random_device rd;
			/// <summary>Deterministic random number generator.</summary>
#ifdef _WIN64
			extern thread_local std::mt19937_64 gen;
#else
			extern thread_local std::mt19937 gen;
#endif
			/// <summary>An integer distribution that gives values from 1 to 100 uniformly.</summary>
			extern thread_local std::uniform_real_distribution<double> distro_uniform;
		}
	}
}