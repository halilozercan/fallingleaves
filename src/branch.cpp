#include <vector>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include "branch.h"
#include "tree.h"
#include "leaf.h"

double min(double a, double b) {
	return a < b ? a : b;
}

Branch::Branch(Tree *t, int x, int y, int z) {
  number = x;
  parent = y;
  tree = t;
  if(z==-1){
    angle = 0;
  }
  else{
    angle = ((rand() % 10) - 5 - (180/BRANCH_PER_LEVEL)*(z-1));
  }
  
  rootDistanceParameter = (rand() % 20 + 170.0f) / 200.0f;
  //rootDistanceParameter = 1;
  direction = rand() % 360;
  if(parent >= 0) {
    level = parentBranch()->level + 1;
  }
  else{
    level = 0;
  }
  printf("Length, %d %d %f %f\n", number, z, angle, direction);
}

float Branch::length() {
  //int treeHeight = (int)ceil(log_w_b(MAX_BRANCH_COUNT, BRANCH_PER_LEVEL));
  return ((float)age()/(level+2))/20;
}

bool Branch::isTrunk() {
  return parent == -1;
}

float Branch::radius() {
  float baseRadius = radiusAt(age());
  if(isTrunk()) {
    return baseRadius;
  }
  else {
    float parentRadius = parentBranch()->radius();
    float parentLength = parentBranch()->length();
    float distance = rootDistance();
    float allowedRadius = ((parentLength-distance/2)*parentRadius)/sqrt(pow(parentRadius, 2)+pow(parentLength, 2));
    return min(baseRadius, allowedRadius);
  }
}

Branch* Branch::parentBranch() {
  return tree->getBranch(parent);
}

int Branch::age() {
	return MAX_BRANCH_AGE - number;
}

float Branch::rootDistance() {
  float parentLength = tree->getBranch(parent)->length();
  return rootDistanceParameter * parentLength;
}

float Branch::radiusAt(float time) {
  return log(1+time/(MAX_BRANCH_AGE*(level+1)));
}

void Branch::addChild(int child){
  this->children.push_back(child);
}

std::vector<int> Branch::parents() {
  std::vector<int> result;
  int par = parent;
  while(par >= 0) {
    result.push_back(par);
    par = tree->getBranch(par)->parent;
  }
  return result;
}

void Branch::addLeaf(int second) {
  if(level > 2 && leaves.size() < 5) {
    Leaf *newLeaf = new Leaf(tree, this, second);
    leaves.push_back(newLeaf);
  }
}

void Branch::startFalling(Clock* _clock){
  this->clock = _clock;
  for(int i=0; i<leaves.size(); i++){
    leaves[i]->rotateAngle = rand();
  }
}