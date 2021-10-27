// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_CRYPTO_IVGEN_RANDOM_H
#define CEPH_LIBRBD_CRYPTO_IVGEN_RANDOM_H

#include "librbd/crypto/IVGenerator.h"


namespace librbd {

class ImageCtx;

namespace crypto {
namespace ivgen {

template <typename I>
class Random : public IVGenerator {
public:
  Random(I* image_ctx, uint32_t sector_size) : m_image_ctx(image_ctx),
                                               m_sector_size(sector_size) {
  }

  std::optional<io::ReadMetadata> get_required_metadata(
          const io::ReadExtents& extents) const override;

  int get(unsigned char* iv, uint32_t iv_length, uint64_t image_offset,
          CipherMode mode,
          std::optional<io::ObjectMetadata>* metadata) const override;

private:
  const std::string metadata_key_for_offset(uint64_t offset) const;

  I* m_image_ctx;
  uint64_t m_sector_size;
};

} // namespace ivgen
} // namespace crypto
} // namespace librbd

extern template class librbd::crypto::ivgen::Random<librbd::ImageCtx>;

#endif // CEPH_LIBRBD_CRYPTO_IVGEN_RANDOM_H
