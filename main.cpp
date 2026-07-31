#include <bits/stdc++.h>
#include "avl.h"

int main(){
    AVL tree;
    Node* root = nullptr;

    vector<pair<int,int>> data = {
        {50,1},{30,2},{70,3},{20,4},{40,5},{60,6},
        {80,7},{10,8},{25,9},{35,10},{45,11},{65,12},
        {50,99}   // deliberate score tie with {50,1}; id breaks it
    };
    for(auto& p : data) root = tree.insert(root, p.first, p.second);

    auto show = [&](const string& label){
        vector<pair<int,int>> out;
        tree.inorder(root, out);

        bool sorted_ok = true;
        for(size_t i = 1; i < out.size(); ++i)
            if(out[i].first < out[i-1].first ||
               (out[i].first == out[i-1].first && out[i].second < out[i-1].second)) sorted_ok = false;

        cout << label << "\n  in-order: ";
        for(auto& p : out) cout << "(" << p.first << ",id" << p.second << ") ";
        cout << "\n  sorted? " << (sorted_ok ? "YES" : "NO")
             << " | verify? " << (tree.verify(root) ? "PASS" : "FAIL")
             << " | count = " << out.size()
             << " | root->size = " << (root ? root->size : 0) << "\n\n";
    };

    show("After inserts:");

    // ---- Order-statistics checks: rank & kth are inverses over every node ----
    {
        vector<pair<int,int>> out;
        tree.inorder(root, out);
        bool ok = true;
        for(int i = 0; i < (int)out.size(); ++i){
            Node* node = tree.kth(root, i);                       // i-th smallest
            if(!node){ ok = false; break; }
            if(node->score != out[i].first || node->id != out[i].second) ok = false; // kth returns the right node
            if(tree.rank(root, node) != i)  ok = false;          // rank is its 0-based position
            if(tree.kth(root, tree.rank(root, node)) != node) ok = false; // kth(rank(x)) == x
        }
        cout << "rank/kth inverse over all nodes? " << (ok ? "PASS" : "FAIL") << "\n";
        cout << "  kth(0) (lowest)  = score " << tree.kth(root, 0)->score
             << " | kth(size-1) (highest) = score " << tree.kth(root, (int)out.size()-1)->score << "\n\n";
    }

    // Exercise every deletion case: leaf, one-child, and two-children nodes.
    // Note: deleting (50,1) must leave the tied (50,99) untouched.
    vector<pair<int,int>> toDelete = {{10,8},{20,4},{50,1},{70,3}};
    for(auto& p : toDelete){
        root = tree.delete_node(root, p.first, p.second);
        show("After deleting (" + to_string(p.first) + ",id" + to_string(p.second) + "):");
    }

    return 0;
}
