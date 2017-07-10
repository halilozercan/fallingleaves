#ifndef TREE_H
#define TREE_H

#define MAX_BRANCH_COUNT 512
#define BRANCH_PER_LEVEL 4

#include<vector>

#include "clock.h"

class Branch;

class Tree {
  public:
    Tree();
    Branch* getBranch(int x);
    Branch* getTrunk();
    void grow();
    int branchCount();
    void startFalling(Clock* _clock);
    Clock *clock;
    int maxLevel();
    static double log_w_b(int n, int base);
  private:
    std::vector<Branch*> branches;
    void addBranch(int number);

};

#endif // TREE_H
