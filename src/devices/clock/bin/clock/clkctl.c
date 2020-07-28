// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <fuchsia/hardware/clock/c/fidl.h>
#include <getopt.h>
#include <lib/fdio/directory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int usage(const char* cmd) {
  fprintf(stderr,
          "\nInteract with clocks on the SOC:\n"
          "   %s measure [idx]              Measures all clock values\n"
          "                                     or clock at idx if given\n"
          "   %s setrate <domain> <hz>      Set rate of clock source for domain to hz\n"
          "   %s getrate <domain>           Get current rate of clock source for domain\n"
          "   %s help                       Print this message\n"
          "\nNote: measure uses on chip measurement capabilities whereas getrate\n"
          "  calculates the rate based on hardware configuration.\n",
          cmd, cmd, cmd, cmd);
  return -1;
}

char* guess_dev(void) {
  char path[26];  // strlen("/dev/class/clock-impl/###") + 1
  DIR* d = opendir("/dev/class/clock-impl");
  if (!d) {
    return NULL;
  }

  struct dirent* de;
  while ((de = readdir(d)) != NULL) {
    if (strlen(de->d_name) != 3) {
      continue;
    }

    if (isdigit(de->d_name[0]) && isdigit(de->d_name[1]) && isdigit(de->d_name[2])) {
      sprintf(path, "/dev/class/clock-impl/%.3s", de->d_name);
      closedir(d);
      return strdup(path);
    }
  }

  closedir(d);
  return NULL;
}

int measure_clk_util(zx_handle_t ch, uint32_t idx) {
  fuchsia_hardware_clock_FrequencyInfo info;
  ssize_t rc = fuchsia_hardware_clock_DeviceMeasure(ch, idx, &info);

  if (rc < 0) {
    fprintf(stderr, "ERROR: Failed to measure clock: %zd\n", rc);
    return rc;
  }

  printf("[%4d][%4ld MHz] %s\n", idx, info.frequency, info.name);
  return 0;
}

int measure_clk(zx_handle_t ch, uint32_t idx, bool clk) {
  uint32_t num_clocks = 0;
  ssize_t rc = fuchsia_hardware_clock_DeviceGetCount(ch, &num_clocks);
  if (rc < 0) {
    fprintf(stderr, "ERROR: Failed to get num_clocks: %zd\n", rc);
    return rc;
  }

  if (clk) {
    if (idx > num_clocks) {
      fprintf(stderr, "ERROR: Invalid clock index.\n");
      return -1;
    }
    return measure_clk_util(ch, idx);
  } else {
    for (uint32_t i = 0; i < num_clocks; i++) {
      rc = measure_clk_util(ch, i);
      if (rc < 0) {
        return rc;
      }
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  const char* cmd = basename(argv[0]);
  char* path = NULL;
  bool clk = false;
  uint32_t idx = 50000000;

  // If no arguments passed, bail out after dumping
  // usage information.
  if (argc < 2) {
    return usage(cmd);
  }

  // Get the device path.
  path = guess_dev();
  if (!path) {
    fprintf(stderr, "No CLK device found.\n");
    return usage(cmd);
  }

  int fd = open(path, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "ERROR: Failed to open clock device: %d\n", fd);
    return -1;
  }

  zx_handle_t ch;
  zx_status_t status = fdio_get_service_handle(fd, &ch);
  if (status != ZX_OK) {
    fprintf(stderr, "Could not get service handle: %d\n", status);
    return status;
  }

  if (!strcmp(argv[1], "setrate")) {
    if (argc < 4) {
      return usage(cmd);
    }
    uint32_t domain = atoi(argv[2]);
    uint32_t clkfreq = atoi(argv[3]);
    uint64_t actual;
    ssize_t rc = fuchsia_hardware_clock_DeviceSetRate(ch, domain, clkfreq, &status, &actual);
    if (rc || status) {
      fprintf(stderr, "SetRate failed: rc = %zd, status = %d\n", rc, status);
      return rc;
    } else {
      printf("Clock set for domain %u successful.  Actual rate = %lu\n", domain, actual);
    }
    return 0;
  }

  if (!strcmp(argv[1], "getrate")) {
    if (argc < 3) {
      return usage(cmd);
    }
    uint32_t domain = atoi(argv[2]);
    uint64_t freq;
    ssize_t rc = fuchsia_hardware_clock_DeviceGetRate(ch, domain, &status, &freq);
    if (rc || status) {
      fprintf(stderr, "SetRate failed: rc = %zd, status = %d\n", rc, status);
      return rc;
    } else {
      printf("Clock frequency for domain[%u] = %lu\n", domain, freq);
    }
    return 0;
  }

  if (!strcmp(argv[1], "measure")) {
    if (argc == 3) {
      idx = atoi(argv[2]);
      clk = true;
    }
    return measure_clk(ch, idx, clk);
  }
  return usage(cmd);
}
