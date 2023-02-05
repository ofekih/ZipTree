/**
 * This zip tree implementation is an adaptation of the pseudocode from the
 * 2019 Zip Trees paper by Tarjan et al.
 *
 * See https://arxiv.org/pdf/1806.06726.pdf
 */

#ifndef TREAP_H
#define TREAP_H

#include "BinarySearchTree.h"

#include <algorithm>
#include <memory>
#include <random>


struct TreapRank
{
	uint64_t urank;
	uint64_t* totalComparisons;
	uint64_t* firstTies;

	inline int updateComparisons(const TreapRank& other) const noexcept
	{
		++(*totalComparisons);
		if (urank == other.urank)
		{
			++(*firstTies);
			return 0;
		}
	}

	bool operator<(const TreapRank& other) const noexcept
	{
		return updateComparisons(other) < 0;
	}

	bool operator>(const TreapRank& other) const noexcept
	{
		return updateComparisons(other) > 0;
	}

	bool operator==(const TreapRank& other) const noexcept
	{
		return updateComparisons(other) == 0;
	}

	bool operator<=(const TreapRank& other) const noexcept
	{
		return updateComparisons(other) <= 0;
	}

	bool operator>=(const TreapRank& other) const noexcept
	{
		return updateComparisons(other) >= 0;
	}
};


namespace
{
	/**
	 * @return a random node rank from a uniform distribution
	 */
	TreapRank getRandomTreapRank(uint64_t* totalComparisons, uint64_t* firstTies) noexcept
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());
		static std::geometric_distribution<uint8_t> gdistribution(0.5);
		std::uniform_int_distribution<uint64_t> udistribution(0, std::numeric_limits<uint64_t>::max());

		return {udistribution(generator), totalComparisons, firstTies};
	}
}

template <typename KeyType>
class ZipZipTree : public BinarySearchTree<KeyType>
{
public:
	using BinarySearchTree<KeyType>::_totalComparisons;
	using BinarySearchTree<KeyType>::_firstTies;
	using BinarySearchTree<KeyType>::_bothTies;

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

	/**
	 * @param  key key of node to find
	 * @return     val of node to find, or nullptr if not found
	 */
	bool find(const KeyType& key) const noexcept;

	/**
	 * @return number of items in zip tree
	 */
	unsigned getSize() const noexcept;

	/**
	 * @return tree height
	 */
	int getHeight() const noexcept;

	/**
	 * @param  key key of node
	 * @return     depth of node, -1 if not found
	 */
	int getDepth(const KeyType& key) const noexcept;

protected:
	struct Node
	{
		KeyType key;
		TreapRank rank;
		std::unique_ptr<Node> left;
		std::unique_ptr<Node> right;
	};

	std::unique_ptr<Node> _head;

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
	unsigned _size;

	Node* insertRecursive(Node* x, std::unique_ptr<Node>& root) noexcept;
	Node* removeRecursive(const KeyType& key, std::unique_ptr<Node>& root) noexcept;
	Node* zip(Node* x, Node* y) noexcept;

	int getHeight(const std::unique_ptr<Node>& node) const noexcept;
};

template <typename KeyType>
ZipZipTree<KeyType>::ZipZipTree(unsigned maxSize) : BinarySearchTree<KeyType>(maxSize), _head(nullptr), _size(0u)
{
}

template <typename KeyType>
typename ZipZipTree<KeyType>::Node* ZipZipTree<KeyType>::updateNode(Node* node) noexcept
{
	return node;
}

template <typename KeyType>
void ZipZipTree<KeyType>::insert(const KeyType& key) noexcept
{
	_head = std::unique_ptr<Node>(insertRecursive(new Node{key, getRandomTreapRank(&_totalComparisons, &_firstTies), nullptr, nullptr}, _head));
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
	unsigned prevSize = getSize();

	_head = std::unique_ptr<Node>(removeRecursive(key, _head));

	return prevSize == getSize();
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

template <typename KeyType>
bool ZipZipTree<KeyType>::find(const KeyType& key) const noexcept
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

template <typename KeyType>
unsigned ZipZipTree<KeyType>::getSize() const noexcept
{
	return _size;
}

template <typename KeyType>
int ZipZipTree<KeyType>::getHeight() const noexcept
{
	return getHeight(_head);
}

template <typename KeyType>
int ZipZipTree<KeyType>::getHeight(const std::unique_ptr<Node>& node) const noexcept
{
	if (node == nullptr)
	{
		return -1;
	}

	return std::max(getHeight(node->left), getHeight(node->right)) + 1;
}

template <typename KeyType>
int ZipZipTree<KeyType>::getDepth(const KeyType& key) const noexcept
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

#endif
