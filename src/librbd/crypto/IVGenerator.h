// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_CRYPTO_IV_GENERATOR_H
#define CEPH_LIBRBD_CRYPTO_IV_GENERATOR_H

#include <optional>

#include "include/int_types.h"
#include "librbd/crypto/Types.h"
#include "librbd/io/Types.h"


namespace librbd {
namespace crypto {

class IVGenerator {

public:
  virtual ~IVGenerator() = default;
  virtual std::optional<io::ReadMetadata> get_required_metadata(
          const io::ReadExtents& extents) const = 0;
  virtual int get(
          unsigned char* iv, uint32_t iv_length, uint64_t image_offset,
          CipherMode mode,
          std::optional<io::ObjectMetadata>* metadata) const = 0;
};

} // namespace crypto
} // namespace librbd

#endif // CEPH_LIBRBD_CRYPTO_IV_GENERATOR_H
