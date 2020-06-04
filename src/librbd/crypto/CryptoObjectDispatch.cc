// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "librbd/crypto/CryptoObjectDispatch.h"
#include "common/WorkQueue.h"
#include "include/Context.h"
#include "librbd/ImageCtx.h"
#include "librbd/io/AioCompletion.h"
#include "librbd/io/ObjectDispatchSpec.h"
#include "librbd/io/ReadResult.h"

namespace librbd {
namespace crypto {

static const unsigned BLOCK_SIZE = 4096;

struct C_EncryptedObjectReadRequest : public Context {

    io::ReadResult::C_ObjectReadRequest *backing_request;
    uint64_t object_off;
    uint64_t object_len;

    C_EncryptedObjectReadRequest(
            io::AioCompletion *aio_completion, uint64_t object_off, uint64_t object_len) {
      this->object_off = object_off;
      this->object_len = object_len;

      // pre-align
      unsigned pre_aligned = object_off % BLOCK_SIZE;
      uint64_t aligned_off = object_off - pre_aligned;
      uint64_t aligned_len = object_len + pre_aligned;

      // post-align
      aligned_len += (BLOCK_SIZE - (aligned_len % BLOCK_SIZE)) % BLOCK_SIZE;

      backing_request = new io::ReadResult::C_ObjectReadRequest(
              aio_completion, aligned_off, aligned_len, {{0, object_len}});
    }

    void finish(int r) override {
      if (r == 0) {
        bufferlist bl_copy(std::move(backing_request->bl));
        printf("QQQ bl_copy_length=%d, offset=%ld, length=%ld\n",
               bl_copy.length(), object_off - backing_request->object_off, object_len);
        backing_request->bl.substr_of(bl_copy,
                                     object_off - backing_request->object_off,
                                     object_len);
      }
      backing_request->finish(r);
    }
};

template <typename I>
CryptoObjectDispatch<I>::CryptoObjectDispatch(
    I* image_ctx)
  : m_image_ctx(image_ctx) {
}

template <typename I>
void CryptoObjectDispatch<I>::shut_down(Context* on_finish) {
  m_image_ctx->op_work_queue->queue(on_finish, 0);
}

template <typename I>
bool CryptoObjectDispatch<I>::read(
    uint64_t object_no, uint64_t object_off, uint64_t object_len,
    librados::snap_t snap_id, int op_flags, const ZTracer::Trace &parent_trace,
    ceph::bufferlist* read_data, io::ExtentMap* extent_map,
    int* object_dispatch_flags, io::DispatchResult* dispatch_result,
    Context** on_finish, Context* on_dispatched) {
  printf("read object_no=%ld, off=%ld, len=%ld\n", object_no, object_off, object_len);

  // re-use standard object read state machine
  auto aio_comp = io::AioCompletion::create_and_start(*on_finish, m_image_ctx,
                                                      io::AIO_TYPE_READ);
  aio_comp->read_result = io::ReadResult{read_data};
  aio_comp->set_request_count(1);

  auto req_comp = new C_EncryptedObjectReadRequest(
          aio_comp, object_off, object_len);

  *dispatch_result = io::DISPATCH_RESULT_COMPLETE;
  auto req = io::ObjectDispatchSpec::create_read(
          m_image_ctx, io::OBJECT_DISPATCH_LAYER_CRYPTO, object_no,
          req_comp->backing_request->object_off, req_comp->backing_request->object_len, snap_id,
          op_flags, parent_trace, &req_comp->backing_request->bl, &req_comp->backing_request->extent_map, req_comp);
  req->send();
  return true;
}

template <typename I>
bool CryptoObjectDispatch<I>::write(
    uint64_t object_no, uint64_t object_off, ceph::bufferlist&& data,
    const ::SnapContext &snapc, int op_flags,
    const ZTracer::Trace &parent_trace, int* object_dispatch_flags,
    uint64_t* journal_tid, io::DispatchResult* dispatch_result,
    Context** on_finish, Context* on_dispatched) {
  printf("write object_no=%ld, off=%ld, len=%d\n", object_no, object_off, data.length());
  return false;
}

template <typename I>
bool CryptoObjectDispatch<I>::write_same(
    uint64_t object_no, uint64_t object_off, uint64_t object_len,
    io::LightweightBufferExtents&& buffer_extents, ceph::bufferlist&& data,
    const ::SnapContext &snapc, int op_flags,
    const ZTracer::Trace &parent_trace, int* object_dispatch_flags,
    uint64_t* journal_tid, io::DispatchResult* dispatch_result,
    Context** on_finish, Context* on_dispatched) {
  printf("write_same object_no=%ld, off=%ld\n", object_no, object_off);
  return false;
}

template <typename I>
bool CryptoObjectDispatch<I>::compare_and_write(
    uint64_t object_no, uint64_t object_off, ceph::bufferlist&& cmp_data,
    ceph::bufferlist&& write_data, const ::SnapContext &snapc, int op_flags,
    const ZTracer::Trace &parent_trace, uint64_t* mismatch_offset,
    int* object_dispatch_flags, uint64_t* journal_tid,
    io::DispatchResult* dispatch_result, Context** on_finish,
    Context* on_dispatched) {
  printf("compare_and_write object_no=%ld, off=%ld, len=%d\n", object_no, object_off, write_data.length());
  return false;
}

} // namespace crypto
} // namespace librbd

template class librbd::crypto::CryptoObjectDispatch<librbd::ImageCtx>;
