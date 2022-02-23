#pragma once
// File Author: Isaiah Hoffman
// File Created: April 14, 2018
#include <random>

namespace hoffman_isaiah {
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

		/// <summary>Represents a normally distributed random variable.</summary>
		class NormalRandomVariable {
		public:
			/// <param name="mu">The mean of the random variable.</param>
			/// <param name="sigma">The standard deviation of the random variable.</param>
			NormalRandomVariable(double mu, double sigma) :
				mean {mu},
				standard_deviation {sigma},
				distro_normal {mu, sigma} {
			}

			/// <returns>A random normally distributed value as specified by this class's data.</returns>
			double operator()() const noexcept {
				return this->distro_normal(rng::gen);
			}
		private:
			/// <summary>The mean of the random variable.</summary>
			double mean;
			/// <summary>The standard deviation of the random variable.</summary>
			double standard_deviation;
			/// <summary>The normal distribution associated with the random variable.</summary>
			mutable std::normal_distribution<double> distro_normal;
		};
	}
}