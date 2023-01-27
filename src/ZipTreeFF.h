#ifndef ZIPTREEFF_H
#define ZIPTREEFF_H

#include "ZigZagZipTree.h"

#include "cdouble.h"

struct FFBin
{
	CDouble remainingCapacity;
	CDouble bestRemainingCapacity;
};

class ZipTreeFF : public ZigZagZipTree<unsigned, FFBin>
{
public:
	/**
	 * Inserts item with weight to first available bin that has enough capacity,
	 * then returns the index of that bin.
	 * @param  itemWeight capacity cost of item to insert
	 * @return            index of inserted-into bin
	 */
	unsigned insertFirst(CDouble itemWeight) noexcept;
protected:
	Node* updateNode(Node* node) noexcept override;

private:
	unsigned insertFirstSubtree(Node* node, CDouble itemWeight) noexcept;
};

#endif
