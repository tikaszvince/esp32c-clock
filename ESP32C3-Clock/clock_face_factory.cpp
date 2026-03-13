#include "Arduino.h"
#include "clock_face_factory.h"
#include "clock_face_classic.h"
#include "clock_face_orbit.h"
#include "clock_face_bauhaus.h"
#include "clock_face_bauhaus_auto.h"

struct ClockFaceEntry {
  ClockFaceType type;
  ClockFace* instance;
};

static ClockFaceClassic classicFace;
static ClockFaceOrbit orbitFace;
static ClockFaceBauhaus bauhausLight = ClockFaceBauhaus::createLight();
static ClockFaceBauhaus bauhausDark = ClockFaceBauhaus::createDark();
static ClockFaceBauhausAuto bauhausAuto;

static ClockFaceEntry entries[] = {
  { CLOCK_FACE_CLASSIC,      &classicFace  },
  { CLOCK_FACE_ORBIT,        &orbitFace    },
  { CLOCK_FACE_BAUHAUS_LIGHT, &bauhausLight },
  { CLOCK_FACE_BAUHAUS_DARK,  &bauhausDark  },
  { CLOCK_FACE_BAUHAUS_AUTO,  &bauhausAuto  },
};

static const int FACE_COUNT = sizeof(entries) / sizeof(entries[0]);

ClockFace* getInstance(ClockFaceType type) {
  for (int i = 0; i < FACE_COUNT; i++) {
    if (entries[i].type == type) {
      return entries[i].instance;
    }
  }
  return &classicFace;
}

ClockFace* getFaceAt(int index) {
  if (index < 0 || index >= FACE_COUNT) {
    return &classicFace;
  }
  return entries[index].instance;
}

ClockFaceType getTypeAt(int index) {
  if (index < 0 || index >= FACE_COUNT) {
    return CLOCK_FACE_CLASSIC;
  }
  return entries[index].type;
}

int getFaceCount() {
  return FACE_COUNT;
}

ClockFace* getFaceById(const char* id) {
  if (id == nullptr) {
    return nullptr;
  }
  for (int i = 0; i < FACE_COUNT; i++) {
    if (strcmp(entries[i].instance->getId(), id) == 0) {
      return entries[i].instance;
    }
  }
  return nullptr;
}

int getIndexById(const char* id) {
  if (id == nullptr) {
    return -1;
  }
  for (int i = 0; i < FACE_COUNT; i++) {
    if (strcmp(entries[i].instance->getId(), id) == 0) {
      return i;
    }
  }
  return -1;
}
