// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "librbd/crypto/ivgen/Random.h"
#include "librbd/ImageCtx.h"
#include "librbd/io/Utils.h"
#include "common/dout.h"
#include "include/byteorder.h"
#include "include/ceph_assert.h"
#include <algorithm>
#include <limits>
#include <openssl/rand.h>

#define dout_subsys ceph_subsys_rbd
#undef dout_prefix
#define dout_prefix *_dout << "librbd::crypto::ivgen::Random: " << this        \
                           << " " << __func__ << ": "

namespace librbd {
namespace crypto {
namespace ivgen {

template <typename I>
const std::string Random<I>::metadata_key_for_offset(uint64_t offset) const {
  ceph_assert((offset & (m_sector_size - 1)) == 0);

  auto sector_number = offset / m_sector_size;
  ceph_assert(sector_number <= 0xffffff);

  std::string res = "iv000000";
  auto buf = res.data();

  int pos = 7;
  while (sector_number != 0) {
    uint8_t nibble = sector_number & 0xf;
    if (nibble < 10) {
      buf[pos] = '0' + nibble;
    } else {
      buf[pos] = 'a' + nibble - 10;
    }
    --pos;
    sector_number >>= 4;
  }

  return res;
}

template <typename I>
std::optional<io::ReadMetadata> Random<I>::get_required_metadata(
          const io::ReadExtents& extents) const {
  uint64_t min_offset = std::numeric_limits<uint64_t>::max();
  uint64_t max_offset = 0;
  for (auto& extent: extents) {
    min_offset = std::min(min_offset, extent.offset);
    max_offset = std::max(max_offset, extent.offset + extent.length);
  }

  if (max_offset == 0) {
    return std::nullopt;
  }

  return {{{min_offset == 0 ? "iv00000." : metadata_key_for_offset(
                                             min_offset - m_sector_size)},
           std::nullopt,
           (max_offset - min_offset) / m_sector_size}};
}

template <typename I>
int Random<I>::get(unsigned char* iv, uint32_t iv_length, uint64_t image_offset,
                 CipherMode mode,
                 std::optional<io::ObjectMetadata>* metadata) const {
  striper::LightweightObjectExtents extents;
  io::util::file_to_extents(m_image_ctx, image_offset, m_sector_size, 0,
                            &extents);
  ceph_assert(extents.size() == 1);
  auto key = metadata_key_for_offset(extents[0].offset);
  switch(mode) {
    case CIPHER_MODE_ENC: {
      if (RAND_bytes((unsigned char *) iv, iv_length) != 1) {
        lderr(m_image_ctx->cct) << "cannot generate random iv" << dendl;
        return -EAGAIN;
      }
      if (!metadata->has_value()) {
        metadata->emplace();
      }
      auto &kv = metadata->value();
      auto& bl = kv[key];
      ceph_assert(bl.length() == 0);
      bl.append(reinterpret_cast<char *>(iv), iv_length);
      break;
    }

    case CIPHER_MODE_DEC: {
      ceph_assert(metadata->has_value());
      auto &kv = metadata->value();
      ceph::buffer::list* bl;
      try {
        bl = &kv.at(key);
      } catch (std::out_of_range &) {
        lderr(m_image_ctx->cct) << "missing IV metadata for offset: "
                                << image_offset << dendl;
        return -EINVAL;
      }

      if (bl->length() != iv_length) {
        lderr(m_image_ctx->cct) << "bad IV length for offset: " << image_offset
                                << " expected: " << iv_length << " but got: "
                                << bl->length() << dendl;
        return -EINVAL;
      }

      memcpy(iv, bl->c_str(), iv_length);
      break;
    }

    default:
      ceph_assert(false);
  }

  return 0;
}

} // namespace ivgen
} // namespace crypto
} // namespace librbd

template class librbd::crypto::ivgen::Random<librbd::ImageCtx>;
