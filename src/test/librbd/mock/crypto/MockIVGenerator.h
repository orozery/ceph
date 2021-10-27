// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_TEST_LIBRBD_MOCK_CRYPTO_MOCK_IV_GENERATOR_H
#define CEPH_TEST_LIBRBD_MOCK_CRYPTO_MOCK_IV_GENERATOR_H

#include "include/buffer.h"
#include "gmock/gmock.h"
#include "librbd/crypto/IVGenerator.h"

namespace librbd {
namespace crypto {

struct MockIVGenerator : IVGenerator {

  MOCK_CONST_METHOD1(get_required_metadata,
                     std::optional<io::ReadMetadata>(const io::ReadExtents&));
  MOCK_CONST_METHOD5(get, int(unsigned char*, uint32_t, uint64_t, CipherMode,
                              std::optional<io::ObjectMetadata>*));
};

} // namespace crypto
} // namespace librbd

#endif // CEPH_TEST_LIBRBD_MOCK_CRYPTO_MOCK_IV_GENERATOR_H
