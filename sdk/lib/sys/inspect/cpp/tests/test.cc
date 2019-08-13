// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/io/cpp/fidl.h>
#include <lib/async/default.h>
#include <lib/fdio/directory.h>
#include <lib/fdio/fd.h>
#include <lib/fdio/fdio.h>
#include <lib/inspect/cpp/reader.h>
#include <lib/inspect/testing/cpp/inspect.h>
#include <lib/sys/cpp/testing/test_with_environment.h>
#include <zircon/device/vfs.h>

#include <src/lib/fxl/strings/substitute.h>

#include "gmock/gmock.h"
#include "src/lib/files/glob.h"

namespace {

using ::fxl::Substitute;
using sys::testing::EnclosingEnvironment;
using ::testing::UnorderedElementsAre;
using namespace inspect::testing;

constexpr char kTestComponent[] =
    "fuchsia-pkg://fuchsia.com/sys_inspect_cpp_tests#meta/"
    "sys_inspect_cpp_bin.cmx";
constexpr char kTestProcessName[] = "sys_inspect_cpp_bin.cmx";

class SysInspectTest : public sys::testing::TestWithEnvironment {
 protected:
  SysInspectTest() {
    fuchsia::sys::LaunchInfo launch_info;
    launch_info.url = kTestComponent;

    environment_ = CreateNewEnclosingEnvironment("test", CreateServices());
    environment_->CreateComponent(std::move(launch_info), controller_.NewRequest());
    bool ready = false;
    controller_.events().OnDirectoryReady = [&ready] { ready = true; };
    RunLoopUntil([&ready] { return ready; });
    if (!ready) {
      printf("The output directory is not ready\n");
    }
  }
  ~SysInspectTest() { CheckShutdown(); }

  void CheckShutdown() {
    controller_->Kill();
    bool done = false;
    controller_.events().OnTerminated = [&done](int64_t code,
                                                fuchsia::sys::TerminationReason reason) {
      ASSERT_EQ(fuchsia::sys::TerminationReason::EXITED, reason);
      done = true;
    };
    RunLoopUntil([&done] { return done; });
  }

  // Open the root object connection on the given sync pointer.
  // Returns ZX_OK on success.
  zx_status_t GetInspectVmo(zx::vmo* out_vmo) {
    files::Glob glob(Substitute("/hub/r/test/*/c/$0/*/out/objects/root.inspect", kTestProcessName));
    if (glob.size() == 0) {
      return ZX_ERR_NOT_FOUND;
    }

    fuchsia::io::FileSyncPtr file;
    zx_status_t status;
    status = fdio_open(std::string(*glob.begin()).c_str(), ZX_FS_RIGHT_READABLE,
                       file.NewRequest().TakeChannel().release());
    if (status != ZX_OK) {
      return status;
    }

    EXPECT_TRUE(file.is_bound());

    fuchsia::io::NodeInfo info;
    auto get_status = file->Describe(&info);
    if (get_status != ZX_OK) {
      printf("get failed\n");
      return get_status;
    }

    if (!info.is_vmofile()) {
      printf("not a vmofile");
      return ZX_ERR_NOT_FOUND;
    }

    *out_vmo = std::move(info.vmofile().vmo);
    return ZX_OK;
  }

 private:
  std::unique_ptr<EnclosingEnvironment> environment_;
  fuchsia::sys::ComponentControllerPtr controller_;
};

TEST_F(SysInspectTest, ReadHierarchy) {
  zx::vmo vmo;
  ASSERT_EQ(ZX_OK, GetInspectVmo(&vmo));

  auto hierarchy = inspect::ReadFromVmo(std::move(vmo)).take_value();

  EXPECT_THAT(hierarchy,
              NodeMatches(AllOf(NameMatches("root"),
                                PropertyList(UnorderedElementsAre(
                                    IntIs("val1", 1), IntIs("val2", 2), IntIs("val3", 3))))));
}

TEST_F(SysInspectTest, ReadHealth) {
  zx::vmo vmo;
  ASSERT_EQ(ZX_OK, GetInspectVmo(&vmo));

  auto hierarchy = inspect::ReadFromVmo(std::move(vmo)).take_value();

  EXPECT_THAT(hierarchy, ChildrenMatch(UnorderedElementsAre(NodeMatches(AllOf(
                             NameMatches("fuchsia.inspect.Health"),
                             PropertyList(UnorderedElementsAre(StringIs("status", "OK"))))))));
}

}  // namespace
