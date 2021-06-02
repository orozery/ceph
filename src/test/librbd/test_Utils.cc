// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "test/librbd/test_fixture.h"
#include "test/librbd/test_support.h"
#include "librbd/ImageCtx.h"
#include "librbd/Utils.h"

using librbd::util::data_object_name;

void register_test_utils() {
}

struct TestUtils : public TestFixture {
    librbd::ImageCtx *ictx;

    void SetUp() override {
      TestFixture::SetUp();
      ASSERT_EQ(0, open_image(m_image_name, &ictx));
    }

    void TearDown() override {
      close_image(ictx);
      TestFixture::TearDown();
    }

    void expect_object_name(uint64_t object_no,
                            const std::string& expected_object_name_postfix) {
      EXPECT_STREQ(
              (ictx->object_prefix + std::string(".") +
               expected_object_name_postfix).c_str(),
              data_object_name(ictx, object_no).c_str());
    }
};

TEST_F(TestUtils, test_data_object_name) {
  ictx->old_format = false;
  expect_object_name(0, "0000000000000000");
  expect_object_name(1, "0000000000000001");
  expect_object_name(0x123456789abcdef, "0123456789abcdef");
  expect_object_name(0x8000000000000000, "8000000000000000");
  expect_object_name(0x8000000000000001, "8000000000000001");
  expect_object_name(0xffffffffffffffff, "ffffffffffffffff");
  ictx->old_format = true;
  expect_object_name(0, "000000000000");
  expect_object_name(1, "000000000001");
  expect_object_name(0x123456789ab, "0123456789ab");
  expect_object_name(0x800000000000, "800000000000");
  expect_object_name(0x800000000001, "800000000001");
  expect_object_name(0xffffffffffff, "ffffffffffff");
}

