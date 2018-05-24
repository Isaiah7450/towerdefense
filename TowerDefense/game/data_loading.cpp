// File Author: Isaiah Hoffman
// File Created: May 24, 2018
#include <iostream>
#include <string>
#include "./../file_util.hpp"
#include "./../globals.hpp"
#include "./enemy_type.hpp"
using namespace std::literals::string_literals;

namespace hoffman::isaiah {
	namespace game {
		std::wostream& operator<<(std::wostream& os, const EnemyType& data) {
			UNREFERENCED_PARAMETER(data);
			return os;
		}

		std::wistream& operator>>(std::wistream& is, const EnemyType& data) {
			UNREFERENCED_PARAMETER(data);
			return is;
		}

		std::wostream& operator<<(std::wostream& os, const BuffBase& data) {
			UNREFERENCED_PARAMETER(data);
			return os;
		}

		std::wistream& operator>>(std::wistream& is, const BuffBase& data) {
			UNREFERENCED_PARAMETER(data);
			return is;
		}
	}
}