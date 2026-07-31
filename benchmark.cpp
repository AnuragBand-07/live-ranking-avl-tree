#include <bits/stdc++.h>
#include "avl.h"
using namespace std;
using Clock = chrono::high_resolution_clock;

static double ms(Clock::time_point a, Clock::time_point b){
    return chrono::duration<double, milli>(b - a).count();
}

int main(int argc, char** argv){
    mt19937 rng(2024);
    const int N       = 1000000;   // 1M players (unique ids -> no rejected inserts)
    const int QUERIES = 1000000;   // 1M mixed rank/kth queries
    const int SCORE_MAX = 1000000000;
    // Baseline size overridable from the command line: ./benchmark <NB>
    const int NB = (argc > 1) ? atoi(argv[1]) : 100000;

    // Pre-generate random scores so RNG cost isn't counted in the timed sections.
    vector<int> scores(N);
    for(int i = 0; i < N; ++i) scores[i] = (int)(rng() % SCORE_MAX);

    // ---------------------------------------------------------------
    // 1) AVL: 1M insertions
    // ---------------------------------------------------------------
    AVL tree;
    Node* root = nullptr;

    auto t0 = Clock::now();
    for(int i = 0; i < N; ++i)
        root = tree.insert(root, scores[i], i);   // id = i -> every pair unique
    auto t1 = Clock::now();

    double insertMs = ms(t0, t1);
    int    h        = tree.height(root);
    double log2N    = log2((double)N);

    cout << "=== AVL: " << N << " insertions ===\n";
    cout << "  time            : " << insertMs << " ms\n";
    cout << "  throughput      : " << (long long)(N / (insertMs / 1000.0)) << " inserts/sec\n";
    cout << "  final tree size : " << root->size << "\n";
    cout << "  tree height     : " << h << "\n";
    cout << "  log2(N)         : " << log2N << "\n";
    cout << "  1.44*log2(N)    : " << 1.44 * log2N << "   <- AVL height stays under this\n";
    cout << "  invariants ok?  : " << (tree.verify(root) ? "YES" : "NO") << "\n\n";

    // ---------------------------------------------------------------
    // 2) AVL: 1M mixed rank/kth queries
    // ---------------------------------------------------------------
    volatile long long sink = 0;   // volatile so the optimizer can't delete the loop
    auto q0 = Clock::now();
    for(int i = 0; i < QUERIES; ++i){
        if(i & 1){
            int k = (int)(rng() % root->size);
            Node* got = tree.kth(root, k);
            sink += got->score;
        }else{
            int k = (int)(rng() % root->size);
            Node* got = tree.kth(root, k);           // grab an existing node...
            sink += tree.rank(root, got);            // ...and rank it
        }
    }
    auto q1 = Clock::now();
    double queryMs = ms(q0, q1);
    cout << "=== AVL: " << QUERIES << " mixed rank/kth queries ===\n";
    cout << "  time            : " << queryMs << " ms\n";
    cout << "  throughput      : " << (long long)(QUERIES / (queryMs / 1000.0)) << " queries/sec\n\n";

    // ---------------------------------------------------------------
    // 3) Baseline: sorted-array leaderboard vs AVL, SAME insert workload
    //    (smaller N, because array insertion is O(N) per op -> O(N^2) total)
    // ---------------------------------------------------------------
    vector<int> bscores(NB);
    for(int i = 0; i < NB; ++i) bscores[i] = (int)(rng() % SCORE_MAX);

    // AVL insert of NB
    AVL t2; Node* r2 = nullptr;
    auto a0 = Clock::now();
    for(int i = 0; i < NB; ++i) r2 = t2.insert(r2, bscores[i], i);
    auto a1 = Clock::now();
    double avlMs = ms(a0, a1);

    // Sorted-array insert of NB: binary-search the position, then shift (O(N))
    vector<pair<int,int>> arr;
    arr.reserve(NB);
    auto b0 = Clock::now();
    for(int i = 0; i < NB; ++i){
        pair<int,int> key = {bscores[i], i};
        auto pos = lower_bound(arr.begin(), arr.end(), key);
        arr.insert(pos, key);
    }
    auto b1 = Clock::now();
    double arrMs = ms(b0, b1);

    cout << "=== Baseline: " << NB << " insertions, AVL vs sorted array ===\n";
    cout << "  AVL          : " << avlMs << " ms\n";
    cout << "  sorted array : " << arrMs << " ms\n";
    cout << "  speedup      : " << (arrMs / avlMs) << "x  (AVL faster)\n\n";

    cout << "  (sink=" << sink << ")\n";  // prevents query loop from being optimized away
    return 0;
}
