#ifndef CLOCK_H
#define CLOCK_H

class Clock {
  public:
    Clock();
    void tick();
    float from(float time);
    void speedUp(float factor);
    void slowDown(float factor);
    void reset();
    float getSpeed();
    float value;
  private:
    float speed;
};

#endif // CLOCK_H
