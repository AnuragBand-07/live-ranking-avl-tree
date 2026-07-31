#ifndef AVL_H
#define AVL_H

#include <bits/stdc++.h>
#include "Node.h"

class AVL{
    private:
    int getbalance(Node* root){
        if(!root)return 0;
        return getheight(root->left) - getheight(root->right);
    }
    int getsize(Node* n){ return n ? n->size : 0; }

    // true if (s1,id1) ranks strictly before (s2,id2): score first, id breaks ties
    bool before(int s1, int id1, int s2, int id2){
        if(s1 != s2) return s1 < s2;
        return id1 < id2;
    }

    void update(Node* n){
        n->height = 1 + max(getheight(n->left), getheight(n->right));
        n->size   = 1 + getsize(n->left) + getsize(n->right);
    }
    int getheight(Node* root){
        if(!root)return 0;
        
        return root->height;
    }

    Node* leftRotate(Node* root){
        Node* child = root->right;
        Node* childLeft = root->right->left;

        child->left = root;
        root->right = childLeft;

        // Height + size update: root first (it now sits below child), then child
        update(root);
        update(child);

        return child;

    }

    Node* rightRotate(Node* root){
        Node* child = root->left;
        Node* childRight = root->left->right;

        child->right = root;
        root->left = childRight;
        // Height + size update: root first (it now sits below child), then child
        update(root);
        update(child);

        return child;
    }

    public:
    Node* insert(Node* root, int key, int id){
        // Doesn't exist
        if(!root)return new Node(key, id);

        //Exists — order by the (score,id) pair
        if(before(key, id, root->score, root->id)){
            root->left = insert(root->left, key, id);
        }
        else if(before(root->score, root->id, key, id)){
            root->right = insert(root->right,key,id);
        }
        else{
            return root;  // exact (score,id) duplicate — not allowed
        }

        //update height
        update(root);

        // Balancing check — decide the case by subtree shape, not the inserted key
        int balance = getbalance(root);
        int balance_left = getbalance(root->left);
        int balance_right = getbalance(root->right);

        // LL
        if(balance > 1 && balance_left >= 0){
            return rightRotate(root);
        }
        // LR
        else if(balance > 1 && balance_left < 0){
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }
        // RR
        else if(balance < -1 && balance_right <= 0){
            return leftRotate(root);
        }
        // RL
        else if(balance < -1 && balance_right > 0){
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }
        else{
            return root;
        }
    }
    Node* delete_node(Node* root, int key, int id){
        if(!root)return nullptr;

        
        if(before(key, id, root->score, root->id)){
            root->left = delete_node(root->left, key, id);
        }
        else if(before(root->score, root->id, key, id)){
            root->right =  delete_node(root->right, key, id);
        }
        else{

            if(!root->left && !root->right){
                delete root;
                return nullptr;
            }
            else if(!root->right && root->left){
                Node* temp = root->left;
                delete root;
                return temp;
            }
            else if(root->right && !root->left){
                Node* temp = root->right;
                delete root;
                return temp;
            }
            else{
                Node* temp = root->right;
                while (temp->left != nullptr) {
                    temp = temp->left;
                }
                root->score = temp->score;
                root->id    = temp->id;
                root->right = delete_node(root->right, temp->score, temp->id);
            }
        }
        

        //update height
        update(root);

        // Balancing check
        int balance_parent = getbalance(root);
        int balance_left = getbalance(root->left);
        int balance_right = getbalance(root->right);

        // LL
        if(balance_parent > 1 && balance_left >= 0){
            return rightRotate(root);
        }
        // LR
        else if(balance_parent > 1 && balance_left < 0){
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }
        // RR
        else if(balance_parent < -1 && balance_right <= 0){
            return leftRotate(root);
        }
        // RL
        else if(balance_parent < -1 && balance_right > 0){
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }
        else return root;
    }

    int rank(Node* root, Node* n){
        if(!root)return 0;
        if(before(root->score, root->id, n->score, n->id)){
            return getsize(root->left) + 1 + rank(root->right, n);
        }
        else if(before(n->score, n->id, root->score, root->id)){
            return rank(root->left, n);
        }
        else return rank(root->left, n);
    }

    // k-th smallest, 0-based: k in [0, size-1]. Returns nullptr if k is out of range.
    Node* kth(Node* root, int k){
        if(!root) return nullptr;                       // fell off the tree -> out of range
        int leftSize = getsize(root->left);
        if(k < leftSize)        return kth(root->left, k);        // k-th lies in the left subtree
        else if(k == leftSize)  return root;                     // this node IS the k-th smallest
        else                    return kth(root->right, k - leftSize - 1); // skip left subtree + root
    }

    // k-th BEST, 1-based: kthBest(1) = highest score
    Node* kthBest(Node* root, int k){ return kth(root, root->size - k); }

    // leaderboard rank, 1-based: rankBest = 1 means best player
    int rankBest(Node* root, Node* n){ return root->size - rank(root, n); }

    // Height of the whole tree (0 for empty) — used by the benchmark to check the O(log N) bound.
    int height(Node* root){ return getheight(root); }

    // ---- Test helpers ----

    // Collect (score, id) pairs in ascending tree order.
    void inorder(Node* root, vector<pair<int,int>>& out){
        if(!root) return;
        inorder(root->left, out);
        out.push_back({root->score, root->id});
        inorder(root->right, out);
    }

    // Recursively assert every AVL + augmentation invariant.
    bool verify(Node* root){
        if(!root) return true;
        if(abs(getbalance(root)) > 1) return false;                          // balance factor in {-1,0,1}
        if(root->height != 1 + max(getheight(root->left), getheight(root->right))) return false;
        if(root->size   != 1 + getsize(root->left)  + getsize(root->right))  return false;
        return verify(root->left) && verify(root->right);
    }
};

#endif // AVL_H
