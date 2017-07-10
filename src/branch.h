#ifndef BRANCH_H
#define BRANCH_H

#define MAX_BRANCH_AGE 200

#include <vector>
#include "clock.h"

class Tree;

class Leaf;

class Branch {
  public:
    Branch(Tree *t, int number, int parent, int childNumber);
    float length();
    float radius();
    float rootDistance();
    int age();
    bool isTrunk();
    std::vector<int> parents();
    std::vector<Leaf*> leaves;
    int number;
    float angle;
    float rootDistanceParameter;
    float direction;
    int parent;
    int level;
    std::vector<int> children;
    void addLeaf(int second);
    void addChild(int child);

    void startFalling(Clock* _clock);
    Clock *clock;
    Tree *tree;
  private:
    float radiusAt(float time);
    Branch *parentBranch();
};

#endif // BRANCH_H
