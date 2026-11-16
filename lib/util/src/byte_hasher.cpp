// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <util/byte_hasher.hpp>

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>

#include <cassert>
#include <memory>

namespace util
{
struct Byte_hasher::Impl
{
  XXH3_state_t state;
};

Byte_hasher::Byte_hasher()
: impl_(std::make_unique<Impl>())
{
  XXH3_INITSTATE(&impl_->state);
  reset();
}

Byte_hasher::~Byte_hasher() = default;

Byte_hasher::Byte_hasher(const Byte_hasher& other)
: impl_(std::make_unique<Impl>())
{
  XXH3_copyState(&impl_->state, &other.impl_->state);
}

Byte_hasher::Byte_hasher(Byte_hasher&& other) noexcept = default;

Byte_hasher& Byte_hasher::operator=(const Byte_hasher& other)
{
  if (this != &other)
  {
    Byte_hasher copy{other};
    swap(*this, copy);
  }

  return *this;
}

Byte_hasher& Byte_hasher::operator=(Byte_hasher&& other) noexcept = default;

void Byte_hasher::reset()
{
  [[maybe_unused]] const auto result = XXH3_64bits_reset(&impl_->state);
  assert(result == XXH_OK);
}

Hash_value Byte_hasher::digest() const
{
  return std::bit_cast<Hash_value>(XXH3_128bits_digest(&impl_->state));
}

void Byte_hasher::operator()(std::span<const std::byte> bytes)
{
  XXH3_128bits_update(&impl_->state, bytes.data(), bytes.size());
}
} // namespace util
