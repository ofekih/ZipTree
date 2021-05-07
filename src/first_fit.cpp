#include "first_fit.h"

#include "cdouble.h"

#include "ZipTreeFF.h"

#include <algorithm>
#include <vector>

void first_fit(const std::vector<double>& items, std::vector<int>& assignment, std::vector<double>& freeSpace)
{
	ZipTreeFF tree{};

	for (unsigned i = 0u; i < items.size(); ++i)
	{
		double item = items[i];

		assignment[i] = tree.insertFirst(item);

		if (static_cast<unsigned>(assignment[i]) == freeSpace.size() + 1u)
		{
			freeSpace.emplace_back(1.0 - item);
		}
		else
		{
			freeSpace[assignment[i] - 1u] -= item;
		}
	}
}

void first_fit_decreasing(const std::vector<double>& items, std::vector<int>& assignment, std::vector<double>& freeSpace)
{
	std::vector<double> sortedItems(&items[0u], &items[items.size()]);
	std::sort(sortedItems.begin(), sortedItems.end(), std::greater<double>());
	first_fit(sortedItems, assignment, freeSpace);
}
