/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/trace_processor/importers/perf/reader.h"

#include <stddef.h>
#include <cstdint>
#include <vector>

#include "perfetto/base/endian.h"
#include "perfetto/trace_processor/trace_blob.h"
#include "perfetto/trace_processor/trace_blob_view.h"
#include "test/gtest_and_gmock.h"

namespace perfetto::trace_processor::perf_importer {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::SizeIs;

template <typename T>
TraceBlobView TraceBlobViewFromVector(std::vector<T> nums) {
  for (T& n : nums) {
    n = base::HostToLE(n);
  }
  size_t data_size = sizeof(T) * nums.size();
  auto blob = TraceBlob::Allocate(data_size);
  memcpy(blob.mutable_data(), nums.data(), data_size);
  return TraceBlobView(std::move(blob));
}

TEST(ReaderUnittest, Read) {
  TraceBlobView tbv = TraceBlobViewFromVector(std::vector<uint64_t>{2, 4, 8});
  Reader reader(std::move(tbv), ByteOrder::kLittleEndian);
  uint64_t val;
  reader.Read(val);
  EXPECT_EQ(val, 2u);
}

TEST(ReaderUnittest, ReadOptional) {
  TraceBlobView tbv = TraceBlobViewFromVector(std::vector<uint64_t>{2, 4, 8});
  Reader reader(std::move(tbv), ByteOrder::kLittleEndian);
  std::optional<uint64_t> val;
  reader.ReadOptional(val);
  EXPECT_EQ(val, 2u);
}

TEST(ReaderUnittest, ReadVector) {
  TraceBlobView tbv =
      TraceBlobViewFromVector(std::vector<uint64_t>{2, 4, 8, 16, 32});
  Reader reader(std::move(tbv), ByteOrder::kLittleEndian);

  std::vector<uint64_t> res(3);
  reader.ReadVector(res);

  std::vector<uint64_t> valid{2, 4, 8};
  EXPECT_EQ(res, valid);
}

TEST(ReaderUnittest, Skip) {
  TraceBlobView tbv = TraceBlobViewFromVector(std::vector<uint64_t>{2, 4, 8});
  Reader reader(std::move(tbv), ByteOrder::kLittleEndian);

  reader.Skip<uint64_t>();

  uint64_t val;
  reader.Read(val);
  EXPECT_EQ(val, 4u);
}

void Append(std::vector<uint8_t>& out,
            uint64_t value,
            size_t size,
            ByteOrder order) {
  for (size_t i = 0; i < size; ++i) {
    size_t shift = order == ByteOrder::kLittleEndian ? i : size - 1 - i;
    out.push_back(static_cast<uint8_t>(value >> (8 * shift)));
  }
}

TraceBlobView TraceBlobViewFromBytes(const std::vector<uint8_t>& bytes) {
  auto blob = TraceBlob::Allocate(bytes.size());
  memcpy(blob.mutable_data(), bytes.data(), bytes.size());
  return TraceBlobView(std::move(blob));
}

TEST(ReaderUnittest, ReadBigEndian) {
  std::vector<uint8_t> bytes;
  Append(bytes, 0x0102030405060708, 8, ByteOrder::kBigEndian);
  Append(bytes, 0x0a0b, 2, ByteOrder::kBigEndian);
  Reader reader(TraceBlobViewFromBytes(bytes), ByteOrder::kBigEndian);
  uint64_t u64;
  uint16_t u16;
  ASSERT_TRUE(reader.Read(u64));
  ASSERT_TRUE(reader.Read(u16));
  EXPECT_EQ(u64, 0x0102030405060708u);
  EXPECT_EQ(u16, 0x0a0bu);
}

// Encodes the flags bit-fields the way a compiler for |order| lays them out.
uint64_t AttrFlags(ByteOrder order) {
  constexpr int kFreqBit = 10;
  constexpr int kPreciseIpBit = 15;
  constexpr int kPreciseIpWidth = 2;
  constexpr int kSampleIdAllBit = 18;
  if (order == ByteOrder::kLittleEndian) {
    return (1ull << kFreqBit) | (2ull << kPreciseIpBit) |
           (1ull << kSampleIdAllBit);
  }
  return (1ull << (63 - kFreqBit)) |
         (2ull << (64 - kPreciseIpBit - kPreciseIpWidth)) |
         (1ull << (63 - kSampleIdAllBit));
}

TEST(ReaderUnittest, ReadPerfEventAttr) {
  for (ByteOrder order : {ByteOrder::kLittleEndian, ByteOrder::kBigEndian}) {
    std::vector<uint8_t> bytes;
    Append(bytes, PERF_TYPE_SOFTWARE, 4, order);  // type
    Append(bytes, 64, 4, order);                  // size
    Append(bytes, 0x1234, 8, order);              // config
    Append(bytes, 4000, 8, order);                // sample_freq
    Append(bytes, 0, 8, order);                   // sample_type
    Append(bytes, 0, 8, order);                   // read_format
    Append(bytes, AttrFlags(order), 8, order);    // flags
    Append(bytes, 0, 8, order);                   // wakeup_events, bp_type
    perf_event_attr attr;
    Reader reader(TraceBlobViewFromBytes(bytes), order);
    ASSERT_TRUE(reader.ReadPerfEventAttr(attr, bytes.size()));
    EXPECT_EQ(attr.type, PERF_TYPE_SOFTWARE);
    EXPECT_EQ(attr.size, 64u);
    EXPECT_EQ(attr.config, 0x1234u);
    EXPECT_EQ(attr.sample_freq, 4000u);
    EXPECT_EQ(attr.freq, 1u);
    EXPECT_EQ(attr.precise_ip, 2u);
    EXPECT_EQ(attr.sample_id_all, 1u);
    EXPECT_EQ(attr.disabled, 0u);
    EXPECT_EQ(attr.mmap_data, 0u);
  }
}

}  // namespace
}  // namespace perfetto::trace_processor::perf_importer
