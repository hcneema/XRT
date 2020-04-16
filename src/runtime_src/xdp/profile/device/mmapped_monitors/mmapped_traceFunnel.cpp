/*
 * Copyright (C) 2020 Xilinx Inc - All rights reserved
 * Xilinx Debug & Profile (XDP) APIs
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef _WIN32

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "mmapped_traceFunnel.h"

namespace xdp {

MMappedTraceFunnel::MMappedTraceFunnel(Device* handle, uint64_t index, debug_ip_data* data)
                  : TraceFunnel(handle, index, data)
{
  // Open TraceFunnel Device Driver File
  std::string subDev("trace_funnel");
  std::string driverFileName = getDevice()->getSubDevicePath(subDev, 0 /* a design can have atmost 1 TraceFunnel*/);

  driver_FD = open(driverFileName.c_str(), O_RDWR);
  if(-1 == driver_FD) {
    showWarning("Could not open device file.");
    return;
  }

  // mmap opened device driver file 
  mapped_device = (char*)mmap(NULL, PROFILE_IP_SZ, PROT_READ | PROT_WRITE, MAP_SHARED, driver_FD, 0);
  if(mapped_device == MAP_FAILED) {
    showWarning("mmap failed for device file.");
    return;
  }
}

MMappedTraceFunnel::~MMappedTraceFunnel()
{
  munmap(mapped_device, PROFILE_IP_SZ);
  close(driver_FD);
}

bool MMappedTraceFunnel::isMMapped()
{
  if(mapped_device == nullptr || mapped_device == MAP_FAILED) {
    return false;
  }
  return true;
}

int MMappedTraceFunnel::read(uint64_t offset, size_t size, void* data)
{
  if(!isMMapped()) {
    return 0;
  }
  memcpy((char*)data, mapped_device + offset, size);
  return size;
}

int MMappedTraceFunnel::write(uint64_t offset, size_t size, void* data)
{
  if(!isMMapped()) {
    return 0;
  }
  memcpy(mapped_device + offset, (char*)data, size);
  return size;
}

}
#endif
