#include "BinarySearchTree.h"
#include "ZipTree.h"
// #include "ZigZagZipTree.h"
#include "ZipZipTree.h"
#include "Treap.h"

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

static const std::string DATA_FILE_DIRECTORY = "data/";
static const std::string NORMAL_FILE_NAME = "n-ns-min-med-max-height-avg.csv";
// total comparisons, first tie, both ties
static const std::string ZIPZIP_NORMAL_FILE_NAME = "n-ns-min-med-max-height-tc-ft-bt.csv";
static const std::string DEPTH_FILE_NAME = "n-ns-depths.csv";


// create unordered map of BinarySearchTree types

static const std::unordered_map<std::string, std::function<std::unique_ptr<BinarySearchTree<unsigned>>(unsigned n)>> BST_MAP = {
	// {"zigzag", [](unsigned n) { return std::make_unique<ZigZagZipTree<unsigned>>(n); }},
	{"original", [](unsigned n) { return std::make_unique<ZipTree<unsigned>>(n); }},
	{"treap", [](unsigned n) { return std::make_unique<Treap<unsigned>>(n); }},
	{"zipzip", [](unsigned n) { return std::make_unique<ZipZipTree<unsigned>>(n); }}
};


void save_normal_data(const std::string& ziptree_type, unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, double avg)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + NORMAL_FILE_NAME, std::ios::app);
	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << avg << std::endl;
}

void save_zipzip_normal_data(const std::string& ziptree_type, unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height, uint64_t tc, uint64_t ft, uint64_t bt)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + ZIPZIP_NORMAL_FILE_NAME, std::ios::app);
	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << "," << tc << "," << ft << "," << bt << std::endl;
}

void run_zipzip_experiment(const std::string& ziptree_type, unsigned n)
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
	uint64_t total_comparisons = tree->getTotalComparisons();
	uint64_t first_tie = tree->getFirstTies();
	uint64_t both_tie = tree->getBothTies();

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_zipzip_normal_data(ziptree_type, n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, total_comparisons, first_tie, both_tie);
}

void run_normal_experiment(const std::string& ziptree_type, unsigned n)
{
	// ZipZipTree<unsigned> tree(n);
	// ZipTree<unsigned> tree(n);

	auto tree = BST_MAP.at(ziptree_type)(n);

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

	unsigned height = tree->getHeight();
	unsigned min_val_depth = tree->getDepth(0);
	unsigned med_val_depth = tree->getDepth(n / 2);
	unsigned max_val_depth = tree->getDepth(n - 1);
	double average_height = tree->getAverageHeight();

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_normal_data(ziptree_type, n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height, average_height);
}

void save_depth_data(const std::string& ziptree_type, unsigned n, size_t ns, const std::vector<unsigned>& depths)
{
	std::ofstream data_file(DATA_FILE_DIRECTORY + ziptree_type + "/" + DEPTH_FILE_NAME, std::ios::app);
	data_file << n << "," << ns;
	for (auto depth : depths)
	{
		data_file << "," << depth;
	}
	data_file << std::endl;
}

void run_depth_experiment(const std::string& ziptree_type, unsigned n)
{
	auto tree = BST_MAP.at(ziptree_type)(n);

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < n; ++i)
	{
		tree->insert(i);
	}

	std::vector<unsigned> depths;
	depths.reserve(n);

	for (unsigned i = 0; i < n; ++i)
	{
		depths[i] = tree->getDepth(i);
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_depth_data(ziptree_type, n, elapsed.count(), depths);
}

void run_experiments(unsigned num_trials, unsigned min_n, unsigned max_n)
{
	for (unsigned n = min_n; n <= max_n; n *= 2)
	{
		auto start = std::chrono::high_resolution_clock::now();

		for (auto& [ziptree_type, tree] : BST_MAP)
		{
			for (unsigned i = 0; i < num_trials; ++i)
			{
				// run_zipzip_experiment(ziptree_type, n);
				// run_zipzip_experiment("original", n);
				// run_zipzip_experiment("treap", n);
				run_normal_experiment(ziptree_type, n);
				// run_depth_experiment(ziptree_type, n);
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "n = " << n << " took " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
	}
}


int main()
{
	// run_experiments(1, 16, 268435456);
	// run_experiments(10000, 268435456 / 256, 268435456);
	run_experiments(1000, 2, 268435456);

	return 0;
}
