#ifndef ZIPZIPTREE2_H
#define ZIPZIPTREE2_H

#include "GeneralizedZipTree.h"

#include "UniformOpenSSLRandom.h"
#include <random>

struct GeometricGeometricRank
{
	uint16_t grank1;
	uint16_t grank2;
	uint64_t* totalComparisons;
	uint64_t* firstTies;
	uint64_t* bothTies;

	inline int updateComparisons(const GeometricGeometricRank& other) const noexcept
	{
		++(*totalComparisons);
		if (grank1 == other.grank1)
		{
			++(*firstTies);
			if (grank2 == other.grank2)
			{
				++(*bothTies);
				return 0;
			}
			return grank2 < other.grank2 ? -1 : 1;
		}

		return grank1 < other.grank1 ? -1 : 1;
	}

	bool operator<(const GeometricGeometricRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const GeometricGeometricRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const GeometricGeometricRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const GeometricGeometricRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const GeometricGeometricRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};

template <typename KeyType>
class ZipZipTree : public GeneralizedZipTree<KeyType, GeometricGeometricRank>
{
public:
	ZipZipTree(unsigned maxSize);

protected:
	GeometricGeometricRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
		static std::random_device rd;
		static std::mt19937 generator(rd());
		static std::geometric_distribution<uint16_t> gdistribution(0.5);

		return {gdistribution(generator), gdistribution(generator), totalComparisons, firstTies, bothTies};
		// return {get_random_geometric(), get_random_geometric(), totalComparisons, firstTies, bothTies};
	}
};

template <typename KeyType>
ZipZipTree<KeyType>::ZipZipTree(unsigned maxSize)
	: GeneralizedZipTree<KeyType, GeometricGeometricRank>(maxSize)
{
}

#endif