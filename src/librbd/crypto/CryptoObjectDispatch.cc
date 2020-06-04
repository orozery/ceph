// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "librbd/crypto/CryptoObjectDispatch.h"
#include "common/WorkQueue.h"
#include "librbd/ImageCtx.h"

namespace librbd {
namespace crypto {

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
  return false;
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
