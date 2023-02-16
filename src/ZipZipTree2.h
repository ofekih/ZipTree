#ifndef ZIPZIPTREE2_H
#define ZIPZIPTREE2_H

#include "GeneralizedZipTree.h"

#include <random>

struct GeometricUniformRank
{
	uint8_t grank;
	uint16_t urank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;
	uint64_t* bothTies;

	inline int updateComparisons(const GeometricUniformRank& other) const noexcept
	{
		++(*totalComparisons);
		if (grank == other.grank)
		{
			++(*firstTies);
			if (urank == other.urank)
			{
				++(*bothTies);
				return 0;
			}
			return urank < other.urank ? -1 : 1;
		}

		return grank < other.grank ? -1 : 1;
	}

	bool operator<(const GeometricUniformRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const GeometricUniformRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const GeometricUniformRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const GeometricUniformRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const GeometricUniformRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};

template <typename KeyType>
class ZipZipTree : public GeneralizedZipTree<KeyType, GeometricUniformRank>
{
public:
	ZipZipTree(unsigned maxSize);

protected:
	GeometricUniformRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());
		static std::geometric_distribution<uint8_t> gdistribution(0.5);
		std::uniform_int_distribution<uint16_t> udistribution(0, _maxURank);

		return {gdistribution(generator), udistribution(generator), totalComparisons, firstTies, bothTies};
	}

private:
	uint16_t _maxURank;
};

template <typename KeyType>
ZipZipTree<KeyType>::ZipZipTree(unsigned maxSize)
	: GeneralizedZipTree<KeyType, GeometricUniformRank>(maxSize)
{
	_maxURank = std::log2(maxSize);
	_maxURank = _maxURank * _maxURank * _maxURank;
}

#endif