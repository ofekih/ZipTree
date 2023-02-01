#include "ZipTree.h"
#include "ZigZagZipTree.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <vector>
#include <stdio.h>

static const std::string DATA_FILE_DIRECTORY = "data/";


// static const std::string DATA_FILE_PATH = "data/zigzag-true/n-ns-min-med-max-height.csv";

// void save_data(unsigned n, size_t ns, unsigned min, unsigned med, unsigned max, unsigned height)
// {
// 	std::ofstream data_file(DATA_FILE_PATH, std::ios::app);
// 	data_file << n << "," << ns << "," << min << "," << med << "," << max << "," << height << std::endl;
// }

// void run_experiment(unsigned n)
// {
// 	ZigZagZipTree<unsigned, bool> tree;

// 	auto start = std::chrono::high_resolution_clock::now();
// 	for (unsigned i = 0; i < n; ++i)
// 	{
// 		tree.insert(i, false);
// 	}

// 	unsigned height = tree.getHeight();
// 	unsigned min_val_depth = tree.getDepth(0);
// 	unsigned med_val_depth = tree.getDepth(n / 2);
// 	unsigned max_val_depth = tree.getDepth(n - 1);

// 	auto end = std::chrono::high_resolution_clock::now();
// 	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

// 	save_data(n, elapsed.count(), min_val_depth, med_val_depth, max_val_depth, height);
// }

static const std::string DATA_FILE_PATH = "data/zigzag/n-ns-depths.csv";
// static const std::string ODATA_FILE_PATH = "data/original/n-ns-depths-avg.csv";

void save_data(unsigned n, size_t ns, const std::vector<unsigned>& depths)
{
	std::ofstream data_file(DATA_FILE_PATH, std::ios::app);
	data_file << n << "," << ns;
	for (auto depth : depths)
	{
		data_file << "," << depth;
	}
	data_file << "\n";
}

void run_experiment(unsigned n)
{
	ZigZagZipTree<unsigned, bool> tree;

	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < n; ++i)
	{
		tree.insert(i, false);
	}

	std::vector<unsigned> depths;

	for (unsigned i = 0; i < n; ++i)
	{
		depths.push_back(tree.getDepth(i));
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	save_data(n, elapsed.count(), depths);
}

void run_experiments(unsigned num_trials)
{
	for (unsigned n = 4194304; ; n *= 2)
	{
		auto start = std::chrono::high_resolution_clock::now();

		for (unsigned i = 0; i < num_trials; ++i)
		{
			run_experiment(n);
		}

		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "n = " << n << " took " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
		break;
	}
}

const std::vector<unsigned> read_csv_line(const std::string& line)
{
	std::istringstream iss(line);
	std::string token;
	std::vector<unsigned> depths;
	while (std::getline(iss, token, ','))
	{
		depths.push_back(std::stoi(token));
	}
	return depths;
}


void average_data(const std::string& input, const std::string& output, unsigned n)
{
	std::ifstream data_file(input);

	std::vector<unsigned> total_depths(n, 0);
	unsigned num_trials = 0;

	std::string line;
	while (std::getline(data_file, line))
	{
		const auto& depths = read_csv_line(line);

		if (depths.size() != n + 2) {
			// std::cout << "ERROR: depths.size() = " << depths.size() << " != " << n + 2 << std::endl;
			
			// // print them all out
			// for (auto depth : depths)
			// {
			// 	std::cout << depth << ",";
			// }
			// std::cout << std::endl;
			
			// exit(1);
			continue;
		}

		for (unsigned i = 0; i < n; ++i)
		{
			total_depths[i] += depths[i + 2];
		}

		++num_trials;

		if (num_trials % 100 == 0)
		{
			std::cout << "num_trials = " << num_trials << std::endl;
		}
	}

	// open file to append
	std::ofstream data_file_out(output, std::ios::app);

	data_file_out << n << "," << num_trials;

	for (unsigned i = 0; i < n; ++i)
	{
		data_file_out << "," << static_cast<double>(total_depths[i]) / num_trials;
	}

	data_file_out << "\n";
}


int main()
{
	average_data("data/zigzag/n-ns-depths.csv", "data/zigzag/n-ns-depths-avg.csv", 4194304);

	// run_experiments(5000);

	return 0;
}

