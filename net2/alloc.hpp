#pragma once
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace net2 {
class bucket {
 public:
  const std::size_t BlockSize;
  const std::size_t BlockCount;
  bucket(std::size_t block_size, std::size_t block_count);
  ~bucket();
  // Tests if the pointer belongs to this bucket
  auto belongs(void* ptr) const noexcept -> bool;
  // Returns nullptr if failed
  [[nodiscard]] void* allocate(std::size_t bytes) noexcept;
  void deallocate(void* ptr, std::size_t bytes) noexcept;

 private:
  // Finds n free contiguous blocks in the ledger and returns the first block’s
  // index or BlockCount on failure
  std::size_t find_contiguous_blocks(std::size_t n) const noexcept;
  // Marks n blocks in the ledger as “in-use” starting at ‘index’
  void set_blocks_in_use(std::size_t index, std::size_t n) noexcept;
  // Marks n blocks in the ledger as “free” starting at ‘index’
  void set_blocks_free(std::size_t index, std::size_t n) noexcept;
  // Actual memory for allocations
  std::byte* m_data{nullptr};
  // Reserves one bit per block to indicate whether it is in-use
  std::byte* m_ledger{nullptr};
};
}  // namespace net2
