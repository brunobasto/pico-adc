/**
 * Unit Test Main
 */

#include <stdio.h>
#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char** av) {
	printf("RUNNING TESTS\n");

	return CommandLineTestRunner::RunAllTests(ac, av);
}
