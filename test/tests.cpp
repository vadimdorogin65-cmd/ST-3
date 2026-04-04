// Copyright 2021 GHA Test Team

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <thread>

#include "TimedDoor.h"

using ::testing::NiceMock;
using ::testing::Return;

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

class DoorController {
 public:
  explicit DoorController(Door& d) : door_(d) {}
  void secure() { door_.lock(); }
  void open() { door_.unlock(); }
  bool isOpened() const { return door_.isDoorOpened(); }

 private:
  Door& door_;
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override { door_ = std::make_unique<TimedDoor>(5); }
  void TearDown() override { door_.reset(); }
  std::unique_ptr<TimedDoor> door_;
};

TEST_F(TimedDoorTest, GetTimeOut_ReturnsConstructorValue) {
  ASSERT_NE(door_, nullptr);
  EXPECT_EQ(door_->getTimeOut(), 5);
}

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, Lock_KeepsDoorClosed) {
  door_->lock();
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, Unlock_ThrowsWhenDoorStillOpenAfterTimeout) {
  TimedDoor fast_door(0);
  EXPECT_THROW(fast_door.unlock(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowState_DoesNotThrowWhenDoorIsLocked) {
  door_->lock();
  EXPECT_NO_THROW(door_->throwState());
}

TEST_F(TimedDoorTest, ThrowState_ThrowsWhenDoorIsOpen) {
  TimedDoor fast_door(0);
  EXPECT_THROW(fast_door.unlock(), std::runtime_error);
  ASSERT_TRUE(fast_door.isDoorOpened());
  EXPECT_THROW(fast_door.throwState(), std::runtime_error);
}

TEST(TimedDoorConcurrencyTest, Unlock_DoesNotThrowIfLockedBeforeTimeout) {
  TimedDoor door(1);
  std::exception_ptr err;
  std::thread worker([&]() {
    try {
      door.unlock();
    } catch (...) {
      err = std::current_exception();
    }
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  door.lock();
  worker.join();
  EXPECT_EQ(err, nullptr);
}

TEST(TimerTest, Tregister_InvokesTimeoutOnClient) {
  MockTimerClient client;
  Timer timer;
  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(0, &client);
}

TEST(TimerTest, Tregister_NullClient_DoesNotCrash) {
  Timer timer;
  EXPECT_NO_THROW(timer.tregister(0, nullptr));
}

TEST(TimerTest, Tregister_DelayZero_StrictMock_InvokesTimeoutOnce) {
  testing::StrictMock<MockTimerClient> client;
  Timer timer;
  EXPECT_CALL(client, Timeout()).Times(1);
  timer.tregister(0, &client);
}

TEST(TimerTest, RepeatedTregisterCallsTimeoutEachTime) {
  MockTimerClient mock;
  EXPECT_CALL(mock, Timeout()).Times(2);
  Timer timer;
  timer.tregister(0, &mock);
  timer.tregister(0, &mock);
}

TEST(MockDoorTest, ControllerSecure_CallsLockOnDoor) {
  MockDoor mock;
  DoorController controller(mock);
  EXPECT_CALL(mock, lock()).Times(1);
  EXPECT_CALL(mock, unlock()).Times(0);
  EXPECT_CALL(mock, isDoorOpened()).Times(0);
  controller.secure();
}

TEST(MockDoorTest, ControllerOpen_CallsUnlockOnDoor) {
  MockDoor mock;
  DoorController controller(mock);
  EXPECT_CALL(mock, unlock()).Times(1);
  EXPECT_CALL(mock, lock()).Times(0);
  controller.open();
}

TEST(MockDoorTest, ControllerIsOpened_QueriesDoorState) {
  MockDoor mock;
  DoorController controller(mock);
  EXPECT_CALL(mock, isDoorOpened()).WillOnce(::testing::Return(true));
  EXPECT_TRUE(controller.isOpened());
}

TEST(DoorTimerAdapterIntegrationTest, UnlockWithZeroTimeout_EndsWithException) {
  TimedDoor door(0);
  EXPECT_THROW(door.unlock(), std::runtime_error);
}

TEST(DoorTimerAdapterTest, Timeout_WhenDoorClosed_DoesNotThrow) {
  TimedDoor door(1);
  door.lock();
  DoorTimerAdapter adapter(door);
  EXPECT_NO_THROW(adapter.Timeout());
}

static void CloseDoor(Door& d) { d.lock(); }

TEST(MockDoorTest, LockIsInvokedThroughDoorInterface) {
  NiceMock<MockDoor> mock;
  EXPECT_CALL(mock, lock()).Times(1);
  CloseDoor(mock);
}
