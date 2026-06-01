// Copyright 2021 GHA Test Team

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "TimedDoor.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

class MockTimer : public Timer {
 public:
  MOCK_METHOD(void, tregister, (int, TimerClient*), (override));
};

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    timer_ = std::make_unique<NiceMock<MockTimer>>();
    door_ = std::make_unique<TimedDoor>(5);
    door_->setTimer(timer_.get());
  }
  void TearDown() override {
    door_.reset();
    timer_.reset();
  }
  std::unique_ptr<NiceMock<MockTimer>> timer_;
  std::unique_ptr<TimedDoor> door_;
};

TEST_F(TimedDoorTest, DoorIsClosedAfterCreation) {
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, LockKeepsDoorClosed) {
  door_->lock();
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsConstructorValue) {
  EXPECT_EQ(door_->getTimeOut(), 5);
}

TEST_F(TimedDoorTest, UnlockOpensTheDoor) {
  EXPECT_CALL(*timer_, tregister(_, _)).Times(1);
  door_->unlock();
  EXPECT_TRUE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockRegistersTimerWithCorrectTimeout) {
  EXPECT_CALL(*timer_, tregister(5, _)).Times(1);
  door_->unlock();
}

TEST_F(TimedDoorTest, LockDoesNotTouchTimer) {
  EXPECT_CALL(*timer_, tregister(_, _)).Times(0);
  door_->lock();
}

TEST_F(TimedDoorTest, ThrowsWhenDoorStillOpenOnTimeout) {
  EXPECT_CALL(*timer_, tregister(_, _))
      .WillOnce(Invoke([](int, TimerClient* c) {
        ASSERT_NE(c, nullptr);
        c->Timeout();
      }));
  EXPECT_THROW(door_->unlock(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoesNotThrowWhenDoorClosedBeforeTimeout) {
  EXPECT_CALL(*timer_, tregister(_, _))
      .WillOnce(Invoke([this](int, TimerClient* c) {
        door_->lock();
        ASSERT_NE(c, nullptr);
        c->Timeout();
      }));
  EXPECT_NO_THROW(door_->unlock());
}

TEST_F(TimedDoorTest, ThrowStateThrowsWhenDoorOpen) {
  EXPECT_CALL(*timer_, tregister(_, _)).Times(1);
  door_->unlock();
  EXPECT_THROW(door_->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateNoThrowWhenDoorClosed) {
  door_->lock();
  EXPECT_NO_THROW(door_->throwState());
}

TEST_F(TimedDoorTest, UnlockWithoutTimerDoesNotCrash) {
  TimedDoor lonely(3);
  EXPECT_NO_THROW(lonely.unlock());
  EXPECT_TRUE(lonely.isDoorOpened());
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenDoorOpen) {
  door_->unlock();
  DoorTimerAdapter adapter(*door_);
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutNoThrowWhenDoorClosed) {
  door_->lock();
  DoorTimerAdapter adapter(*door_);
  EXPECT_NO_THROW(adapter.Timeout());
}

TEST(TimerTest, TregisterCallsTimeoutOnClient) {
  StrictMock<MockTimerClient> client;
  EXPECT_CALL(client, Timeout()).Times(1);
  Timer timer;
  timer.tregister(0, &client);
}

TEST(TimerTest, TregisterWithNullClientDoesNotCrash) {
  Timer timer;
  EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST(MockDoorTest, DoorInterfaceMethodsAreMockable) {
  NiceMock<MockDoor> door;
  EXPECT_CALL(door, lock()).Times(1);
  EXPECT_CALL(door, unlock()).Times(1);
  EXPECT_CALL(door, isDoorOpened())
      .WillOnce(Return(false))
      .WillOnce(Return(true));

  door.lock();
  door.unlock();
  EXPECT_FALSE(door.isDoorOpened());
  EXPECT_TRUE(door.isDoorOpened());
}
