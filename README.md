# Real-Time Gaming Leaderboard — Order-Statistics AVL Tree

A live ranking engine for a gaming leaderboard, built on a **self-balancing AVL tree**
augmented with **subtree sizes**. It maintains player scores under a continuous stream of
updates and answers **order-statistics queries** — "what rank is this player?" and
"who is the k-th best?" — in **O(log N)**.

Ordinary structures force a trade-off: a sorted array gives instant `kth` but O(N)
insertion, while a hash map gives O(1) updates but no ranking at all. A size-augmented
balanced BST gives **both** live updates *and* ranking in logarithmic time.

## Features

- Insert / delete / find a player in **O(log N)** worst case.
- `rank(player)` — how many players rank before a given player — in **O(log N)**.
- `kth(k)` — the k-th player in order — in **O(log N)**.
- Leaderboard views: `kthBest(k)` (1 = highest score) and `rankBest(player)` (1 = best).
- Deterministic ordering by the **`(score, id)`** pair, so ties never rank ambiguously.
- Worst-case (not just amortized) O(log N): the AVL height stays under `1.44·log₂N`.

## Complexity

| Operation | Time | Notes |
|---|---|---|
| `insert`, `delete_node`, `find` | O(log N) | with rebalancing |
| `rank(player)` | O(log N) | uses subtree sizes to skip whole subtrees |
| `kth(k)` | O(log N) | order-statistic select |
| in-order / range scan | O(N) / O(log N + k) | |
| space | O(N) | one fixed-size node per player |

## Design

- **Node** (`Node.h`): `score`, `id`, `height`, `size` (subtree node count), `left`, `right`.
- **Ordering** (`before()` in `avl.h`): compare by `score`; break ties by `id`. The same rule
  is used everywhere (insert, delete, rank), so ordering is total and deterministic.
- **Augmentation:** every node stores `size = 1 + size(left) + size(right)`, refreshed by a
  single `update()` helper anywhere height changes — including inside both rotations. This is
  what turns an ordinary AVL tree into an *order-statistics* tree.
- **Rotation-case detection** uses **subtree balance factors**, not the inserted key, so the
  exact same rebalancing logic works for both insertion and deletion.

## Files

| File | Purpose |
|---|---|
| `Node.h` | the node struct |
| `avl.h` | the AVL tree (insert, delete, rotations, `rank`, `kth`, augmentation) |
| `main.cpp` | demo + small correctness tests (`verify()`, `rank`/`kth` round-trip, a tie) |
| `stress_test.cpp` | randomized correctness check against a `std::set` oracle |
| `benchmark.cpp` | performance: 1M inserts, height check, 1M queries, sorted-array baseline |

## Build & run

Requires a C++17 compiler (e.g. `g++`).

```bash
g++ -std=c++17 -O2 -Wall main.cpp        -o avl_demo   && ./avl_demo
g++ -std=c++17 -O2 -Wall stress_test.cpp -o stress     && ./stress
g++ -std=c++17 -O2 -Wall benchmark.cpp   -o benchmark  && ./benchmark
# optional: sweep the sorted-array baseline size
./benchmark 250000
```

## Correctness testing

`stress_test.cpp` runs **300,000 randomized operations** (insert / delete / `rank` / `kth`)
on the AVL tree *and* on a `std::set<pair<int,int>>` used as a trusted reference ("oracle").
`std::set` orders pairs exactly like `before()`, so it mirrors what the tree should contain.
After every operation the test checks:

- `kth(k)` equals the k-th element of the set,
- `rank(x)` equals x's position in the set,
- `root->size` equals the set size,
- and, periodically, `verify()` — that every node's balance factor is in {−1, 0, 1}, its
  height is correct, and `size == 1 + left.size + right.size`.

The score range is kept small on purpose, so score **ties** occur constantly and the
tie-breaking path is heavily exercised. Result: **0 mismatches**.

## Measured performance

Measured with `-O2`, `N = 1,000,000` (numbers vary by machine):

- **1M insertions:** ~377 ms → **~2.65M inserts/sec**.
- **O(log N) proof:** final tree **height = 24**, well under the AVL bound
  `1.44·log₂(10⁶) ≈ 28.7` (and `log₂N ≈ 19.9`). A million players, ≤ 24 nodes touched per query.
- **1M mixed `rank`/`kth` queries:** ~395 ms → **~2.53M queries/sec**.
- **Update speedup vs a sorted-array baseline** (same insert workload):

  | N (updates) | AVL | Sorted array | Speedup |
  |---|---|---|---|
  | 100k | 17.8 ms | 0.93 s | ~52x |
  | 200k | 40.5 ms | 3.5 s | ~86x |
  | 400k | 92.6 ms | 13.9 s | ~150x |
  | 800k | 244 ms | 57.2 s | ~234x |

  The advantage is on **updates** (a sorted array pays O(N) per insert to shift elements);
  a sorted array answers *queries* quickly too, so the win is specifically the live-update path.

## Why AVL, not a Fenwick / segment tree?

A Fenwick or segment tree also answers rank queries in O(log N), but it indexes by **value**
and needs a bounded, known key range (or coordinate compression up front). This engine is a
**live stream of arbitrary scores** with no fixed range, so a size-augmented AVL tree — which
indexes by tree position and rebalances online with worst-case O(log N) — is the natural fit.

## Résumé bullets (all figures above are measured, not assumed)

```
Real-Time Gaming Leaderboard Using AVL Trees | Self Project
- Designed a dynamic ranking engine on a self-balancing AVL tree to maintain live player scores under skewed updates.
- Implemented subtree-size augmentation with score-and-ID tie-breakers to support rank() and k-th-best order queries.
- Achieved O(log N) inserts, deletes, and rank/select over 1M+ updates (height 24 vs bound 29), in O(N) space.
```

Aggressive variant (backed by the baseline sweep — quote the scale, and note it is *update*
throughput, not query latency):

```
- Sustained ~2.6M inserts/sec over 1M+ live updates, cutting update latency ~100x vs a sorted-array baseline, in O(N) space.
```
