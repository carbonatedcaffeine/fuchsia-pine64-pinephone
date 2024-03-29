// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/scenic/lib/flatland/flatland_manager.h"

#include <lib/syslog/cpp/macros.h>

#include <gtest/gtest.h>

#include "fuchsia/ui/scenic/internal/cpp/fidl.h"
#include "lib/gtest/real_loop_fixture.h"
#include "src/ui/scenic/lib/flatland/renderer/mocks/mock_buffer_collection_importer.h"
#include "src/ui/scenic/lib/flatland/tests/mock_flatland_presenter.h"
#include "src/ui/scenic/lib/scheduling/frame_scheduler.h"
#include "src/ui/scenic/lib/scheduling/id.h"

using ::testing::_;
using ::testing::Return;

using flatland::FlatlandManager;
using flatland::FlatlandPresenter;
using flatland::LinkSystem;
using flatland::MockFlatlandPresenter;
using flatland::UberStructSystem;
using fuchsia::ui::scenic::internal::Flatland;
using fuchsia::ui::scenic::internal::Flatland_Present_Result;

// These macros works like functions that check a variety of conditions, but if those conditions
// fail, the line number for the failure will appear in-line rather than in a function.

// This macro calls Present() on a Flatland object and immediately triggers the session update
// for all sessions so that changes from that Present() are visible in global systems. This is
// primarily useful for testing the user-facing Flatland API.
//
// This macro must be used within a test using the FlatlandManagerTest harness.
//
// |flatland| is a Flatland object constructed with the MockFlatlandPresenter owned by the
// FlatlandManagerTest harness. |session_id| is the SessionId for |flatland|. |expect_success|
// should be false if the call to Present() is expected to trigger an error.
#define PRESENT(flatland, session_id, expect_success)                                    \
  {                                                                                      \
    if (expect_success) {                                                                \
      EXPECT_CALL(*mock_flatland_presenter_, RegisterPresent(session_id, _));            \
      EXPECT_CALL(*mock_flatland_presenter_, ScheduleUpdateForSession(_, _));            \
    }                                                                                    \
    bool processed_callback = false;                                                     \
    flatland->Present(/*requested_presentation_time=*/0, /*acquire_fences=*/{},          \
                      /*release_fences=*/{}, [&](Flatland_Present_Result result) {       \
                        EXPECT_EQ(!expect_success, result.is_err());                     \
                        if (expect_success) {                                            \
                          EXPECT_EQ(1u, result.response().num_presents_remaining);       \
                        } else {                                                         \
                          EXPECT_EQ(fuchsia::ui::scenic::internal::Error::BAD_OPERATION, \
                                    result.err());                                       \
                        }                                                                \
                        processed_callback = true;                                       \
                      });                                                                \
    /* Wait for the worker thread to process the request. */                             \
    RunLoopUntil([this, session_id] { return HasSessionUpdate(session_id); });           \
    /* Trigger the Present callback on the test looper. */                               \
    RunLoopUntilIdle();                                                                  \
    EXPECT_TRUE(processed_callback);                                                     \
    ApplySessionUpdates();                                                               \
  }

namespace {

class FlatlandManagerTest : public gtest::RealLoopFixture {
 public:
  FlatlandManagerTest()
      : uber_struct_system_(std::make_shared<UberStructSystem>()),
        link_system_(std::make_shared<LinkSystem>(uber_struct_system_->GetNextInstanceId())) {}

  void SetUp() override {
    gtest::RealLoopFixture::SetUp();

    mock_flatland_presenter_ = new ::testing::StrictMock<MockFlatlandPresenter>();

    ON_CALL(*mock_flatland_presenter_, RegisterPresent(_, _))
        .WillByDefault(::testing::Invoke(
            [&](scheduling::SessionId session_id, std::vector<zx::event> release_fences) {
              EXPECT_TRUE(release_fences.empty());

              const auto next_present_id = scheduling::GetNextPresentId();

              pending_presents_.insert({session_id, next_present_id});

              return next_present_id;
            }));

    ON_CALL(*mock_flatland_presenter_, ScheduleUpdateForSession(_, _))
        .WillByDefault(::testing::Invoke(
            [&](zx::time requested_presentation_time, scheduling::SchedulingIdPair id_pair) {
              // The ID must be already registered.
              EXPECT_TRUE(pending_presents_.count(id_pair));

              // Ensure IDs are strictly increasing.
              auto current_id_kv = pending_session_updates_.find(id_pair.session_id);
              EXPECT_TRUE(current_id_kv == pending_session_updates_.end() ||
                          current_id_kv->second < id_pair.present_id);

              // Only save the latest PresentId: the UberStructSystem will flush all Presents prior
              // to it.
              pending_session_updates_[id_pair.session_id] = id_pair.present_id;
            }));

    flatland_presenter_ = std::shared_ptr<FlatlandPresenter>(mock_flatland_presenter_);

    manager_ = std::make_unique<FlatlandManager>(
        flatland_presenter_, uber_struct_system_, link_system_,
        std::vector<std::shared_ptr<flatland::BufferCollectionImporter>>());
  }

  void TearDown() override {
    // Triggers cleanup of manager resources for Flatland instances that have exited.
    RunLoopUntilIdle();

    // |manager_| may have been reset during the test.
    if (manager_) {
      EXPECT_EQ(manager_->GetSessionCount(), 0ul);
    }

    auto snapshot = uber_struct_system_->Snapshot();
    EXPECT_TRUE(snapshot.empty());

    manager_.reset();
    flatland_presenter_.reset();

    gtest::RealLoopFixture::TearDown();
  }

  // Applies the most recently scheduled session update for each session.
  void ApplySessionUpdates() {
    uber_struct_system_->UpdateSessions(pending_session_updates_);
    pending_presents_.clear();
    pending_session_updates_.clear();
  }

  // Returns true if |session_id| currently has a session update pending.
  bool HasSessionUpdate(scheduling::SessionId session_id) const {
    return pending_session_updates_.count(session_id);
  }

 protected:
  ::testing::StrictMock<MockFlatlandPresenter>* mock_flatland_presenter_;
  const std::shared_ptr<UberStructSystem> uber_struct_system_;

  std::unique_ptr<FlatlandManager> manager_;

 private:
  std::shared_ptr<FlatlandPresenter> flatland_presenter_;
  const std::shared_ptr<LinkSystem> link_system_;

  // Storage for |mock_flatland_presenter_|.
  std::unordered_set<scheduling::SchedulingIdPair> pending_presents_;
  std::unordered_map<scheduling::SessionId, scheduling::PresentId> pending_session_updates_;
};

}  // namespace

namespace flatland {
namespace test {

TEST_F(FlatlandManagerTest, CreateFlatlands) {
  fidl::InterfacePtr<fuchsia::ui::scenic::internal::Flatland> flatland1;
  manager_->CreateFlatland(flatland1.NewRequest());

  fidl::InterfacePtr<fuchsia::ui::scenic::internal::Flatland> flatland2;
  manager_->CreateFlatland(flatland2.NewRequest());

  RunLoopUntilIdle();

  EXPECT_TRUE(flatland1.is_bound());
  EXPECT_TRUE(flatland2.is_bound());

  EXPECT_EQ(manager_->GetSessionCount(), 2ul);
}

TEST_F(FlatlandManagerTest, ManagerDiesBeforeClients) {
  fidl::InterfacePtr<fuchsia::ui::scenic::internal::Flatland> flatland;
  manager_->CreateFlatland(flatland.NewRequest());

  RunLoopUntilIdle();

  EXPECT_TRUE(flatland.is_bound());
  EXPECT_EQ(manager_->GetSessionCount(), 1ul);

  // Explicitly kill the server.
  manager_.reset();

  RunLoopUntilIdle();

  EXPECT_FALSE(flatland.is_bound());
}

TEST_F(FlatlandManagerTest, FlatlandsPublishToSharedUberStructSystem) {
  fidl::InterfacePtr<fuchsia::ui::scenic::internal::Flatland> flatland1;
  manager_->CreateFlatland(flatland1.NewRequest());
  const scheduling::SessionId id1 = uber_struct_system_->GetLatestInstanceId();

  fidl::InterfacePtr<fuchsia::ui::scenic::internal::Flatland> flatland2;
  manager_->CreateFlatland(flatland2.NewRequest());
  const scheduling::SessionId id2 = uber_struct_system_->GetLatestInstanceId();

  RunLoopUntilIdle();

  // Both instances publish to the shared UberStructSystem.
  PRESENT(flatland1, id1, true);
  PRESENT(flatland2, id2, true);

  auto snapshot = uber_struct_system_->Snapshot();
  EXPECT_EQ(snapshot.size(), 2ul);
}

#undef PRESENT

}  // namespace test
}  // namespace flatland
