#ifndef DYNAMICZIPTREE2_H
#define DYNAMICZIPTREE2_H

#include "GeneralizedZipTree.h"

#include <limits>
#include <random>

#include <bitset>
#include <iostream>

struct GeometricDynamicUniformRank
{
	uint8_t grank;
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
		while (num_bits < other.num_bits)
			addBit();

		while (other.num_bits < num_bits)
			other.addBit();

		++(*totalComparisons);
		if (grank == other.grank)
		{
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

template <typename KeyType>
class DynamicZipTree : public GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>
{
public:
	using GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>::_buckets;

	DynamicZipTree(unsigned maxSize);

	uint8_t getMaxBits() const noexcept
	{
		uint8_t max_bits = 0;
		for (const auto& bucket : _buckets)
		{
			if (bucket.rank.num_bits > max_bits)
				max_bits = bucket.rank.num_bits;
		}
		return max_bits;
	}

	uint64_t getTotalBits() const noexcept
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
		static std::geometric_distribution<uint8_t> gdistribution(0.5);

		return {gdistribution(generator), 0uLL, totalComparisons, firstTies, bothTies};
	}
};

template <typename KeyType>
DynamicZipTree<KeyType>::DynamicZipTree(unsigned maxSize)
	: GeneralizedZipTree<KeyType, GeometricDynamicUniformRank>(maxSize)
{
}

#endif