#pragma once
#include <cstdint>
#include <map>
#include <mutex>
namespace deadspace {
// Diagnostic guest arena, separate from the image, loader data and stack.
class VirtualMemory {
 public:
  explicit VirtualMemory(uint8_t* base):base_(base){}
  uint32_t allocate(uint32_t& address,uint32_t& size,uint32_t flags,uint32_t protection,uint32_t debug);
  uint32_t release(uint32_t& address,uint32_t& size,uint32_t flags,uint32_t debug);
 private:
  uint8_t* base_;
  std::map<uint32_t,uint32_t> reservations_;
  std::map<uint32_t,uint32_t> pageSizes_;
  std::mutex mutex_;
};
}
