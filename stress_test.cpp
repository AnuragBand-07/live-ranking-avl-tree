#include <bits/stdc++.h>
#include "avl.h"
using namespace std;

/*
 * Randomized stress test.
 *
 * Idea: run the same random operations against two structures:
 *   1. our AVL tree            (fast, complex -> maybe buggy)
 *   2. std::set<(score,id)>    (slow but obviously correct -> the "oracle")
 *
 * std::set orders pairs lexicographically = (score, then id) = exactly our
 * `before()` rule, so it is a faithful reference. If any query ever disagrees,
 * or an invariant breaks, we print the offending step and stop.
 */

int main(){
    AVL tree;
    Node* root = nullptr;
    set<pair<int,int>> ref;               // the oracle, ordered by (score, id)

    mt19937 rng(123456789);               // fixed seed -> reproducible failures
    const int OPS       = 300000;         // number of random operations
    const int SCORE_MAX = 2000;           // small range on purpose -> forces score ties
    const int ID_MAX    = 1000000;

    long long inserts = 0, deletes = 0, ranks = 0, kths = 0;

    auto fail = [&](const string& what, int step){
        cout << "FAIL at step " << step << ": " << what << "\n";
        exit(1);
    };

    for(int step = 0; step < OPS; ++step){
        int op = rng() % 4;

        // Force an insert when the set is empty (nothing to delete/query).
        if(op == 0 || ref.empty()){
            int score = (int)(rng() % SCORE_MAX);
            int id    = (int)(rng() % ID_MAX);
            pair<int,int> key = {score, id};
            bool isNew = !ref.count(key);
            root = tree.insert(root, score, id);   // AVL rejects exact-pair duplicates itself
            if(isNew) ref.insert(key);
            inserts++;
        }
        else if(op == 1){
            // delete a random existing element
            int idx = (int)(rng() % ref.size());
            auto it = next(ref.begin(), idx);
            int score = it->first, id = it->second;
            root = tree.delete_node(root, score, id);
            ref.erase(it);
            deletes++;
        }
        else if(op == 2){
            // rank(x) for a random existing x: must equal its 0-based position in the set
            int idx = (int)(rng() % ref.size());
            auto it = next(ref.begin(), idx);
            Node probe(it->first, it->second);        // stack node, only score/id are read
            int rAvl = tree.rank(root, &probe);
            if(rAvl != idx) fail("rank mismatch (got " + to_string(rAvl) +
                                 ", expected " + to_string(idx) + ")", step);
            ranks++;
        }
        else{
            // kth(k): must equal the k-th element of the set
            int k = (int)(rng() % ref.size());
            Node* got = tree.kth(root, k);
            auto it = next(ref.begin(), k);
            if(!got || got->score != it->first || got->id != it->second)
                fail("kth mismatch at k=" + to_string(k), step);
            kths++;
        }

        // Cheap check every step: sizes agree.
        if((int)ref.size() != (root ? root->size : 0))
            fail("size mismatch (ref " + to_string(ref.size()) +
                 " vs tree " + to_string(root ? root->size : 0) + ")", step);

        // Expensive full-invariant check periodically (O(n), so not every step).
        if(step % 2000 == 0 && !tree.verify(root))
            fail("verify() failed (balance/height/size invariant broken)", step);
    }

    // Final full invariant check.
    if(!tree.verify(root)) fail("final verify() failed", OPS);

    cout << "STRESS TEST PASSED\n";
    cout << "  operations : " << OPS << "\n";
    cout << "    inserts  : " << inserts << "\n";
    cout << "    deletes  : " << deletes << "\n";
    cout << "    rank     : " << ranks   << "\n";
    cout << "    kth      : " << kths    << "\n";
    cout << "  final size : " << (root ? root->size : 0) << "\n";
    return 0;
}
