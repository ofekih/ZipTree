#include "BinarySearchTree.h"
// #include "ZipTree.h"
// // #include "ZigZagZipTree.h"
// #include "ZipZipTree.h"
// #include "Treap.h"

#include "ZipTree2.h"
#include "UniformZipTree2.h"
#include "ZipZipTree2.h"
#include "DynamicZipTree2.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <vector>
#include <stdio.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <utility>

static const std::string DATA_FILE_DIRECTORY = "datav2/";
static const std::string NORMAL_FILE_NAME = "n-ns-min-med-max-height-avg-";
static const std::string SQRT_FILE_NAME = "n-ns-min-med-max-height-sqrt-";
// total comparisons, first tie, both ties
static const std::string COMPARISON_NORMAL_FILE_NAME = "n-ns-min-med-max-height-avg-tc-ft-bt.csv";
static const std::string DYNAMIC_FILE_NAME = "n-ns-min-med-max-height-avg-tc-ft-bt-mb-ab.csv";
static const std::string DEPTH_FILE_NAME = "n-ns-depths.csv";


// create unordered map of BinarySearchTree types

// static const std::unordered_map<std::string, std::function<std::unique_ptr<BinarySearchTree<unsigned>>(unsigned n)>> BST_MAP = {
// 	// {"zigzag", [](unsigned n) { return std::make_unique<ZigZagZipTree<unsigned>>(n); }},
// 	{"original", [](unsigned n) { return std::make_unique<ZipTree<unsigned>>(n); }},
// 	{"treap", [](unsigned n) { return std::make_unique<Treap<unsigned>>(n); }},
// 	{"zipzip", [](unsigned n) { return std::make_unique<ZipZipTree<unsigned>>(n); }}
// };

static const std::unordered_map<std::string, std::function<std::unique_ptr<BinarySearchTree<unsigned>>(unsigned n)>> BST_MAP = {
	{"original", [](unsigned n) { return std::make_unique<ZipTree<unsigned>>(n); }},
	{"uniform", [](unsigned n) { return std::make_unique<UniformZipTree<unsigned>>(n); }},
	{"zipzip", [](unsigned n) { return std::make_unique<ZipZipTree<unsigned>>(n); }}
};


void save_normal_data(const std::string& ziptree_type, const std::string& computer_name, unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, double avg)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + NORMAL_FILE_NAME + computer_name + ".csv", std::ios::app);
	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << avg << ",2.0" << std::endl;
}

// void save_sqrt_data(const std::string& ziptree_type, const std::string& computer_name, unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, unsigned sqrt)
// {
// 	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + SQRT_FILE_NAME + computer_name + ".csv", std::ios::app);
// 	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << sqrt << ",2.0" << std::endl;
// }

void save_comparison_normal_data(const std::string& ziptree_type, unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, double avg, uint64_t tc, uint64_t ft, uint64_t bt)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + COMPARISON_NORMAL_FILE_NAME, std::ios::app);
	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << avg << "," << tc << "," << ft << "," << bt << std::endl;
}

void save_dynamic_data(unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, double avg, uint64_t tc, uint64_t ft, uint64_t bt, unsigned mb, double ab)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + "dynamic/" + DYNAMIC_FILE_NAME, std::ios::app);
	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << avg << "," << tc << "," << ft << "," << bt << "," << mb << "," << ab << std::endl;
}

void run_comparison_experiment(const std::string& ziptree_type, unsigned n)
{
	auto tree = BST_MAP.at(ziptree_type)(n);

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < n; ++i)
	{
		tree->insert(i);
	}

	unsigned height = tree->getHeight();
	unsigned min_val_depth = tree->getDepth(0);
	unsigned med_val_depth = tree->getDepth(n / 2);
	unsigned max_val_depth = tree->getDepth(n - 1);
	double average_height = tree->getAverageHeight();
	uint64_t total_comparisons = tree->getTotalComparisons();
	uint64_t first_tie = tree->getFirstTies();
	uint64_t both_tie = tree->getBothTies();

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_comparison_normal_data(ziptree_type, n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, average_height, total_comparisons, first_tie, both_tie);
}

void run_dynamic_experiment(unsigned n)
{
	DynamicZipTree tree = DynamicZipTree<unsigned>(n);

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < n; ++i)
	{
		tree.insert(i);
	}

	unsigned height = tree.getHeight();
	unsigned min_val_depth = tree.getDepth(0);
	unsigned med_val_depth = tree.getDepth(n / 2);
	unsigned max_val_depth = tree.getDepth(n - 1);
	double average_height = tree.getAverageHeight();
	uint64_t total_comparisons = tree.getTotalComparisons();
	uint64_t first_tie = tree.getFirstTies();
	uint64_t both_tie = tree.getBothTies();
	unsigned max_bits = tree.getMaxBits();
	double average_bits = static_cast<double>(tree.getTotalBits()) / n;

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_dynamic_data(n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, average_height, total_comparisons, first_tie, both_tie, max_bits, average_bits);
}

void run_normal_experiment(const std::string& ziptree_type, unsigned n, const std::string& computer_name)
{
	// ZipZipTree<unsigned> tree(n);
	// ZipTree<unsigned> tree(n);

	auto tree = BST_MAP.at(ziptree_type)(n);

	// auto tree = ZipTree<unsigned>(n);

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < n; ++i)
	{
		// tree.insert(i);
		tree->insert(i);
	}

	// unsigned height = tree.getHeight();
	// unsigned min_val_depth = tree.getDepth(0);
	// unsigned med_val_depth = tree.getDepth(n / 2);
	// unsigned max_val_depth = tree.getDepth(n - 1);
	// double average_height = tree.getAverageHeight();


	unsigned height = tree->getHeight();
	unsigned min_val_depth = tree->getDepth(0);
	unsigned med_val_depth = tree->getDepth(n / 2);
	unsigned max_val_depth = tree->getDepth(n - 1);
	double average_height = tree->getAverageHeight();

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_normal_data(ziptree_type, computer_name, n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, average_height);
}

// void run_sqrt_experiment(const std::string& ziptree_type, unsigned sqrtn, const std::string& computer_name)
// {
// 	unsigned n = sqrtn * sqrtn;

// 	auto tree = BST_MAP.at(ziptree_type)(n);

// 	auto start = std::chrono::high_resolution_clock::now();
// 	for (unsigned i = 0; i < n; ++i)
// 	{
// 		tree->insert(i);
// 	}

// 	unsigned height = tree->getHeight();
// 	unsigned min_val_depth = tree->getDepth(0);
// 	unsigned med_val_depth = tree->getDepth(n / 2);
// 	unsigned max_val_depth = tree->getDepth(n - 1);
// 	unsigned sqrt_height = tree->getDepth(sqrtn - 1);

// 	auto end = std::chrono::high_resolution_clock::now();
// 	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

// 	save_sqrt_data(ziptree_type, computer_name, n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, sqrt_height);
// }


// void save_depth_data(const std::string& ziptree_type, unsigned n, size_t ns, const std::vector<unsigned>& depths)
// {
// 	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + DEPTH_FILE_NAME, std::ios::app);
// 	data_file << n << "," << ns;
// 	for (auto depth : depths)
// 	{
// 		data_file << "," << depth;
// 	}
// 	data_file << std::endl;
// }

// void run_depth_experiment(const std::string& ziptree_type, unsigned n)
// {
// 	auto tree = BST_MAP.at(ziptree_type)(n);

// 	auto start = std::chrono::high_resolution_clock::now();
// 	for (unsigned i = 0; i < n; ++i)
// 	{
// 		tree->insert(i);
// 	}

// 	std::vector<unsigned> depths;
// 	depths.reserve(n);

// 	for (unsigned i = 0; i < n; ++i)
// 	{
// 		depths[i] = tree->getDepth(i);
// 	}

// 	auto end = std::chrono::high_resolution_clock::now();
// 	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

// 	save_depth_data(ziptree_type, n, elapsed.count(), depths);
// }

void run_experiments(unsigned num_trials, unsigned min_n, unsigned max_n, const std::string& computer_name)
{
	for (unsigned n = min_n; n <= max_n; n *= 2)
	{
		auto start = std::chrono::high_resolution_clock::now();

		for (unsigned i = 0; i < num_trials; ++i)
		{
			for (auto& [ziptree_type, tree] : BST_MAP)
			{
				// run_zipzip_experiment(ziptree_type, n);
				// run_zipzip_experiment("original", n);
				// run_zipzip_experiment("treap", n);
				// run_normal_experiment(ziptree_type, n, computer_name);
				// run_comparison_experiment(ziptree_type, n);
				run_dynamic_experiment(n);
				// run_depth_experiment(ziptree_type, n);
				// run_sqrt_experiment(ziptree_type, n, computer_name);
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << computer_name << ": n = " << n << " took " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
	}
}


int main(int argc, char *argv[])
{
	// ZipTree<unsigned> tree(5);

	// tree.insert(0);

	// std::cout << "Depth (0): " << tree.getDepth(0) << std::endl;


	// tree.insert(1);

	// run_experiments(1, 16, 268435456);
	// run_experiments(10000, 2, 1024 * 8, "");
	// run_experiments(1, 33554432, 268435456, "");8192 * 2
	// run_experiments(10000, 1073741824 / 4, 536870912 * 2, std::string(argv[1]));
	run_experiments(10000, 2, 536870912 * 2, std::string(argv[1]));
	// run_experiments(1000000, 2, 1024, std::string(argv[1]));

	return 0;
}
