#include <gtest\gtest.h>
#include <ctime>
#include <random>

int main(int argc, char *argv[]){
	srand((unsigned int)time(nullptr));

	::testing::InitGoogleTest(&argc, argv);
	int error = RUN_ALL_TESTS();

	std::getchar();
	return error;
}