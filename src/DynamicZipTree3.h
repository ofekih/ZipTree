#ifndef DYNAMICZIPTREE2_H
#define DYNAMICZIPTREE2_H

#include "GeneralizedZipTree.h"

#include <array>
#include <limits>
#include <random>

#include <iostream>

constexpr std::array<uint8_t, 256> BITS_REQUIRED = []{
	std::array<uint8_t, 256> table{};
	for (size_t i = 0; i < table.size(); i++)
		for (uint8_t j = i; j > 0; j >>= 1)
			table[i]++;
	table[0] = 1;
	return table;
}();

struct GeometricDynamicUniformRank
{
	uint64_t grank;
	uint64_t urank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;
	uint64_t* bothTies;
	uint8_t num_bits = 0;

	inline void addBit() noexcept
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());

		static std::uniform_int_distribution<uint64_t> udistribution(0, std::numeric_limits<uint64_t>::max());

		static uint64_t random_number;
		static unsigned bit_index = sizeof(random_number) * 8;

		if (bit_index == sizeof(random_number) * 8)
		{
			random_number = udistribution(generator);
			bit_index = 0;
		}

		++num_bits;
		urank |= ((random_number >> bit_index) & 1) << (sizeof(urank) * 8 - num_bits);
		++bit_index;
	}

	inline int updateComparisons(GeometricDynamicUniformRank& other) noexcept
	{
		++(*totalComparisons);
		if (grank == other.grank)
		{

			while (num_bits < other.num_bits)
			{
				if (urank > other.urank)
					return 1;

				addBit();
			}

			while (other.num_bits < num_bits)
			{
				if (urank < other.urank)
					return -1;

				other.addBit();
			}

			++(*firstTies);
			while (urank == other.urank)
			{
				++(*bothTies);
				addBit();
				other.addBit();
			}

			return urank < other.urank ? -1 : 1;
		}

		return grank < other.grank ? -1 : 1;
	}

	bool operator<(GeometricDynamicUniformRank& other) noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(GeometricDynamicUniformRank& other) noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(GeometricDynamicUniformRank& other) noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(GeometricDynamicUniformRank& other) noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(GeometricDynamicUniformRank& other) noexcept
	{
		return updateComparisons(other) >= 0;
	}
};

namespace
{
	inline uint8_t num_bits_required(uint64_t value)
	{
		if (value <= BITS_REQUIRED.size())
		{
			return BITS_REQUIRED[value];
		}

		uint8_t bits = 0;
		while (value > 0)
		{
			++bits;
			value >>= 1;
		}

		return bits;
	}
}

template <typename KeyType>
class DynamicZipTree : public GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>
{
public:
	using GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>::_buckets;
	using GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>::_rootIndex;
	using GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>::NULLPTR;

	DynamicZipTree(unsigned maxSize);

	uint8_t getMaxGeometricBits(unsigned nodeIndex) const noexcept
	{
		const auto& node = _buckets[nodeIndex];

		uint8_t max_left = 0, max_right = 0;
		if (node.left != NULLPTR)
		{
			// uint8_t nbr = num_bits_required(static_cast<uint8_t>(node.rank.grank - _buckets[node.left].rank.grank));
			// if (nbr > 7)
			// {
			// 	std::cout << "grank: " << node.rank.grank << " " << _buckets[node.left].rank.grank << std::endl;
			// 	std::cout << "left: " << node.rank.grank - _buckets[node.left].rank.grank << " " << static_cast<unsigned>(nbr) << std::endl;
			// 	uint8_t difference = node.rank.grank - _buckets[node.left].rank.grank;

			// 	std::cout << "difference: " << difference << " sizeof(difference): " << sizeof(difference) << std::endl;
			// }
			max_left = std::max(getMaxGeometricBits(node.left), num_bits_required(node.rank.grank - _buckets[node.left].rank.grank));
		}

		if (node.right != NULLPTR)
		{
			max_right = std::max(getMaxGeometricBits(node.right), num_bits_required(node.rank.grank - _buckets[node.right].rank.grank));
		}

		return std::max(max_left, max_right);
	}

	uint8_t getMaxGeometricBits() const noexcept
	{
		return getMaxGeometricBits(_rootIndex);
	}

	uint64_t getTotalGeometricBits(unsigned nodeIndex) const noexcept
	{
		const auto& node = _buckets[nodeIndex];

		uint64_t total_left = 0, total_right = 0;
		if (node.left != NULLPTR)
			total_left = getTotalGeometricBits(node.left) + num_bits_required(node.rank.grank - _buckets[node.left].rank.grank);

		if (node.right != NULLPTR)
			total_right = getTotalGeometricBits(node.right) + num_bits_required(node.rank.grank - _buckets[node.right].rank.grank);

		return total_left + total_right;
	}

	uint64_t getTotalGeometricBits() const noexcept
	{
		return getTotalGeometricBits(_rootIndex) + num_bits_required(_buckets[_rootIndex].rank.grank);
	}


	uint8_t getMaxUniformBits() const noexcept
	{
		uint8_t max_bits = 0;
		for (const auto& bucket : _buckets)
		{
			if (bucket.rank.num_bits > max_bits)
				max_bits = bucket.rank.num_bits;
		}
		return max_bits;
	}

	uint64_t getTotalUniformBits() const noexcept
	{
		uint64_t total_bits = 0;
		for (const auto& bucket : _buckets)
		{
			total_bits += bucket.rank.num_bits;
		}
		return total_bits;
	}

protected:
	GeometricDynamicUniformRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());
		static std::geometric_distribution<uint64_t> gdistribution(0.5);

		return {gdistribution(generator), 0uLL, totalComparisons, firstTies, bothTies};
	}
};

template <typename KeyType>
DynamicZipTree<KeyType>::DynamicZipTree(unsigned maxSize)
	: GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>(maxSize)
{
}

#endif