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

	bool operator<(const ZZRank& other) const noexcept
	{
		return grank < other.grank || (grank == other.grank && urank < other.urank);
	}

	bool operator>(const ZZRank& other) const noexcept
	{
		return grank > other.grank || (grank == other.grank && urank > other.urank);
	}


	bool operator==(const ZZRank& other) const noexcept
	{
		return grank == other.grank && urank == other.urank;
	}

	bool operator<=(const ZZRank& other) const noexcept
	{
		return *this < other || *this == other;
	}

	bool operator>=(const ZZRank& other) const noexcept
	{
		return *this > other || *this == other;
	}
};


namespace
{
	/**
	 * @return a random node rank from a geometric distribution with a mean of 1
	 */
	ZZRank getRandomZZRank(uint16_t maxURank)
	{
		static std::random_device rd;
		static std::default_random_engine generator(rd());
		static std::geometric_distribution<uint8_t> gdistribution(0.5);
		static std::uniform_int_distribution<uint16_t> udistribution(0, maxURank);

		return {gdistribution(generator), udistribution(generator)};
	}
}

template <typename KeyType, typename ValType>
class ZipZipTree : public BinarySearchTree<KeyType, ValType>
{
public:
	ZipZipTree(unsigned maxSize);

	/**
	 * Inserts a key, value pair into the zip tree. Note that inserting there is
	 * no validation that the keys don't already exist. Add only unique keys to
	 * avoid undefined behavior.
	 *
	 * @param key new node key
	 * @param val new node value
	 */
	void insert(const KeyType& key, const ValType& val) noexcept;

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
	const ValType* find(const KeyType& key) const noexcept;

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
		ValType val;
		ZZRank rank;
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
	unsigned _maxURank;

	Node* insertRecursive(Node* x, std::unique_ptr<Node>& root) noexcept;
	Node* removeRecursive(const KeyType& key, std::unique_ptr<Node>& root) noexcept;
	Node* zip(Node* x, Node* y) noexcept;

	int getHeight(const std::unique_ptr<Node>& node) const noexcept;
};

template <typename KeyType, typename ValType>
ZipZipTree<KeyType, ValType>::ZipZipTree(unsigned maxSize) : _head(nullptr), _size(0u)
{
	_maxURank = std::log2(maxSize);
	_maxURank = _maxURank * _maxURank * _maxURank;
}

template <typename KeyType, typename ValType>
typename ZipZipTree<KeyType, ValType>::Node* ZipZipTree<KeyType, ValType>::updateNode(Node* node) noexcept
{
	return node;
}

template <typename KeyType, typename ValType>
void ZipZipTree<KeyType, ValType>::insert(const KeyType& key, const ValType& val) noexcept
{
	_head = std::unique_ptr<Node>(insertRecursive(new Node{key, val, getRandomZZRank(_maxURank), nullptr, nullptr}, _head));
	++_size;
}

template <typename KeyType, typename ValType>
typename ZipZipTree<KeyType, ValType>::Node* ZipZipTree<KeyType, ValType>::insertRecursive(Node* x, std::unique_ptr<Node>& root) noexcept
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

template <typename KeyType, typename ValType>
bool ZipZipTree<KeyType, ValType>::remove(const KeyType& key) noexcept
{
	unsigned prevSize = getSize();

	_head = std::unique_ptr<Node>(removeRecursive(key, _head));

	return prevSize == getSize();
}

template <typename KeyType, typename ValType>
typename ZipZipTree<KeyType, ValType>::Node* ZipZipTree<KeyType, ValType>::removeRecursive(const KeyType& key, std::unique_ptr<Node>& root) noexcept
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

template <typename KeyType, typename ValType>
typename ZipZipTree<KeyType, ValType>::Node* ZipZipTree<KeyType, ValType>::zip(Node* x, Node* y) noexcept
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

template <typename KeyType, typename ValType>
const ValType* ZipZipTree<KeyType, ValType>::find(const KeyType& key) const noexcept
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
			return &curr->val;
		}
	}

	return nullptr;
}

template <typename KeyType, typename ValType>
unsigned ZipZipTree<KeyType, ValType>::getSize() const noexcept
{
	return _size;
}

template <typename KeyType, typename ValType>
int ZipZipTree<KeyType, ValType>::getHeight() const noexcept
{
	return getHeight(_head);
}

template <typename KeyType, typename ValType>
int ZipZipTree<KeyType, ValType>::getHeight(const std::unique_ptr<Node>& node) const noexcept
{
	if (node == nullptr)
	{
		return -1;
	}

	return std::max(getHeight(node->left), getHeight(node->right)) + 1;
}

template <typename KeyType, typename ValType>
int ZipZipTree<KeyType, ValType>::getDepth(const KeyType& key) const noexcept
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
