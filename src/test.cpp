#include "ZipTree.h"
#include "first_fit.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>

typedef struct ProblemInstance {
	std::vector<double> items;
	std::vector<int> assignments;
	std::vector<double> free_space;
} ProblemInstance;

typedef void (*algorithm) (const std::vector<double>&, std::vector<int>&, std::vector<double>&);

bool compare(const std::vector<double>& v1, const std::vector<double>& v2)
{
	if (v1.size() != v2.size())
	{
		return false;
	}

	// floating point comparison
	for (int i = 0; i<v1.size(); i++) {
		if (std::fabs(v1[i] - v2[i]) > 1e-6) {
			return false;
		}
	}
	return true;
}

bool testAlgorithm(ProblemInstance test, ProblemInstance expected_result, algorithm algo, std::string name)
{
	algo(test.items, test.assignments, test.free_space);
	return test.assignments == expected_result.assignments and compare(test.free_space, expected_result.free_space);
}

void assertTrue(bool b, const char * name)
{
	std::cout << (b ? "PASSED: " : "FAILED: ") << name << " ";
}

void testSampleBinPacking()
{
	std::vector<double> items {0.1, 0.8, 0.3, 0.5, 0.7, 0.2, 0.6, 0.4};
	std::vector<int> assignments(items.size(), 0);
	std::vector<double> free_space;
	ProblemInstance test1 = {items, assignments, free_space}, expected_result;

	// first_fit
	expected_result = {items, {1, 1, 2, 2, 3, 2, 4, 4}, {0.1, 0.0, 0.3, 0.0}};
	assertTrue(testAlgorithm(test1, expected_result, first_fit, "first_fit"), "testFF1");

	// first_fit_decreasing
	expected_result = {items, {1, 2, 3, 4, 3, 2, 1, 4}, {0.0, 0.0, 0.0, 0.4}};
	assertTrue(testAlgorithm(test1, expected_result, first_fit_decreasing, "first_fit_decreasing"), "testFFD1");


	// ----------------------------------test 2 ----------------------------------
	std::cout << std::endl;
	items = {0.79, 0.88, 0.95, 0.12, 0.05, 0.46, 0.53, 0.64, 0.04, 0.38, 0.03, 0.26};
	assignments = std::vector<int> (items.size(), 0);
	ProblemInstance test2 = {items, assignments, free_space};

	// first_fit
	expected_result = {items, {1, 2, 3, 1, 1, 4, 4, 5, 1, 6, 2, 5}, {0, 0.09, 0.05, 0.01, 0.1, 0.62}};
	assertTrue(testAlgorithm(test2, expected_result, first_fit, "first_fit"), "testFF2");

	// first_fit_decreasing
	expected_result = {items, {1, 2, 3, 4, 5, 5, 6, 4, 2, 1, 3, 3}, {0, 0, 0.14, 0.1, 0.01, 0.62}};
	assertTrue(testAlgorithm(test2, expected_result, first_fit_decreasing, "first_fit_decreasing"), "testFFD2");

	// ----------------------------------test 3 ----------------------------------
	std::cout << std::endl;
	items = {0.43, 0.75, 0.25, 0.42, 0.54, 0.03, 0.64};
	assignments = std::vector<int> (items.size(), 0);
	ProblemInstance test3 = {items, assignments, free_space};

	// first_fit
	expected_result = {items, {1, 2, 1, 3, 3, 1, 4}, {0.29, 0.25, 0.04, 0.36}};
	assertTrue(testAlgorithm(test3, expected_result, first_fit, "first_fit"), "testFF3");

	// first_fit_decreasing
	expected_result = {items, {1, 2, 3, 3, 4, 1, 2}, {0, 0.33, 0.03, 0.58}};
	assertTrue(testAlgorithm(test3, expected_result, first_fit_decreasing, "first_fit_decreasing"), "testFFD3");

	// ----------------------------------test 4----------------------------------
	std::cout << std::endl;
	items = {0.54, 0.67, 0.46, 0.57, 0.06, 0.23, 0.83, 0.64, 0.47, 0.03, 0.53, 0.74, 0.36, 0.24, 0.07, 0.25, 0.05, 0.63, 0.43, 0.04};
	assignments = std::vector<int> (items.size(), 0);
	ProblemInstance test4 = {items, assignments, free_space};

	// first_fit
	expected_result = {items, {1, 2, 1, 3, 2, 2, 4, 5, 6, 2, 6, 7, 3, 5, 3, 7, 4, 8, 9, 4}, {0, 0.01, 0, 0.08, 0.12, 0, 0.01, 0.37, 0.57}};
	assertTrue(testAlgorithm(test4, expected_result, first_fit, "first_fit"), "testFF4");

	// first_fit_decreasing
	expected_result = {items, {1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 4, 2, 3, 5, 1, 1, 3, 1, 3}, {0, 0.01, 0.01, 0, 0.14, 0, 0, 0}};
	assertTrue(testAlgorithm(test4, expected_result, first_fit_decreasing, "first_fit_decreasing"), "testFFD4");
}

int main()
{
	testSampleBinPacking();
	std::cout << std::endl;

	return 0;
}

