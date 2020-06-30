all:
	g++-10 -std=c++20 main.cpp -O2 -Wall -Wextra -Wpedantic -Wfatal-errors -I. -o fizzbuzz
