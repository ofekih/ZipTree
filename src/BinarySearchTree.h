#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include <cstdint>
#include <memory>

template <typename KeyType>
class BinarySearchTree
{
public:
	BinarySearchTree() = default;
	BinarySearchTree(unsigned maxSize);
	virtual ~BinarySearchTree() = default;

	virtual void insert(const KeyType& key) noexcept = 0;
	// virtual bool remove(const KeyType& key) noexcept = 0;
	virtual int getDepth(const KeyType& key) const noexcept = 0;
	virtual int getHeight() const noexcept = 0;
	virtual double getAverageHeight() const noexcept = 0;
	virtual unsigned getSize() const noexcept = 0;
	virtual bool find(const KeyType& key) const noexcept = 0;
	virtual uint64_t getTotalComparisons() const noexcept = 0;
	virtual uint64_t getFirstTies() const noexcept = 0;
	virtual uint64_t getBothTies() const noexcept = 0;
};

template <typename KeyType, typename RankType>
class BinarySearchTreeRank : public BinarySearchTree<KeyType>
{
public:
	BinarySearchTreeRank(unsigned maxSize);

	int getDepth(const KeyType& key) const noexcept;
	int getHeight() const noexcept;
	double getAverageHeight() const noexcept;
	unsigned getSize() const noexcept;
	bool find(const KeyType& key) const noexcept;

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
	unsigned _size;

	struct Node
	{
		KeyType key;
		RankType rank;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	std::unique_ptr<Node> _head;

private:
	int getHeight(const std::unique_ptr<Node>& node) const noexcept;
	uint64_t getTotalDepth(const std::unique_ptr<Node>& node, uint64_t depth) const noexcept;
};

template <typename KeyType, typename RankType>
BinarySearchTreeRank<KeyType, RankType>::BinarySearchTreeRank(unsigned maxSize): _head(nullptr), _size(0), _totalComparisons(0), _firstTies(0), _bothTies(0)
{
}

template <typename KeyType, typename RankType>
bool BinarySearchTreeRank<KeyType, RankType>::find(const KeyType& key) const noexcept
{
	auto* curr = _head.get();
	while (curr != nullptr)
	{
		if (key < curr->key)
		{
			curr = curr->left.get();
		}
		else if (curr->key < key)
		{
			curr = curr->right.get();
		}
		else
		{
			return true;
		}
	}

	return false;
}

template <typename KeyType, typename RankType>
unsigned BinarySearchTreeRank<KeyType, RankType>::getSize() const noexcept
{
	return _size;
}

template <typename KeyType, typename RankType>
int BinarySearchTreeRank<KeyType, RankType>::getHeight() const noexcept
{
	return getHeight(_head);
}

template <typename KeyType, typename RankType>
int BinarySearchTreeRank<KeyType, RankType>::getHeight(const std::unique_ptr<Node>& node) const noexcept
{
	if (node == nullptr)
	{
		return -1;
	}

	return std::max(getHeight(node->left), getHeight(node->right)) + 1;
}

template <typename KeyType, typename RankType>
int BinarySearchTreeRank<KeyType, RankType>::getDepth(const KeyType& key) const noexcept
{
	auto* curr = _head.get();
	int depth = 0;
	while (curr != nullptr)
	{
		if (key < curr->key)
		{
			curr = curr->left.get();
		}
		else if (curr->key < key)
		{
			curr = curr->right.get();
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
double BinarySearchTreeRank<KeyType, RankType>::getAverageHeight() const noexcept
{
	return static_cast<double>(getTotalDepth(_head, 0)) / _size;
}

template <typename KeyType, typename RankType>
uint64_t BinarySearchTreeRank<KeyType, RankType>::getTotalDepth(const std::unique_ptr<Node>& node, uint64_t depth) const noexcept
{
	if (node == nullptr)
	{
		return 0;
	}

	return getTotalDepth(node->left, depth + 1) + getTotalDepth(node->right, depth + 1) + depth;
}


#endif