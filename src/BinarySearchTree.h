#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

template <typename KeyType, typename ValType>
class BinarySearchTree
{
public:
	BinarySearchTree() = default;
	BinarySearchTree(unsigned maxSize);

	virtual void insert(const KeyType& key, const ValType& val) noexcept = 0;
	virtual bool remove(const KeyType& key) noexcept = 0;
	virtual int getDepth(const KeyType& key) const noexcept = 0;
	virtual int getHeight() const noexcept = 0;
};

#endif