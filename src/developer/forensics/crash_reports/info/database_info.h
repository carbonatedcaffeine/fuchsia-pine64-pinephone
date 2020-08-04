// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_DEVELOPER_FORENSICS_CRASH_REPORTS_INFO_DATABASE_INFO_H_
#define SRC_DEVELOPER_FORENSICS_CRASH_REPORTS_INFO_DATABASE_INFO_H_

#include <memory>

#include "src/developer/forensics/crash_reports/info/info_context.h"
#include "src/developer/forensics/utils/cobalt/metrics.h"
#include "src/developer/forensics/utils/storage_size.h"

namespace forensics {
namespace crash_reports {

// Information about the database we want to export.
struct DatabaseInfo {
 public:
  DatabaseInfo(std::shared_ptr<InfoContext> context);

  void CrashpadError(cobalt::CrashpadFunctionError function);

  void LogMaxCrashpadDatabaseSize(StorageSize max_crashpad_database_size);
  void LogGarbageCollection(uint64_t num_cleaned, uint64_t num_pruned);

 private:
  std::shared_ptr<InfoContext> context_;
};

}  // namespace crash_reports
}  // namespace forensics

#endif  // SRC_DEVELOPER_FORENSICS_CRASH_REPORTS_INFO_DATABASE_INFO_H_