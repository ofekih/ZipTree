#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include <cstdint>

template <typename KeyType>
class BinarySearchTree
{
public:
	BinarySearchTree() = default;
	BinarySearchTree(unsigned maxSize);
	virtual ~BinarySearchTree() = default;

	virtual void insert(const KeyType& key) noexcept = 0;
	virtual bool remove(const KeyType& key) noexcept = 0;
	virtual int getDepth(const KeyType& key) const noexcept = 0;
	virtual int getHeight() const noexcept = 0;
	virtual unsigned getSize() const noexcept = 0;
	virtual bool find(const KeyType& key) const noexcept = 0;

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

protected:
	uint64_t _totalComparisons;
	uint64_t _firstTies;
	uint64_t _bothTies;
};

template <typename KeyType>
BinarySearchTree<KeyType>::BinarySearchTree(unsigned maxSize): _totalComparisons(0), _firstTies(0), _bothTies(0)
{
}

#endif