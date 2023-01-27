#include "ZipTreeFF.h"
#include "cdouble.h"

#include <algorithm>

namespace
{
	static const CDouble BIN_CAPACITY = 1.0;

	template <typename NodeType>
	CDouble getNodeRemainingCapacity(const NodeType& node) noexcept
	{
		if (node == nullptr)
		{
			return 0.0;
		}

		return node->val.remainingCapacity;
	}

	template <typename NodeType>
	CDouble getNodeBestRemainingCapacity(const NodeType& node) noexcept
	{
		if (node == nullptr)
		{
			return 0.0;
		}

		return node->val.bestRemainingCapacity;
	}
}


typename ZipTreeFF::Node* ZipTreeFF::updateNode(Node* node) noexcept
{
	node->val.bestRemainingCapacity = std::max({
		getNodeRemainingCapacity(node),
		getNodeBestRemainingCapacity(node->left),
		getNodeBestRemainingCapacity(node->right)
	});

	return ZigZagZipTree::updateNode(node);
}

unsigned ZipTreeFF::insertFirstSubtree(Node* node, CDouble itemWeight) noexcept
{
	unsigned insertedNodeKey;
	if (getNodeBestRemainingCapacity(node->left) >= itemWeight)
	{
		insertedNodeKey = insertFirstSubtree(node->left.get(), itemWeight);
	}
	else if (getNodeRemainingCapacity(node) >= itemWeight)
	{
		node->val.remainingCapacity -= itemWeight;
		insertedNodeKey = node->key;
	}
	else
	{
		insertedNodeKey = insertFirstSubtree(node->right.get(), itemWeight);
	}

	updateNode(node);

	return insertedNodeKey;
}

unsigned ZipTreeFF::insertFirst(CDouble itemWeight) noexcept
{
	auto* currentNode = _head.get();
	if (getNodeBestRemainingCapacity(currentNode) < itemWeight)
	{
		insert(getSize() + 1u, {BIN_CAPACITY - itemWeight, BIN_CAPACITY - itemWeight});

		return getSize();
	}

	return insertFirstSubtree(currentNode, itemWeight);
}
