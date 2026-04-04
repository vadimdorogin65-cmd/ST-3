// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <chrono>
#include <stdexcept>
#include <thread>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
  if (door.isDoorOpened()) {
    door.throwState();
  }
}

TimedDoor::TimedDoor(int timeout)
    : adapter(new DoorTimerAdapter(*this)),
      iTimeout(timeout),
      isOpened(false) {}

TimedDoor::~TimedDoor() {
  delete adapter;
  adapter = nullptr;
}

bool TimedDoor::isDoorOpened() { return isOpened; }

void TimedDoor::lock() { isOpened = false; }

void TimedDoor::unlock() {
  isOpened = true;
  Timer timer;
  timer.tregister(iTimeout, adapter);
}

int TimedDoor::getTimeOut() const { return iTimeout; }

void TimedDoor::throwState() {
  if (isOpened) {
    throw std::runtime_error("Door has been left open too long");
  }
}

void Timer::sleep(int delay) {
  if (delay <= 0) {
    return;
  }
  std::this_thread::sleep_for(std::chrono::seconds(delay));
}

void Timer::tregister(int delay, TimerClient* c) {
  client = c;
  if (client == nullptr) {
    return;
  }
  sleep(delay);
  client->Timeout();
}
