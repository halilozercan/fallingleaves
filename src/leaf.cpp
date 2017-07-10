#include "leaf.h"
#include "tree.h"
#include <cmath>
#include <cstdlib>
#include <stdio.h>

double mixn(double a, double b) {
	if (a < b) return a;
	return b;
}

Leaf::Leaf(Tree *t, Branch *b, int second) {
  tree = t;
  branch = b;
  rootDistanceParameter = (rand() % 100) / 100.f;
  created_at = second;
  rotateAngle = rand();
  fallStages[0] = FallInfo(rand()%2);
  fallStages[0].duration = 1000000;
  fallStages[0].forgeTrajectory();
  fallStart = 0.f;
}

float Leaf::length() {
  float dynamicLength = log(1+age()/100.0f);
  return mixn(0.25f, dynamicLength);
}

float Leaf::age() {
  return 10 - created_at;
}

float Leaf::rootDistance() {
  return rootDistanceParameter * branch->length();
}

float Leaf::fallProb() {
  int maxLevel = tree->maxLevel();
  return 1.f / (float)pow(128, maxLevel - branch->level);
}
