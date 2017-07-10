#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>
#include <utility>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <sys/time.h>
#include "math.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "tga.h"
#include "tree.h"
#include "branch.h"
#include "leaf.h"
#include "sky.h"
#include "spherical_coordinates.h"

int lastTime = 0;

GLuint grassTexture;
GLuint branchTexture;
GLuint leafTexture;
GLuint stemTexture;
GLuint skyTexture;
TGAImg image;

Clock *worldClock = new Clock();
Tree *tree = new Tree();
//Sky *sky = new Sky(worldClock);

glm::vec3 wind(0.12f, 0.02f, 0.12f);
SphericalCoordinates *cameraCoordinates = new SphericalCoordinates(20.0f, 0.0f, 0.45f * M_PI, 0.0f, 50.0f, 0.05 * M_PI, 0.45f * M_PI);
bool cameraFlags[6] = { false, false, false, false, false };
bool speedupFlag = false;
bool fallingFlag = false;
bool isFalling = false;

#define CAMERA_UP_KEY GLUT_KEY_UP
#define CAMERA_DOWN_KEY GLUT_KEY_DOWN
#define CAMERA_LEFT_KEY GLUT_KEY_LEFT
#define CAMERA_RIGHT_KEY GLUT_KEY_RIGHT
#define CAMERA_ZOOMIN_KEY 'z'
#define CAMERA_ZOOMOUT_KEY 'x'
#define CLOCK_SPEEDUP_KEY ' '
#define START_FALLING_KEY 'm'

#define CLOCK_SPEEDUP_FACTOR 10.0f

void loadTexture(char *filename, GLuint *handle) {
  printf("Loading texture %s\n", filename);
  if (image.Load(filename) == IMG_OK) {
    glGenTextures(1, handle);
    glBindTexture(GL_TEXTURE_2D, *handle);
    if (image.GetBPP() == 24) {
      glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(), 0, GL_RGB,GL_UNSIGNED_BYTE, image.GetImg());
    }
    else if (image.GetBPP() == 32) {
      glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(), 0, GL_RGBA,GL_UNSIGNED_BYTE, image.GetImg());
    }
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
    printf("Error loading texture %s\n", filename);
}

void drawStem(GLfloat len, GLfloat t) {
  glPushMatrix();
  glRotatef(-90, 1.0, 0.0, 0.0);
  GLUquadricObj *q;
  q = gluNewQuadric();
  gluQuadricNormals(q, GLU_SMOOTH);
  gluQuadricTexture(q, GL_TRUE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, stemTexture);
  gluCylinder(q, t, 0.6*t, len/4, 16, 4); //GLUquadricObj *qobj, baseRadius, topRadius,    
  glDisable(GL_TEXTURE_2D);
  glRotatef(90, 1.0, 0.0, 0.0);
  gluDeleteQuadric(q);       //Destroys the quadrics object q and frees up any memory used by it
  glPopMatrix();
};

void drawLeaf(Leaf *leaf, glm::mat4 V, glm::mat4 M) {
  glm::mat4 lM = glm::translate(M, glm::vec3(0.0f, 0.0f, leaf->rootDistance()));
  std::pair<float, glm::vec3> angularPair;
  float totalAngularRotation = 0.f;
  if(isFalling){
    if(leaf->fallStart>0.0f){
      glm::mat4 leafMatrix=glm::mat4(1.0f);

      int stage = 0;
      float totaltime = 0.0f;
      while(totaltime + leaf->fallStages[stage].duration < (worldClock->value - leaf->fallStart) && stage<4){
        totaltime += leaf->fallStages[stage].duration;
        stage++;
      }
      //printf("%d %d\n", stage, totaltime);
      float totaltimeInCurrentStage = (worldClock->value - leaf->fallStart) - totaltime;
      int tickCount = totaltimeInCurrentStage / worldClock->getSpeed();
      //printf("%f %f %d %d\n", (float)totaltimeInCurrentStage, worldClock->getSpeed(), tickCount, totaltimeInCurrentStage);
      for(int i=0;i<tickCount;i++){
        angularPair = leaf->fallStages[stage].angular[i*15 % leaf->fallStages[stage].angular.size()];
        glm::vec3 velocity = leaf->fallStages[stage].velocity[i*15 % leaf->fallStages[stage].velocity.size()];
        totalAngularRotation += angularPair.first;
        leafMatrix = glm::translate(leafMatrix, velocity);
        // Wind
        leafMatrix = glm::translate(leafMatrix, wind);
        //leafMatrix = glm::rotate(leafMatrix, angularPair.first, angularPair.second);
      }
      const float *leafMatrixSource = (const float*)glm::value_ptr(leafMatrix);
      const float *lMSource = (const float*)glm::value_ptr(lM);
      float finalY = lMSource[13] + leafMatrixSource[13];
      if(finalY < 0){
        finalY = 0;
        leaf->fallStart = -1.0f;
        leaf->hitGround = glm::vec3(lMSource[12] + leafMatrixSource[12], finalY, lMSource[14] + leafMatrixSource[14]);
      }
      float finalSource[16] = {
        lMSource[0], lMSource[1], lMSource[2], lMSource[3],
        lMSource[4], lMSource[5], lMSource[6], lMSource[7],
        lMSource[8], lMSource[9], lMSource[10], lMSource[11],
        lMSource[12] + leafMatrixSource[12], finalY, lMSource[14] + leafMatrixSource[14], lMSource[15],
      };
      lM = glm::make_mat4(finalSource);
    }
    else if(leaf->fallStart == 0.f){
      float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      if(r < leaf->fallProb() * worldClock->value + glm::length(wind)*0.5f){
        leaf->fallStart = worldClock->value;
      }
    }
    else{
      glm::mat4 leafMatrix=glm::mat4(1.0f);
      leafMatrix = glm::translate(leafMatrix, leaf->hitGround);
      lM = leafMatrix;
    }
  }
  glLoadMatrixf(glm::value_ptr(V*lM));
  float len = 1;
  float t = 0.1;
  glScaled(0.2, 0.2, 0.2);
  if(!isFalling || leaf->fallStart == 0.f)
    drawStem(len, t);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,leafTexture);
  /*GLUquadric *qobj = gluNewQuadric();
  gluQuadricTexture(qobj,GL_TRUE);*/
  glRotatef(90, 1.0, 0.0, 0.0);
  glTranslatef(0.0, 0.0, -len);
  
  //glScaled(2.0, 2.0, 2.0);
  if(leaf->fallStart >= 0)
    glRotatef(leaf->rotateAngle, 0.0, 1.0, 0.0);
  if(isFalling && leaf->fallStart > 0.f){
    glRotatef(totalAngularRotation, angularPair.second.x, angularPair.second.y, angularPair.second.z);
  }
  for (int i = 0; i<5; i++) {
    float posAngle = 360 * (rand() / (float)RAND_MAX);

    glPushMatrix();
    glRotatef(36*(i+1), 0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glTexCoord2f(0.5, 0.0);    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.834, 0.083);  glVertex3f(0.167, 0.083, 0.0);
    glTexCoord2f(0.95, 0.208);  glVertex3f(0.225, 0.208, 0.0);
    glTexCoord2f(1.0, 0.42);   glVertex3f(0.25, 0.42, 0.0);
    glTexCoord2f(0.96, 0.583);  glVertex3f(0.23, 0.583, 0.0);
    glTexCoord2f(0.834, 0.75);   glVertex3f(0.167, 0.75, 0.0);
    glTexCoord2f(0.5, 1.0);    glVertex3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.166, 0.75);   glVertex3f(-0.167, 0.75, 0.0);
    glTexCoord2f(0.04, 0.583);  glVertex3f(-0.23, 0.583, 0.0);
    glTexCoord2f(0.0, 0.42);   glVertex3f(-0.25, 0.42, 0.0);
    glTexCoord2f(0.5, 0.208);  glVertex3f(-0.225, 0.208, 0.0);
    glTexCoord2f(0.166, 0.083);  glVertex3f(-0.167, 0.083, 0.0);
    glEnd();
    glPopMatrix();
  }

  glDisable(GL_TEXTURE_2D);
}

int windFlex = 0;
int targetFlex = 0;

void drawBranch(Branch *branch, const glm::mat4 V) {
  glm::mat4 M=glm::mat4(1.0f);
  std::vector<int> parents = branch->parents();

  for(int l = parents.size()-1; l >= 0; l--) {
    M=glm::rotate(M, tree->getBranch(parents[l])->direction, glm::vec3(0.0f, 1.0f, 0.0f));
    M=glm::rotate(M, tree->getBranch(parents[l])->angle, glm::vec3(0.0f, 0.0f, 1.0f));
    if(isFalling){
      M=glm::rotate(M, wind.x*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(0.0f, 1.0f, 0.0f));
      M=glm::rotate(M, wind.y*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(0.0f, 0.0f, 1.0f));
      M=glm::rotate(M, wind.z*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    if(l > 0)
      M=glm::translate(M, glm::vec3(0.0f, tree->getBranch(parents[l-1])->rootDistance(), 0.0f));
  }

  if(branch->parent >= 0)
    M=glm::translate(M, glm::vec3(0.0f, branch->rootDistance(), 0.0f));
  M=glm::rotate(M, branch->direction, glm::vec3(0.0f, 1.0f, 0.0f));
  M=glm::rotate(M, branch->angle, glm::vec3(0.0f, 0.0f, 1.0f));
  if(isFalling){
    M=glm::rotate(M, wind.x*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(0.0f, 1.0f, 0.0f));
    M=glm::rotate(M, wind.y*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(0.0f, 0.0f, 1.0f));
    M=glm::rotate(M, wind.z*windFlex*(float)Tree::log_w_b(branch->level+1, 2), glm::vec3(1.0f, 0.0f, 0.0f));
  }
  M=glm::rotate(M, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
  //glm::vec3 windProjX = glm::gtx::projection::proj(wind, )
  glLoadMatrixf(glm::value_ptr(V*M));
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  float radius = branch->radius()/2;
  float length = branch->length();

  glBindTexture(GL_TEXTURE_2D,branchTexture);
  GLUquadric *qobj = gluNewQuadric();
  gluQuadricTexture(qobj,GL_TRUE);
  gluCylinder(qobj,radius, radius/2, length,50,50);
  gluDeleteQuadric(qobj);
  glDisable(GL_TEXTURE_2D);

  for(int l = 0; l < branch->leaves.size(); l++) {
    drawLeaf(branch->leaves[l], V, M);
  }
}

void drawGround(const glm::mat4 V) {
  glBindTexture(GL_TEXTURE_2D,grassTexture);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  int groundDimension = 100;
  float groundRadius = 1.0f;
  float groundVertices[] = {
    -groundRadius, 0, -groundRadius,
    -groundRadius, 0, groundRadius,
    groundRadius, 0, groundRadius,
    groundRadius, 0, -groundRadius
  };
  int groundIndexes[] = {
    0, 1, 2,
    2, 3, 0
  };
  float textureScale = 0.5f;
  float textureCoords[] = {
    0, 0,
    textureScale, 0,
    textureScale, textureScale,
    0, textureScale
  };
  float groundNormals[] = {
    0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0
  };

  for(int x = 0; x < groundDimension; x++) {
    for(int y = 0; y < groundDimension; y++) {
      glm::mat4 M=glm::translate(glm::mat4(1.0f), glm::vec3(groundRadius * 2 * x - groundDimension / groundRadius, 0.0f, groundRadius * 2 * y - groundDimension / groundRadius));
      glLoadMatrixf(glm::value_ptr(V*M));
      glVertexPointer(3,GL_FLOAT,0,groundVertices);
      glTexCoordPointer(2, GL_FLOAT, 0, textureCoords);
      glNormalPointer(GL_FLOAT, 0, groundNormals);
      glDrawElements(GL_TRIANGLES,sizeof(groundIndexes)/sizeof(int),GL_UNSIGNED_INT,groundIndexes);
    }
  }

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void drawTree(Tree *tree, const glm::mat4 V) {
  if(isFalling){
    if(windFlex == targetFlex){
      /*struct timeval tp;
      gettimeofday(&tp, NULL);
      long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
      std::srand(ms);
      targetFlex = std::rand()%15+glm::length(wind)*200;*/
      if(targetFlex < (int)(5 + glm::length(wind)*100)){
        targetFlex = (int)(5 + glm::length(wind)*100);
      }
      else{
        targetFlex -= 5;
      }
    }

    if(windFlex < targetFlex)
      windFlex += 1;
    else if(windFlex > targetFlex)
      windFlex -= 1;
  }
  for(int i = 0; i < tree->branchCount(); i++) {
    drawBranch(tree->getBranch(i), V);
  }
}

void drawLight(const glm::mat4 V) {
  glLoadMatrixf(glm::value_ptr(V));

  float light0Position[] = { 0.0f, 50.0f, 0.0f, 1.0f};
  //float light0Attenuation = 10*pow(sin(2 * M_PI * worldClock->value / 360.0f - M_PI / 4), 2);
  glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
  //glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light0Attenuation);

  float light1Ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  float light1Diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  float light1Specular[] = { 1.0, 1.0, 1.0, 1.0 };
  float light1Position[] = { 5.0f, 0.5f, 0.0f, 1.0f};

  glLightfv(GL_LIGHT1, GL_AMBIENT, light1Ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light1Position);
}

void displayFrame(void) {
  /*glm::vec3 skyColor = sky->color();
  glClearColor(skyColor.r, skyColor.g, skyColor.b, 1);*/
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_FALSE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 1, 0, -1, 1);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, skyTexture);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);
  glVertex2f(0, 0);

  glTexCoord2f(1, 0);
  glVertex2f(1, 0);

  glTexCoord2f(1, 1);
  glVertex2f(1, 1);

  glTexCoord2f(0, 1);
  glVertex2f(0, 1);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
  
  glm::mat4 P=glm::perspective(50.0f, 1.0f, 1.0f, 50.0f);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(P));

  glMatrixMode(GL_MODELVIEW);

  float altitude = tree->getTrunk()->length();
  glm::vec3 cameraTarget = glm::vec3(0.0f, altitude, 0.0f);
  glm::vec3 cameraObserver = cameraCoordinates->toCarthesian() + glm::vec3(0.0f, altitude, 0.0f);
  glm::vec3 cameraNose = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 V=glm::lookAt(cameraObserver, cameraTarget, cameraNose);
  drawGround(V);
  drawTree(tree, V);
  drawLight(V);

  glutSwapBuffers();
}

void nextFrame(void) {
  int actTime = glutGet(GLUT_ELAPSED_TIME);
  float delta = (actTime - lastTime) / 1000.f;
  lastTime = actTime;
  float angleSpeed = 1.0f;
  float zoomSpeed = 2.0f;

  if(cameraFlags[0]) // left {
    cameraCoordinates->changeAzimuth(angleSpeed * -delta);
  if(cameraFlags[1]) // right
    cameraCoordinates->changeAzimuth(angleSpeed * delta);
  if(cameraFlags[2]) // up
    cameraCoordinates->changePolar(angleSpeed * delta);
  if(cameraFlags[3]) // down
    cameraCoordinates->changePolar(angleSpeed * -delta);
  if(cameraFlags[4]) // zoom in
    cameraCoordinates->changeDistance(zoomSpeed * -delta);
  if(cameraFlags[5]) // zoom out
    cameraCoordinates->changeDistance(zoomSpeed * delta);
  if(fallingFlag){
    worldClock->reset();
    isFalling = true;
    //tree->startFalling(worldClock);
    fallingFlag = false;
  }

  worldClock->tick();
  

  glutPostRedisplay();
}

void keyDown(int c, int x, int y) {
  switch (c) {
    case CAMERA_LEFT_KEY:
      cameraFlags[0] = true;
      break;
    case CAMERA_RIGHT_KEY:
      cameraFlags[1] = true;
      break;
    case CAMERA_UP_KEY:
      cameraFlags[2] = true;
      break;
    case CAMERA_DOWN_KEY:
      cameraFlags[3] = true;
      break;
  }
}

void keyUp(int c, int x, int y) {
  switch (c) {
    case CAMERA_LEFT_KEY:
      cameraFlags[0] = false;
      break;
    case CAMERA_RIGHT_KEY:
      cameraFlags[1] = false;
      break;
    case CAMERA_UP_KEY:
      cameraFlags[2] = false;
      break;
    case CAMERA_DOWN_KEY:
      cameraFlags[3] = false;
      break;
  }
}



void letterDown(unsigned char key, int x, int y) {
  switch(key) {
    case CAMERA_ZOOMIN_KEY:
      cameraFlags[4] = true;
      break;
    case CAMERA_ZOOMOUT_KEY:
      cameraFlags[5] = true;
      break;
    case CLOCK_SPEEDUP_KEY:
      if(!speedupFlag) {
        worldClock->speedUp(CLOCK_SPEEDUP_FACTOR);
        speedupFlag = true;
      }
      break;
    case START_FALLING_KEY:
      fallingFlag = true;
      break;
  }
}

void letterUp(unsigned char key, int x, int y) {
  switch(key) {
    case CAMERA_ZOOMIN_KEY:
      cameraFlags[4] = false;
      break;
    case CAMERA_ZOOMOUT_KEY:
      cameraFlags[5] = false;
      break;
    case CLOCK_SPEEDUP_KEY:
      if(speedupFlag) {
        worldClock->slowDown(CLOCK_SPEEDUP_FACTOR);
        speedupFlag = false;
      }
      break;
  }
}

void initKeyboard() {
  glutSpecialFunc(keyDown);
  glutSpecialUpFunc(keyUp);
  glutKeyboardFunc(letterDown);
  glutKeyboardUpFunc(letterUp);
}

void initLight() {
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
}

void initTextures() {
  loadTexture("res/grass.tga", &grassTexture);
  loadTexture("res/branch.tga", &branchTexture);
  loadTexture("res/leaf.tga", &leafTexture);
  loadTexture("res/cortex.tga", &stemTexture);
  loadTexture("res/sky.tga", &skyTexture);
}

void initGlut(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800,800);
  glutInitWindowPosition(0,0);
  glutCreateWindow("Growing Tree Simulation");
  glutDisplayFunc(displayFrame);
  glutIdleFunc(nextFrame);
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
}

void initRandomness() {
  srand(time(NULL));
}

int main(int argc, char* argv[]) {
  tree->grow();
  
  initRandomness();

  initGlut(argc, argv);

  if(argc>1){
    wind.x = std::stof(std::string(argv[1]));
    wind.y = std::stof(std::string(argv[2]));
    wind.z = std::stof(std::string(argv[3]));  
  }
  

  initKeyboard();

  initLight();

  initTextures();

  glutMainLoop();

  return 0;
}
