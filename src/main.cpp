// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <iostream>

int main() {
  Timer timer;
  TimedDoor door(5);
  door.setTimer(&timer);

  door.lock();
  try {
    door.unlock();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}
