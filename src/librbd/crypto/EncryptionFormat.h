// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#ifndef CEPH_LIBRBD_CRYPTO_ENCRYPTION_FORMAT_H
#define CEPH_LIBRBD_CRYPTO_ENCRYPTION_FORMAT_H

#include "common/ref.h"

struct Context;

namespace librbd {
namespace crypto {

struct CryptoInterface;

template <typename ImageCtxT>
struct EncryptionFormat {
  static const std::string THIN_FORMATTED_METADATA_KEY;

  virtual ~EncryptionFormat() {
  }

  virtual EncryptionFormat<ImageCtxT>* clone() const = 0;
  virtual void format(ImageCtxT* ictx, Context* on_finish) = 0;
  virtual void load(ImageCtxT* ictx, bool* format_mismatch,
                    Context* on_finish) = 0;

  virtual CryptoInterface* get_crypto() = 0;
};

template <typename I>
const std::string EncryptionFormat<I>::THIN_FORMATTED_METADATA_KEY =
        ".rbd_encryption_thin_formatted";

} // namespace crypto
} // namespace librbd

#endif // CEPH_LIBRBD_CRYPTO_ENCRYPTION_FORMAT_H
