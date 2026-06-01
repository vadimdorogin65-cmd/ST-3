// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
  virtual ~TimerClient() = default;
  virtual void Timeout() = 0;
};

class Door {
 public:
  virtual ~Door() = default;
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool isDoorOpened() = 0;
};

class DoorTimerAdapter : public TimerClient {
 private:
  TimedDoor& door;
 public:
  explicit DoorTimerAdapter(TimedDoor&);
  void Timeout() override;
};

class TimedDoor : public Door {
 private:
  DoorTimerAdapter* adapter;
  Timer* timer;
  int iTimeout;
  bool isOpened;
 public:
  explicit TimedDoor(int);
  ~TimedDoor() override;
  void setTimer(Timer* t);
  bool isDoorOpened() override;
  void unlock() override;
  void lock() override;
  int getTimeOut() const;
  void throwState();
};

class Timer {
  TimerClient* client = nullptr;
 protected:
  virtual void sleep(int);
 public:
  virtual ~Timer() = default;
  virtual void tregister(int, TimerClient*);
};

#endif  // INCLUDE_TIMEDDOOR_H_
