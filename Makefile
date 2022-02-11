all:
	g++ -std=c++20 -pthread -o main main.cpp teams.cpp err.c
	g++ -std=c++20 -pthread -o new_process new_process.cpp err.c
