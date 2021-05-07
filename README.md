# ZipTree

Zip tree C++ implementation under `src/ZipTree.h`, adapted from the pseudocode from the 2019 zip tree paper by Tarjan et al. <https://arxiv.org/abs/1806.06726>

This implementation adds an `updateNode` function that is called whenever the children of a node are modified during insertion or deletion, allowing changes to propagate upwards.

This code also contains an example first fit bin packing algorithm use case, where the best remaining capacities of each node are updated upon insertion.
