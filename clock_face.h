#ifndef CLOCK_FACE_H
#define CLOCK_FACE_H

class ClockFace {
public:
  virtual void draw(bool blinkState) = 0;
  virtual ~ClockFace() {}
};

#endif
