// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "librbd/crypto/BlockCrypto.h"
#include "include/ceph_assert.h"

#include <stdlib.h>

namespace librbd {
namespace crypto {

template <typename T>
BlockCrypto<T>::BlockCrypto(CephContext* cct, DataCryptor<T>* data_cryptor,
                            IVGenerator* iv_generator, uint64_t block_size,
                            uint64_t data_offset)
     : CryptoInterface(iv_generator), m_cct(cct),
       m_data_cryptor(data_cryptor), m_block_size(block_size),
       m_data_offset(data_offset), m_iv_size(data_cryptor->get_iv_size()) {
  ceph_assert(isp2(block_size));
  ceph_assert((block_size % data_cryptor->get_block_size()) == 0);
  ceph_assert((block_size % 512) == 0);
}

template <typename T>
BlockCrypto<T>::~BlockCrypto() {
  if (m_data_cryptor != nullptr) {
    delete m_data_cryptor;
    m_data_cryptor = nullptr;
  }
}

template <typename T>
int BlockCrypto<T>::crypt(ceph::bufferlist* data, uint64_t image_offset,
                          std::optional<io::ObjectMetadata>* metadata,
                          CipherMode mode) {
  if (image_offset % m_block_size != 0) {
    lderr(m_cct) << "image offset: " << image_offset
                 << " not aligned to block size: " << m_block_size << dendl;
    return -EINVAL;
  }
  if (data->length() % m_block_size != 0) {
    lderr(m_cct) << "data length: " << data->length()
                 << " not aligned to block size: " << m_block_size << dendl;
    return -EINVAL;
  }

  unsigned char* iv = (unsigned char*)alloca(m_iv_size);

  bufferlist src = *data;
  data->clear();

  auto ctx = m_data_cryptor->get_context(mode);
  if (ctx == nullptr) {
    lderr(m_cct) << "unable to get crypt context" << dendl;
    return -EIO;
  }
  auto appender = data->get_contiguous_appender(src.length());
  unsigned char* out_buf_ptr = nullptr;
  unsigned char* leftover_block = (unsigned char*)alloca(m_block_size);
  uint32_t leftover_size = 0;
  for (auto buf = src.buffers().begin(); buf != src.buffers().end(); ++buf) {
    auto in_buf_ptr = reinterpret_cast<const unsigned char*>(buf->c_str());
    auto remaining_buf_bytes = buf->length();
    while (remaining_buf_bytes > 0) {
      if (leftover_size == 0) {
        auto r = m_iv_generator->get(
                iv, m_iv_size, image_offset, mode, metadata);
        if (r < 0) {
          lderr(m_cct) << "unable to get cipher's IV" << dendl;
          return r;
        }
        r = m_data_cryptor->init_context(ctx, iv, m_iv_size);
        if (r != 0) {
          lderr(m_cct) << "unable to init cipher's IV" << dendl;
          return r;
        }

        out_buf_ptr = reinterpret_cast<unsigned char*>(
                appender.get_pos_add(m_block_size));
        image_offset += m_block_size;
      }

      if (leftover_size > 0 || remaining_buf_bytes < m_block_size) {
        auto copy_size = std::min(
                (uint32_t)m_block_size - leftover_size, remaining_buf_bytes);
        memcpy(leftover_block + leftover_size, in_buf_ptr, copy_size);
        in_buf_ptr += copy_size;
        leftover_size += copy_size;
        remaining_buf_bytes -= copy_size;
      }

      int crypto_output_length = 0;
      if (leftover_size == 0) {
        crypto_output_length = m_data_cryptor->update_context(
              ctx, in_buf_ptr, out_buf_ptr, m_block_size);

        in_buf_ptr += m_block_size;
        remaining_buf_bytes -= m_block_size;
      } else if (leftover_size == m_block_size) {
        crypto_output_length = m_data_cryptor->update_context(
              ctx, leftover_block, out_buf_ptr, m_block_size);
        leftover_size = 0;
      }

      if (crypto_output_length < 0) {
        lderr(m_cct) << "crypt update failed" << dendl;
        return crypto_output_length;
      }

      out_buf_ptr += crypto_output_length;
    }
  }

  m_data_cryptor->return_context(ctx, mode);

  return 0;
}

template <typename T>
int BlockCrypto<T>::encrypt(ceph::bufferlist* data, uint64_t image_offset,
                            std::optional<io::ObjectMetadata>* metadata) {
  return crypt(data, image_offset, metadata, CipherMode::CIPHER_MODE_ENC);
}

template <typename T>
int BlockCrypto<T>::decrypt(ceph::bufferlist* data, uint64_t image_offset,
                            std::optional<io::ObjectMetadata>* metadata) {
  return crypt(data, image_offset, metadata, CipherMode::CIPHER_MODE_DEC);
}

} // namespace crypto
} // namespace librbd

template class librbd::crypto::BlockCrypto<EVP_CIPHER_CTX>;
