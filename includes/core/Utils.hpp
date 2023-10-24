
#pragma once

#include <string>
#include <iostream>

class Utils {

private:
	Utils();

public:
	static void printRed(const std::string& message);
	static void printGreen(const std::string& message);
	static void printYellow(const std::string& message);
};