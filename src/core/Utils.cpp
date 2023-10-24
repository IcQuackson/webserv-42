#include "core/Utils.hpp"

Utils::Utils() {}

void Utils::printRed(const std::string& message) {
	std::cout << "\033[1;31m" << message << "\033[0m" << std::endl;
}

void Utils::printGreen(const std::string& message) {
	std::cout << "\033[1;32m" << message << "\033[0m" << std::endl;
}

void Utils::printYellow(const std::string& message) {
	std::cout << "\033[1;33m" << message << "\033[0m" << std::endl;
}