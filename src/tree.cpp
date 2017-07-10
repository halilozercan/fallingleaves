#include "tree.h"
#include "branch.h"
#include <cstdlib>
#include <stdio.h>
#include <cmath>

Tree::Tree() {
}

int Tree::branchCount() {
  return branches.size();
}

Branch* Tree::getBranch(int x) {
  return branches[x];
}

void Tree::grow() {

  for(int i = 0; i < MAX_BRANCH_COUNT; i++) {
    addBranch((float)i);
  }

  if(branchCount() > 1) {
    for(int i = 0; i < 100000; i++) {
      getBranch(rand() % (branchCount()))->addLeaf(i);
    }
  }
}

Branch *Tree::getTrunk() {
  if(branchCount() > 1)
    return getBranch(0);
  else
    return new Branch(this, 0, -1, -1);
}

void Tree::startFalling(Clock* _clock){
  this->clock = _clock;
  int i;
  for(i = 0; i<branches.size();i++){
    branches[i]->startFalling(_clock);
  }
}

double Tree::log_w_b(int n, int base){
  return log(n)/log(base);
}

void Tree::addBranch(int number) {
  Branch *newBranch;
  if(number == 0) {
    newBranch = new Branch(this, number, -1, -1);
    newBranch->direction = 0;
    newBranch->angle = 0;
  }
  else {
    int parent = (number-1) / BRANCH_PER_LEVEL;
    int childNumber = number % BRANCH_PER_LEVEL;
    newBranch = new Branch(this, number, parent, childNumber);
    printf("Created branch: id = %d, parent = %d, level = %d, length = %f, child = %d.\n", branchCount(), newBranch->parent, newBranch->level, newBranch->length(), childNumber);
  }
  branches.push_back(newBranch);
  if(newBranch->parent != -1){
    branches[newBranch->parent]->addChild(branches.size()-1);
  }
}

int Tree::maxLevel(){
  return (int)ceil(log_w_b(MAX_BRANCH_COUNT, BRANCH_PER_LEVEL)) + 1;
}