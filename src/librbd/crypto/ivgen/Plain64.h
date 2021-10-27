// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_CRYPTO_IVGEN_PLAIN64_H
#define CEPH_LIBRBD_CRYPTO_IVGEN_PLAIN64_H

#include "librbd/crypto/IVGenerator.h"


namespace librbd {
namespace crypto {
namespace ivgen {

class Plain64 : public IVGenerator {

public:

  std::optional<io::ReadMetadata> get_required_metadata(
          const io::ReadExtents& extents) const override {
    return std::nullopt;
  };

  int get(unsigned char* iv, uint32_t iv_length, uint64_t image_offset,
          CipherMode mode,
          std::optional<io::ObjectMetadata>* metadata) const override;
};

} // namespace ivgen
} // namespace crypto
} // namespace librbd

#endif // CEPH_LIBRBD_CRYPTO_IVGEN_PLAIN64_H
