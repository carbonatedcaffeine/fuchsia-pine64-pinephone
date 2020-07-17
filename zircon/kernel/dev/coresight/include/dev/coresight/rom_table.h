// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_DEV_CORESIGHT_INCLUDE_DEV_CORESIGHT_ROM_TABLE_H_
#define ZIRCON_KERNEL_DEV_CORESIGHT_INCLUDE_DEV_CORESIGHT_ROM_TABLE_H_

#include <zircon/assert.h>

#include <utility>

#include <dev/coresight/component.h>
#include <hwreg/bitfields.h>

namespace coresight {

// [CS] D5
// A ROM table is a basic component (of type
// ComponentIDRegister::Class::k0x1ROMTable or
// ComponentIDRegister::Class::kCoreSight) that provides pointers to other
// components (including other ROM tables) in its lower registers. It is an
// organizational structure that can be used to find all CoreSight components
// on a chip (or system of chips). Thought of as a tree, the leaves are the
// system's CoreSight components and the root is typically referred to as the
// "base ROM table" (or, more plainly, "the ROM table").
class ROMTable {
 public:
  // [CS] D6.4.4
  struct Class0x1Entry : public hwreg::RegisterBase<Class0x1Entry, uint32_t> {
    DEF_FIELD(31, 12, offset);
    DEF_RSVDZ_FIELD(11, 9);
    DEF_FIELD(8, 4, powerid);
    DEF_RSVDZ_BIT(3);
    DEF_BIT(2, powerid_valid);
    DEF_BIT(1, format);
    DEF_BIT(0, present);

    static auto GetAt(uint32_t offset, uint32_t N) {
      return hwreg::RegisterAddr<Class0x1Entry>(offset + N * sizeof(uint32_t));
    }
  };

  // [CS] D7.5.17
  struct Class0x9NarrowEntry : public hwreg::RegisterBase<Class0x9NarrowEntry, uint32_t> {
    DEF_FIELD(31, 12, offset);
    DEF_RSVDZ_FIELD(11, 9);
    DEF_FIELD(8, 4, powerid);
    DEF_RSVDZ_BIT(3);
    DEF_BIT(2, powerid_valid);
    DEF_FIELD(1, 0, present);

    static auto GetAt(uint32_t offset, uint32_t N) {
      return hwreg::RegisterAddr<Class0x9NarrowEntry>(offset + N * sizeof(uint32_t));
    }
  };

  // [CS] D7.5.17
  struct Class0x9WideEntry : public hwreg::RegisterBase<Class0x9WideEntry, uint64_t> {
    DEF_FIELD(63, 12, offset);
    DEF_RSVDZ_FIELD(11, 9);
    DEF_FIELD(8, 4, powerid);
    DEF_RSVDZ_BIT(3);
    DEF_BIT(2, powerid_valid);
    DEF_FIELD(1, 0, present);

    static auto GetAt(uint32_t offset, uint32_t N) {
      return hwreg::RegisterAddr<Class0x9WideEntry>(offset + N * sizeof(uint64_t));
    }
  };

  // [CS] D7.5.10
  struct Class0x9DeviceIDRegister : public hwreg::RegisterBase<Class0x9DeviceIDRegister, uint32_t> {
    DEF_RSVDZ_FIELD(31, 6);
    DEF_BIT(5, prr);
    DEF_BIT(4, sysmem);
    DEF_FIELD(3, 0, format);

    static auto GetAt(uint32_t offset) {
      return hwreg::RegisterAddr<Class0x9DeviceIDRegister>(offset + 0xfcc);
    }
  };

  // Conceptually, we construct a ROM table from a span of bytes, the span
  // being required to contain all of the components that the table
  // transitively refers to.
  ROMTable(uintptr_t base, uint32_t span_size) : base_(base), span_size_(span_size) {}

  // The underlying tree of tables is walked with no dynamic allocation,
  // calling a ComponentCallback on each CoreSight component found, the
  // callback having a signature of uintptr_t -> void.
  template <typename IoProvider, typename ComponentCallback>
  void Walk(IoProvider io, ComponentCallback&& callback) {
    WalkFrom(io, callback, 0);
  }

 private:
  // [CS] D6.2.1, D7.2.1
  // The largest possible ROM table entry index, for various types.
  static constexpr uint32_t k0x1EntryUpperBound = 960u;
  static constexpr uint32_t k0x9NarrowEntryUpperBound = 512u;
  static constexpr uint32_t k0x9WideEntryUpperBound = 256u;

  // [CS] D7.5.10
  static constexpr uint8_t kDevidFormatNarrow = 0x0;
  static constexpr uint8_t kDevidFormatWide = 0x1;

  // There are several types of ROM table entry registers; this struct serves
  // as unified front-end for accessing their contents.
  struct EntryContents {
    uint64_t value;
    uint32_t offset;
    bool present;
  };

  template <typename IoProvider, typename ComponentCallback>
  void WalkFrom(IoProvider io, ComponentCallback&& callback, uint32_t offset) {
    const ComponentIDRegister::Class classid =
        ComponentIDRegister::GetAt(offset).ReadFrom(&io).classid();
    const DeviceArchRegister arch_reg = DeviceArchRegister::GetAt(offset).ReadFrom(&io);
    const auto architect = static_cast<const uint16_t>(arch_reg.architect());
    const auto archid = static_cast<const uint16_t>(arch_reg.archid());
    if (IsTable(classid, architect, archid)) {
      const auto format = static_cast<const uint8_t>(
          Class0x9DeviceIDRegister::GetAt(offset).ReadFrom(&io).format());

      for (uint32_t i = 0; i < EntryIndexUpperBound(classid, format); ++i) {
        EntryContents contents = ReadEntryAt(io, offset, i, classid, format);
        if (contents.value == 0) {
          break;  // Terminal entry if identically zero.
        } else if (!contents.present) {
          continue;
        }
        // [CS] D5.4
        // the offset provided by the ROM table entry requires a shift of 12 bits.
        uint32_t new_offset = offset + (contents.offset << 12);
        ZX_ASSERT_MSG(
            new_offset + kMinimumComponentSize <= span_size_,
            "referenced component does not fit within the view: (view size, offset) = (%u, %u)",
            span_size_, new_offset);
        WalkFrom(io, callback, new_offset);
      }
      return;
    }
    ZX_ASSERT_MSG(offset > 0 && classid == ComponentIDRegister::Class::kCoreSight,
                  "expected ROM table or component at offset %u: "
                  "(class, architect, archid) = (%#hhx (%s), %#x, %#x)",
                  offset, classid, ToString(classid).data(), architect, archid);
    std::forward<ComponentCallback>(callback)(base_ + offset);
  }

  bool IsTable(ComponentIDRegister::Class classid, uint16_t architect, uint16_t archid) const;

  uint32_t EntryIndexUpperBound(ComponentIDRegister::Class classid, uint8_t format) const;

  template <typename IoProvider>
  EntryContents ReadEntryAt(IoProvider io, uint32_t offset, uint32_t N,
                            ComponentIDRegister::Class classid, uint8_t format) {
    if (classid == ComponentIDRegister::Class::k0x1ROMTable) {
      auto entry = Class0x1Entry::GetAt(offset, N).ReadFrom(&io);
      return {
          .value = entry.reg_value(),
          .offset = static_cast<uint32_t>(entry.offset()),
          .present = static_cast<bool>(entry.present()),
      };
    } else if (classid == ComponentIDRegister::Class::kCoreSight) {
      // [CS] D7.5.17: only a value of 0b11 for present() signifies presence.
      switch (format) {
        case kDevidFormatNarrow: {
          auto entry = Class0x9NarrowEntry::GetAt(offset, N).ReadFrom(&io);
          return {
              .value = entry.reg_value(),
              .offset = static_cast<uint32_t>(entry.offset()),
              .present = static_cast<bool>(entry.present() & 0b11),
          };
        }
        case kDevidFormatWide: {
          auto entry = Class0x9WideEntry::GetAt(offset, N).ReadFrom(&io);
          uint64_t u32_offset = entry.offset() & 0xffffffff;
          ZX_ASSERT_MSG(
              entry.offset() == u32_offset,
              "a simplifying assumption is made that a ROM table entry's offset only contains 32 "
              "bits of information. If this is no longer true, please file a bug.");
          return {
              .value = entry.reg_value(),
              .offset = static_cast<uint32_t>(u32_offset),
              .present = static_cast<bool>(entry.present() & 0b11),
          };
        }
        default:
          ZX_PANIC("unknown DEVID.FORMAT value: %#x", format);
      }
    }
    ZX_PANIC("a ROM table cannot have a class of %#hhx (%s)", classid, ToString(classid).data());
  }

  uintptr_t base_;
  uint32_t span_size_;
};

}  // namespace coresight

#endif  // ZIRCON_KERNEL_DEV_CORESIGHT_INCLUDE_DEV_CORESIGHT_ROM_TABLE_H_