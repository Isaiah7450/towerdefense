#pragma once
// File Author: Isaiah Hoffman
// File Created: March 26, 2018
#include "./globals.hpp"

namespace hoffman::isaiah {
	namespace math {
		// Constexpr functions
		// Returns the number of microseconds in a second.
		constexpr static long long getMicrosecondsInSecond() {
			return 1000000ll;
		}
		// Returns the number of milliseconds in a single logical frame.
		constexpr double get_milliseconds_per_frame() noexcept {
			return 1000.0 / game::logic_framerate;
		}
		// Converts a time in milliseconds into a number of logical frames
		// that represent the same time span.
		// time_in_ms : The time in milliseconds to convert.
		constexpr double convertMillisecondsToFrames(double time_in_ms) noexcept {
			return time_in_ms / math::get_milliseconds_per_frame();
		}

		// Practically all of these are defined in the standard library; however,
		// since most of them predate C++11 and constexpr, I have decided to redefine
		// a constexpr version of them here.
		// Returns the smallest of the provided arguments.
		template <typename U, typename... Arguments>
		constexpr U get_min(U first, Arguments... rest) noexcept {
			U lowest = first;
			U args[] {rest...};
			for (auto arg : args) {
				lowest = static_cast<U>(arg) < lowest ? static_cast<U>(arg) : lowest;
			}
			return lowest;
		}
		// Returns the largest of the provided arguments.
		template <typename U, typename... Arguments>
		constexpr U get_max(U first, Arguments... rest) noexcept {
			U highest = first;
			U args[] {rest...};
			for (auto arg : args) {
				highest = static_cast<U>(arg) > highest ? static_cast<U>(arg) : highest;
			}
			return highest;
		}
		// Obtains the absolute value of a number
		template <typename T>
		constexpr T get_abs(T val) noexcept {
			return val >= 0 ? val : -val;
		}
		// Obtains the sum of a set of values
		template <typename U, typename... Arguments>
		constexpr U get_sum(U first, Arguments... rest) noexcept {
			U total = first;
			U args[] {rest...};
			for (auto arg : args) {
				total += static_cast<U>(arg);
			}
			return total;
		}
		// Obtains the average of a set of values
		template <typename U, typename... Arguments>
		constexpr U get_avg(U first, Arguments... rest) noexcept {
			const U num_args = static_cast<U>(sizeof...(Arguments));
			U my_sum = get_sum<U>(first, rest...);
			return my_sum / (num_args + U {1});
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
			return static_cast<T>(3.1415926535897932384626433832795);
		}
		constexpr const auto pi = calculate_pi();
#endif
		// Converts an angle "theta" to degrees from radians
		template <typename U = double, typename T = double>
		constexpr U convert_to_degrees(T theta) noexcept {
			return static_cast<U>(theta) * static_cast<U>(180) / calculate_pi<U>();
		}
		// Converts an angle from degrees to radians
		template <typename U = double, typename T = double>
		constexpr U convert_to_radians(T angle) noexcept {
			return static_cast<U>(angle) / static_cast<U>(180) * calculate_pi<U>();
		}

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
		// Raises a number to an integer power
		template <typename T, typename U = double>
		constexpr U get_pow(T base, int exp) noexcept {
			U ret_value = static_cast<U>(base);
			int abs_exp = math::get_abs(exp);
			if (exp == 0) {
				return U {1};
			}
			for (int i = 1; i < abs_exp; ++i) {
				ret_value *= static_cast<U>(base);
			}
			return abs_exp > exp ? U {1} / ret_value : ret_value;
		}
	}
}