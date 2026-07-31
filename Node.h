#include<iostream>

using namespace std;

struct Node{
    int score;
    int id;
    int height;
    int size;
    Node* left;
    Node* right;

    Node(int key, int ID){
        score = key;
        height = 1;
        right = left = nullptr;
        id = ID;
        size = 1;
    }
};