#ifndef GENERALIZEDZIPTREE_H
#define GENERALIZEDZIPTREE_H

#include "BinarySearchTree.h"

#include <limits>
#include <vector>

template <typename KeyType, typename RankType>
class GeneralizedZipTree: public BinarySearchTree<KeyType>
{
public:
	GeneralizedZipTree(unsigned maxSize);

	int getDepth(const KeyType& key) const noexcept;
	int getHeight() const noexcept;
	double getAverageHeight() const noexcept;
	unsigned getSize() const noexcept;
	bool find(const KeyType& key) const noexcept;

	/**
	 * Inserts a key, value pair into the zip tree. Note that inserting there is
	 * no validation that the keys don't already exist. Add only unique keys to
	 * avoid undefined behavior.
	 *
	 * @param key new node key
	 * @param val new node value
	 */
	void insert(const KeyType& key) noexcept;

	/**
	 * Removes a node with a given key from the zip tree.
	 *
	 * @param  key key of node to remove
	 * @return     true if a node was removed, false otherwise
	 */
	// bool remove(const KeyType& key) noexcept;

	/**
	 * @return total number of comparisons made
	 */
	uint64_t getTotalComparisons() const noexcept
	{
		return _totalComparisons;
	}

	/**
	 * @return number of times the first comparison was a tie
	 */
	uint64_t getFirstTies() const noexcept
	{
		return _firstTies;
	}

	/**
	 * @return number of times both comparisons were a tie
	 */
	uint64_t getBothTies() const noexcept
	{
		return _bothTies;
	}

	const RankType& getRootRank() const noexcept
	{
		return _buckets[_rootIndex].rank;
	}

protected:
	uint64_t _totalComparisons;
	uint64_t _firstTies;
	uint64_t _bothTies;
	unsigned _rootIndex;

	static constexpr unsigned NULLPTR = std::numeric_limits<unsigned>::max();


	struct Bucket
	{
		KeyType key;
		RankType rank;
		unsigned left = NULLPTR, right = NULLPTR;
	};

	std::vector<Bucket> _buckets;

	virtual RankType getRandomRank(uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) const noexcept = 0;

private:
	int getHeight(unsigned nodeIndex) const noexcept;
	uint64_t getTotalDepth(unsigned nodeIndex, uint64_t depth) const noexcept;
};

template <typename KeyType, typename RankType>
GeneralizedZipTree<KeyType, RankType>::GeneralizedZipTree(unsigned maxSize): _rootIndex(NULLPTR),  _totalComparisons(0), _firstTies(0), _bothTies(0)
{
	_buckets.reserve(maxSize);
}

template <typename KeyType, typename RankType>
bool GeneralizedZipTree<KeyType, RankType>::find(const KeyType& key) const noexcept
{
	if (_buckets.empty())
	{
		return false;
	}

	unsigned curIndex = _rootIndex;

	while (curIndex != NULLPTR)
	{
		const auto& cur = _buckets[curIndex];

		if (key < cur.key)
		{
			curIndex = cur.left;
		}
		else if (cur.key < key)
		{
			curIndex = cur.right;
		}
		else
		{
			return true;
		}
	}

	return false;
}

template <typename KeyType, typename RankType>
void GeneralizedZipTree<KeyType, RankType>::insert(const KeyType& key) noexcept
{
	Bucket x = { key, getRandomRank(&_totalComparisons, &_firstTies, &_bothTies) };
	unsigned xIndex = _buckets.size();

	if (xIndex == 0)
	{
		_rootIndex = xIndex;
		_buckets.emplace_back(x);
		return;
	}

	auto& rank = x.rank;

	unsigned curIndex = _rootIndex;
	unsigned prevIndex = NULLPTR;

	while (curIndex != NULLPTR && (rank < _buckets[curIndex].rank || (rank == _buckets[curIndex].rank && key > _buckets[curIndex].key)))
	{
		prevIndex = curIndex;
		curIndex = key < _buckets[curIndex].key ? _buckets[curIndex].left : _buckets[curIndex].right;
	}

	_buckets.emplace_back(x);

	if (curIndex == _rootIndex)
	{
		_rootIndex = xIndex;
	}
	else if (key < _buckets[prevIndex].key)
	{
		_buckets[prevIndex].left = xIndex;
	}
	else
	{
		_buckets[prevIndex].right = xIndex;
	}

	if (curIndex == NULLPTR)
	{
		return;
	}

	if (key < _buckets[curIndex].key)
	{
		_buckets[xIndex].right = curIndex;
	}
	else
	{
		_buckets[xIndex].left = curIndex;
	}

	prevIndex = xIndex;

	while (curIndex != NULLPTR)
	{
		unsigned fixIndex = prevIndex;

		if (_buckets[curIndex].key < key)
		{
			do
			{
				prevIndex = curIndex;
				curIndex = _buckets[curIndex].right;
			}
			while (curIndex != NULLPTR && _buckets[curIndex].key < key);
		}
		else
		{
			do
			{
				prevIndex = curIndex;
				curIndex = _buckets[curIndex].left;
			}
			while (curIndex != NULLPTR && _buckets[curIndex].key > key);
		}

		if (_buckets[fixIndex].key > key || (fixIndex == xIndex && _buckets[prevIndex].key > key))
		{
			_buckets[fixIndex].left = curIndex;
		}
		else
		{
			_buckets[fixIndex].right = curIndex;
		}
	}
}


template <typename KeyType, typename RankType>
unsigned GeneralizedZipTree<KeyType, RankType>::getSize() const noexcept
{
	return _buckets.size();
}

template <typename KeyType, typename RankType>
int GeneralizedZipTree<KeyType, RankType>::getHeight() const noexcept
{
	return getHeight(_rootIndex);
}

template <typename KeyType, typename RankType>
int GeneralizedZipTree<KeyType, RankType>::getHeight(unsigned nodeIndex) const noexcept
{
	if (nodeIndex == NULLPTR)
	{
		return -1;
	}

	return std::max(getHeight(_buckets[nodeIndex].left), getHeight(_buckets[nodeIndex].right)) + 1;
}

template <typename KeyType, typename RankType>
int GeneralizedZipTree<KeyType, RankType>::getDepth(const KeyType& key) const noexcept
{
	unsigned curIndex = _rootIndex;
	int depth = 0;

	while (curIndex != NULLPTR)
	{
		if (key < _buckets[curIndex].key)
		{
			curIndex = _buckets[curIndex].left;
		}
		else if (_buckets[curIndex].key < key)
		{
			curIndex = _buckets[curIndex].right;
		}
		else
		{
			return depth;
		}

		++depth;
	}

	return -1;
}

template <typename KeyType, typename RankType>
double GeneralizedZipTree<KeyType, RankType>::getAverageHeight() const noexcept
{
	return static_cast<double>(getTotalDepth(_rootIndex, 0)) / getSize();
}

template <typename KeyType, typename RankType>
uint64_t GeneralizedZipTree<KeyType, RankType>::getTotalDepth(unsigned nodeIndex, uint64_t depth) const noexcept
{
	if (nodeIndex == NULLPTR)
	{
		return 0;
	}

	return getTotalDepth(_buckets[nodeIndex].left, depth + 1) + getTotalDepth(_buckets[nodeIndex].right, depth + 1) + depth;
}

#endif