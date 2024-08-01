#ifndef ZIPTREE2_H
#define ZIPTREE2_H

#include "GeneralizedZipTree.h"

// #include "UniformOpenSSLRandom.h"
#include <random>

struct GeometricRank
{
	uint64_t rank;
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
class ZipTreeVariableP : public GeneralizedZipTree<KeyType, GeometricRank>
{
public:
    // p should be within the range (0, 1)
	ZipTreeVariableP(unsigned maxSize, double p)
        : GeneralizedZipTree<KeyType, GeometricRank>(maxSize), p(p)
    {
        distribution = std::geometric_distribution<uint64_t>(p);
    }

    double getP() const noexcept { return p; }

protected:
	GeometricRank getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept override
	{
        static std::random_device rd;
		static std::mt19937_64 generator(rd());

		return {distribution(generator), totalComparisons, firstTies};
	}

private:
    const double p;
    mutable std::geometric_distribution<uint64_t> distribution;
};



#endif