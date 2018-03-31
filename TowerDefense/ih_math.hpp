#pragma once
// File Author: Isaiah Hoffman
// File Created: March 26, 2018

namespace hoffman::isaiah {
	namespace math {
		// Constexpr functions
		// Returns the number of microseconds in a second.
		constexpr static long long getMicrosecondsInSecond() {
			return 1000000ll;
		}
		// Practically all of these are defined in the standard library; however,
		// since most of them predate C++11 and constexpr, I have decided to redefine
		// a constexpr version of them here.
		// Returns the smaller of a and b. If they are equal, a is returned.
		template <typename T>
		constexpr T get_min(T a, T b) noexcept {
			return a <= b ? a : b;
		}
		// Returns the larger of a and b. If they are equal, a is returned.
		template <typename T>
		constexpr T get_max(T a, T b) noexcept {
			return a >= b ? a : b;
		}
		// Obtains the absolute value of a number
		template <typename T>
		constexpr T get_abs(T val) noexcept {
			return val >= 0 ? val : -val;
		}
		// Obtains the real square root of a number to a certain number of decimal places
		// given by min_delta (which should be a multiple of 10.) (Negative values will result
		// in zero being returned.)
		template <typename T, typename U = double>
		constexpr U get_sqrt(T val, U min_delta = 1e-10) noexcept {
			U ret_value = 0;
			U delta = U {1e10};
			for (; delta >= min_delta; delta /= U {10}) {
				for (int digit_value = 0; digit_value < 11; ++digit_value) {
					const auto cur_total = ret_value + (delta * digit_value);
					const auto cur_total_sq = cur_total * cur_total;
					if (cur_total_sq == static_cast<U>(val)) {
						return cur_total;
					}
					else if (cur_total_sq > static_cast<U>(val)) {
						ret_value += delta * (digit_value - 1);
						break;
					}
				}
			}
			// Round result
			if (val - ret_value * ret_value > delta * 5) {
				ret_value += delta * U {10};
			}
			return ret_value;
		}
		// Computes n factorial where the factorial function
		// is defined recursively as n(n-1)! or alternatively
		// as n(n-1)(n-2)...(3)(2)(1). Also note that 0! = 1.
		template <typename T = int>
		constexpr T calculate_factorial(T n) noexcept {
			T ret_value = 1;
			for (T loop_counter = n; loop_counter > 0; --loop_counter) {
				ret_value *= loop_counter;
			}
			return ret_value;
		}
#if 0
		// Attempts to calculate the value of PI by using a certain number of iterations
		// Iterations should be an integer value.
		template <typename T = double>
		constexpr T calculate_pi(T iterations = 500000) noexcept {
			// Use an infinite series to approximate
			T ret_value = T {3};
			for (T it_no = 0; it_no < iterations; ++it_no) {
				const T start_denom = 2 * it_no + 2;
				const T denom = start_denom * (start_denom + 1) * (start_denom + 2);
				const T it_value = T {4} / denom;
				if (static_cast<int>(it_no) % 2 == 0) {
					ret_value += it_value;
				}
				else {
					ret_value -= it_value;
				}
			}
			return ret_value;
		}
		constexpr const auto pi = calculate_pi();
#else
		// Honestly, hardcoding this may be the best method
		template <typename T = double>
		constexpr T calculate_pi() noexcept {
			return T {3.1415926535897932384626433832795};
		}
		constexpr const auto pi = calculate_pi();
#endif

		// Attempts to calculate the value of e by employing an infinite series
		// which converges way faster than using the limit definition of e.
		// Adjust parameter x if one wants better precision.
		template <typename T = double>
		constexpr T calculate_e(T x = 18) noexcept {
			// Calculate e using an infinite series
			T ret_value = 0;
			for (T it_no = 0; it_no < x; ++it_no) {
				ret_value += T {1} / calculate_factorial(it_no);
			}
			return ret_value;
		}
		constexpr const auto e = calculate_e();
	}
}