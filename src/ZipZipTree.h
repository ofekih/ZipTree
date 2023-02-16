/**
 * This zip tree implementation is an adaptation of the pseudocode from the
 * 2019 Zip Trees paper by Tarjan et al.
 *
 * See https://arxiv.org/pdf/1806.06726.pdf
 */

#ifndef ZIPZIPTREE_H
#define ZIPZIPTREE_H

#include "BinarySearchTree.h"

#include <algorithm>
#include <memory>
#include <random>


struct ZZRank
{
	uint8_t grank;
	uint16_t urank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;
	uint64_t* bothTies;

	inline int updateComparisons(const ZZRank& other) const noexcept
	{
		// ++(*totalComparisons);
		if (grank == other.grank)
		{
			// ++(*firstTies);
			if (urank == other.urank)
			{
				// ++(*bothTies);
				return 0;
			}
			return urank < other.urank ? -1 : 1;
		}

		return grank < other.grank ? -1 : 1;
	}

	bool operator<(const ZZRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const ZZRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const ZZRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const ZZRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const ZZRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};


namespace
{
	/**
	 * @return a random node rank from a geometric distribution with a mean of 1
	 */
	ZZRank getRandomZZRank(uint16_t maxURank, uint64_t* totalComparisons, uint64_t* firstTies, uint64_t* bothTies) noexcept
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());
		static std::geometric_distribution<uint8_t> gdistribution(0.5);
		std::uniform_int_distribution<uint16_t> udistribution(0, maxURank);

		return {gdistribution(generator), udistribution(generator), totalComparisons, firstTies, bothTies};
	}
}

template <typename KeyType>
class ZipZipTree : public BinarySearchTreeRank<KeyType, ZZRank>
{
public:
	using BinarySearchTreeRank<KeyType, ZZRank>::_totalComparisons;
	using BinarySearchTreeRank<KeyType, ZZRank>::_firstTies;
	using BinarySearchTreeRank<KeyType, ZZRank>::_bothTies;
	typedef typename BinarySearchTreeRank<KeyType, ZZRank>::Node Node;
	using BinarySearchTreeRank<KeyType, ZZRank>::_head;
	using BinarySearchTreeRank<KeyType, ZZRank>::_size;

	ZipZipTree(unsigned maxSize);

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
	bool remove(const KeyType& key) noexcept;

protected:
	/**
	 * This function is called on nodes after either:
	 *  - they have a new child on either side
	 *  - one of their children had updateNode called on it
	 *
	 * This is called during both insertion and deletion, and the node argument
	 * is guaranteed not to be null.
	 *
	 * For example, it can be used to update the best remaining capacity of nodes
	 * for the first fit bin packing algorithm, see ZipZipTreeFF for an example.
	 *
	 * @param  node the node to modify
	 * @return      the node after modification
	 */
	virtual Node* updateNode(Node* node) noexcept;

private:
	uint16_t _maxURank;

	Node* insertRecursive(Node* x, std::unique_ptr<Node>& root) noexcept;
	Node* removeRecursive(const KeyType& key, std::unique_ptr<Node>& root) noexcept;
	Node* zip(Node* x, Node* y) noexcept;
};

template <typename KeyType>
ZipZipTree<KeyType>::ZipZipTree(unsigned maxSize) : BinarySearchTreeRank<KeyType, ZZRank>(maxSize)
{
	_maxURank = std::log2(maxSize);
	_maxURank = _maxURank * _maxURank * _maxURank;
}

template <typename KeyType>
typename ZipZipTree<KeyType>::Node* ZipZipTree<KeyType>::updateNode(Node* node) noexcept
{
	return node;
}

template <typename KeyType>
void ZipZipTree<KeyType>::insert(const KeyType& key) noexcept
{
	_head = std::unique_ptr<Node>(insertRecursive(new Node{key, getRandomZZRank(_maxURank, &_totalComparisons, &_firstTies, &_bothTies), nullptr, nullptr}, _head));
	++_size;
}

template <typename KeyType>
typename ZipZipTree<KeyType>::Node* ZipZipTree<KeyType>::insertRecursive(Node* x, std::unique_ptr<Node>& root) noexcept
{
	if (root == nullptr)
	{
		return x;
	}

	if (x->key < root->key)
	{
		Node* subroot = insertRecursive(x, root->left);
		if (subroot == x && x->rank >= root->rank)
		{
			root->left = std::unique_ptr<Node>(x->right.release());
			x->right = std::unique_ptr<Node>(updateNode(root.release()));

			return updateNode(x);
		}
		else
		{
			root->left = std::unique_ptr<Node>(subroot);
		}
	}
	else
	{
		Node* subroot = insertRecursive(x, root->right);
		if (subroot == x && x->rank > root->rank)
		{
			root->right = std::unique_ptr<Node>(x->left.release());
			x->left = std::unique_ptr<Node>(updateNode(root.release()));

			return updateNode(x);
		}
		else
		{
			root->right = std::unique_ptr<Node>(subroot);
		}
	}

	return updateNode(root.release());
}

template <typename KeyType>
bool ZipZipTree<KeyType>::remove(const KeyType& key) noexcept
{
	unsigned prevSize = _size;

	_head = std::unique_ptr<Node>(removeRecursive(key, _head));

	return prevSize == _size;
}

template <typename KeyType>
typename ZipZipTree<KeyType>::Node* ZipZipTree<KeyType>::removeRecursive(const KeyType& key, std::unique_ptr<Node>& root) noexcept
{
	if (!root) // not found
	{
		return root.release();
	}

	if (key == root->key)
	{
		--_size;
		return zip(root->left.release(), root->right.release());
	}

	if (key < root->key)
	{
		root->left = std::unique_ptr<Node>(removeRecursive(key, root->left));
	}
	else
	{
		root->right = std::unique_ptr<Node>(removeRecursive(key, root->right));
	}

	return updateNode(root.release());
}

template <typename KeyType>
typename ZipZipTree<KeyType>::Node* ZipZipTree<KeyType>::zip(Node* x, Node* y) noexcept
{
	if (x == nullptr)
	{
		return y;
	}

	if (y == nullptr)
	{
		return x;
	}

	if (x->rank < y->rank)
	{
		y->left = std::unique_ptr<Node>(zip(x, y->left.release()));

		return updateNode(y);
	}
	else
	{
		x->right = std::unique_ptr<Node>(zip(x->right.release(), y));

		return updateNode(x);
	}
}

#endif
