#ifndef LEAF_H
#define LEAF_H

#include "branch.h"
#include "fallinfo.h"

class Leaf {
  public:
    Leaf(Tree *tree, Branch *branch, int second);
    float length();
    float rootDistance();
    float age();
	float rotateAngle;
    Branch *branch;
    FallInfo fallStages[5];
    float fallStart;
    float fallProb();
    int created_at;
    glm::vec3 hitGround;
  private:
    Tree *tree;
    float rootDistanceParameter;
};

#endif // LEAF_H
