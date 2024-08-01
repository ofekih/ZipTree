#ifndef UNIFORMZIPTREE2_H
#define UNIFORMZIPTREE2_H

#include "GeneralizedZipTree.h"

#include "UniformOpenSSLRandom.h"
// #include <random>

struct UniformRank
{
	uint64_t urank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;

	inline int updateComparisons(const UniformRank& other) const noexcept
	{
		++(*totalComparisons);
		if (urank == other.urank)
		{
			++(*firstTies);
			return 0;
		}
		return urank < other.urank ? -1 : 1;
	}

	bool operator<(const UniformRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const UniformRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const UniformRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const UniformRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const UniformRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};

template <typename KeyType>
class UniformZipTree : public GeneralizedZipTree<KeyType, UniformRank>
{
public:
	UniformZipTree(unsigned maxSize);

protected:
	UniformRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
		// static std::random_device rd;
		// static std::default_random_engine generator(rd());
		// std::uniform_int_distribution<uint64_t> udistribution(0, _maxURank);

		// return {udistribution(generator), totalComparisons, firstTies};
		return {get_random_uint64(0, _maxURank), totalComparisons, firstTies};
	}

private:
	uint64_t _maxURank;
};

template <typename KeyType>
UniformZipTree<KeyType>::UniformZipTree(unsigned maxSize)
	: GeneralizedZipTree<KeyType, UniformRank>(maxSize)
{
	if (maxSize > 2097152)
		_maxURank = std::numeric_limits<uint64_t>::max();
	else
		_maxURank = static_cast<uint64_t>(maxSize) * maxSize * maxSize;
}

#endif