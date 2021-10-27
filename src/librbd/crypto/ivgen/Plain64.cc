// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "librbd/crypto/ivgen/Plain64.h"
#include "include/byteorder.h"
#include "include/ceph_assert.h"

namespace librbd {
namespace crypto {
namespace ivgen {

int Plain64::get(unsigned char* iv, uint32_t iv_length, uint64_t image_offset,
                 CipherMode mode,
                 std::optional<io::ObjectMetadata>* metadata) const {
  ceph_assert(iv_length >= 8);
  auto sector_number = image_offset / 512;
  auto block_offset_le = ceph_le64(sector_number);
  memset(iv, 0, iv_length);
  memcpy(iv, &block_offset_le, sizeof(block_offset_le));
  return 0;
}

} // namespace ivgen
} // namespace crypto
} // namespace librbd
