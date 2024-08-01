#ifndef ZIPTREE2_H
#define ZIPTREE2_H

#include "GeneralizedZipTree.h"

#include "UniformOpenSSLRandom.h"
// #include <random>

struct GeometricRank
{
	uint8_t rank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;

	inline int updateComparisons(const GeometricRank& other) const noexcept
	{
		++(*totalComparisons);
		if (rank == other.rank)
		{
			++(*firstTies);
			return 0;
		}

		return rank < other.rank ? -1 : 1;
	}

	bool operator<(const GeometricRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const GeometricRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const GeometricRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const GeometricRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const GeometricRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};

template <typename KeyType>
class ZipTree : public GeneralizedZipTree<KeyType, GeometricRank>
{
public:
	ZipTree(unsigned maxSize) : GeneralizedZipTree<KeyType, GeometricRank>(maxSize) {}

protected:
	GeometricRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
		// static std::random_device rd;
		// static std::default_random_engine generator(rd());
		// static std::geometric_distribution<uint8_t> distribution(0.5);

		// return {distribution(generator), totalComparisons, firstTies};
		return {get_random_geometric(), totalComparisons, firstTies};
	}
};



#endif