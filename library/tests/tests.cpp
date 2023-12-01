// Main file to create all tests
//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

int main(int argc, char* argv[])
{
	return Catch::Session().run(argc, argv);
}
