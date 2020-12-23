#include <bitset>
#include <vector>

#include <napi.h>

#include "Common/BitUtils.h"

#include "Core/HW/AddressSpace.h"
#include "Core/HW/Memmap.h"
#include "Core/PowerPC/JitInterface.h"

#include "DolphinNode/Js/TypeConv.h"

namespace Js::Memory {

struct JitInterface : Napi::ObjectWrap<JitInterface> {

JitInterface(const Napi::CallbackInfo& info) :
  Napi::ObjectWrap<JitInterface>{info}
{}

static Napi::FunctionReference constructor;

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope{env};

  Napi::Function func =
  DefineClass(env, "JitInterface", {
    StaticMethod("invalidateICache", &JitInterface::InvalidateICache)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("JitInterface", func);

  return exports;
}

static Napi::Value InvalidateICache(const Napi::CallbackInfo& info) {
  ::JitInterface::InvalidateICache(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // size
    TypeConv::AsBool(info[2]) // forced
  );

  return info.Env().Undefined();
}

};

Napi::FunctionReference JitInterface::constructor;

namespace Helpers {

static Napi::Value ReadBufferU8(Napi::Env env, u32 address, u32 size, std::function<u8(u32)> read_fn) {
  std::vector<u8> buf(size);
  for (u32 i{}; i < size; ++i)
    buf[i] = read_fn(address + i);

  return Napi::Buffer<u8>::Copy(env, buf.data(), buf.size());
}

static Napi::Value ReadBitU8(Napi::Env env, u32 address, u32 bit_offset, std::function<u8(u32)> read_fn) {
  return TypeConv::FromBool(env, std::bitset<8>{read_fn(address)}.test(7 - bit_offset));
}

static Napi::Value ReadBitsU8(Napi::Env env, u32 address, std::function<u8(u32)> read_fn) {
  auto data = Napi::Uint8Array::New(env, 8);

  std::bitset<8> bitset{read_fn(address)};
  for (u32 j{}; j < 8; ++j)
    data.Set(j, TypeConv::FromU8(env, bitset.test(7 - j)));

  return data;
}

static Napi::Value ReadBitsBufferU8(Napi::Env env, u32 address, u32 size, std::function<u8(u32)> read_fn) {
  auto data = Napi::Uint8Array::New(env, size * 8);

  for (u32 i{}; i < size; ++i) {
    std::bitset<8> bitset{read_fn(address + i)};
    for (u32 j{}; j < 8; ++j)
      data.Set(i * 8 + j, TypeConv::FromU8(env, bitset.test(7 - j)));
  }

  return data;
}

static void WriteBufferU8(u32 address, Napi::Uint8Array data, std::function<void(u32, u8)> write_fn) {
  for (u32 i{}; i < data.ElementLength(); ++i)
    write_fn(address + i, TypeConv::AsU8(data.Get(i)));
}

static void WriteBitU8(u32 address, u32 bit_offset, bool set, std::function<u8(u32)> read_fn, std::function<void(u32, u8)> write_fn) {
  std::bitset<8> bitset{read_fn(address)};
  bitset.set(7 - bit_offset, set);

  write_fn(address, static_cast<u8>(bitset.to_ulong()));
}

static void WriteBitsU8(u32 address, Napi::Uint8Array data, std::function<void(u32, u8)> write_fn) {
  std::bitset<8> bitset;
  for (u32 j{}; j < 8; ++j)
    bitset.set(7 - j, TypeConv::ToBool(data.Get(j)));

  write_fn(address, static_cast<u8>(bitset.to_ulong()));
}

static void WriteBitsBufferU8(u32 address, Napi::Uint8Array data, std::function<void(u32, u8)> write_fn) {
  for (u32 i{}; i < data.ElementLength() / 8; ++i) {
    std::bitset<8> bitset;
    for (u32 j{}; j < 8; ++j)
      bitset.set(7 - j, TypeConv::ToBool(data.Get(i * 8 + j)));

    write_fn(address + i, static_cast<u8>(bitset.to_ulong()));
  }
}

}

struct Memmap : Napi::ObjectWrap<Memmap> {

Memmap(const Napi::CallbackInfo& info) :
  Napi::ObjectWrap<Memmap>{info}
{}

static Napi::FunctionReference constructor;

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope{env};

  Napi::Function func =
  DefineClass(env, "Memmap", {
    StaticMethod("clear", &Memmap::Clear),
    StaticMethod("memset", &Memmap::Memset),

    StaticMethod("readU8", &Memmap::ReadU8),
    StaticMethod("readU16LE", &Memmap::ReadU16LE),
    StaticMethod("readU32LE", &Memmap::ReadU32LE),
    StaticMethod("readU64LE", &Memmap::ReadU64LE),
    StaticMethod("readF32LE", &Memmap::ReadF32LE),
    StaticMethod("readF64LE", &Memmap::ReadF64LE),
    StaticMethod("readU16BE", &Memmap::ReadU16BE),
    StaticMethod("readU32BE", &Memmap::ReadU32BE),
    StaticMethod("readU64BE", &Memmap::ReadU64BE),
    StaticMethod("readF32BE", &Memmap::ReadF32BE),
    StaticMethod("readF64BE", &Memmap::ReadF64BE),
    StaticMethod("readS8", &Memmap::ReadS8),
    StaticMethod("readS16LE", &Memmap::ReadS16LE),
    StaticMethod("readS32LE", &Memmap::ReadS32LE),
    StaticMethod("readS64LE", &Memmap::ReadS64LE),
    StaticMethod("readS16BE", &Memmap::ReadS16BE),
    StaticMethod("readS32BE", &Memmap::ReadS32BE),
    StaticMethod("readS64BE", &Memmap::ReadS64BE),
    StaticMethod("readBufferU8", &Memmap::ReadBufferU8),
    StaticMethod("readBitU8", &Memmap::ReadBitU8),
    StaticMethod("readBitsU8", &Memmap::ReadBitsU8),
    StaticMethod("readBitsBufferU8", &Memmap::ReadBitsBufferU8),
    StaticMethod("writeU8", &Memmap::WriteU8),
    StaticMethod("writeU16LE", &Memmap::WriteU16LE),
    StaticMethod("writeU32LE", &Memmap::WriteU32LE),
    StaticMethod("writeU64LE", &Memmap::WriteU64LE),
    StaticMethod("writeF32LE", &Memmap::WriteF32LE),
    StaticMethod("writeF64LE", &Memmap::WriteF64LE),
    StaticMethod("writeU16BE", &Memmap::WriteU16BE),
    StaticMethod("writeU32BE", &Memmap::WriteU32BE),
    StaticMethod("writeU64BE", &Memmap::WriteU64BE),
    StaticMethod("writeF32BE", &Memmap::WriteF32BE),
    StaticMethod("writeF64BE", &Memmap::WriteF64BE),
    StaticMethod("writeBufferU8", &Memmap::WriteBufferU8),
    StaticMethod("writeBitU8", &Memmap::WriteBitU8),
    StaticMethod("writeBitsU8", &Memmap::WriteBitsU8),
    StaticMethod("writeBitsBufferU8", &Memmap::WriteBitsBufferU8),

    StaticMethod("derefPtr", &Memmap::DerefPtr),
    StaticMethod("readPtrU8", &Memmap::ReadPtrU8),
    StaticMethod("readPtrU16LE", &Memmap::ReadPtrU16LE),
    StaticMethod("readPtrU32LE", &Memmap::ReadPtrU32LE),
    StaticMethod("readPtrU64LE", &Memmap::ReadPtrU64LE),
    StaticMethod("readPtrF32LE", &Memmap::ReadPtrF32LE),
    StaticMethod("readPtrF64LE", &Memmap::ReadPtrF64LE),
    StaticMethod("readPtrU16BE", &Memmap::ReadPtrU16BE),
    StaticMethod("readPtrU32BE", &Memmap::ReadPtrU32BE),
    StaticMethod("readPtrU64BE", &Memmap::ReadPtrU64BE),
    StaticMethod("readPtrF32BE", &Memmap::ReadPtrF32BE),
    StaticMethod("readPtrF64BE", &Memmap::ReadPtrF64BE),
    StaticMethod("readPtrS8", &Memmap::ReadPtrS8),
    StaticMethod("readPtrS16LE", &Memmap::ReadPtrS16LE),
    StaticMethod("readPtrS32LE", &Memmap::ReadPtrS32LE),
    StaticMethod("readPtrS64LE", &Memmap::ReadPtrS64LE),
    StaticMethod("readPtrS16BE", &Memmap::ReadPtrS16BE),
    StaticMethod("readPtrS32BE", &Memmap::ReadPtrS32BE),
    StaticMethod("readPtrS64BE", &Memmap::ReadPtrS64BE),
    StaticMethod("readPtrBufferU8", &Memmap::ReadPtrBufferU8),
    StaticMethod("readPtrBitU8", &Memmap::ReadPtrBitU8),
    StaticMethod("readPtrBitsU8", &Memmap::ReadPtrBitsU8),
    StaticMethod("readPtrBitsBufferU8", &Memmap::ReadPtrBitsBufferU8),
    StaticMethod("writePtrU8", &Memmap::WritePtrU8),
    StaticMethod("writePtrU16LE", &Memmap::WritePtrU16LE),
    StaticMethod("writePtrU32LE", &Memmap::WritePtrU32LE),
    StaticMethod("writePtrU64LE", &Memmap::WritePtrU64LE),
    StaticMethod("writePtrF32LE", &Memmap::WritePtrF32LE),
    StaticMethod("writePtrF64LE", &Memmap::WritePtrF64LE),
    StaticMethod("writePtrU16BE", &Memmap::WritePtrU16BE),
    StaticMethod("writePtrU32BE", &Memmap::WritePtrU32BE),
    StaticMethod("writePtrU64BE", &Memmap::WritePtrU64BE),
    StaticMethod("writePtrF32BE", &Memmap::WritePtrF32BE),
    StaticMethod("writePtrF64BE", &Memmap::WritePtrF64BE),
    StaticMethod("writePtrBufferU8", &Memmap::WritePtrBufferU8),
    StaticMethod("writePtrBitU8", &Memmap::WritePtrBitU8),
    StaticMethod("writePtrBitsU8", &Memmap::WritePtrBitsU8),
    StaticMethod("writePtrBitsBufferU8", &Memmap::WritePtrBitsBufferU8)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Memmap", func);

  return exports;
}

static Napi::Value Clear(const Napi::CallbackInfo& info) {
  ::Memory::Clear();

  return info.Env().Undefined();
}

static Napi::Value Memset(const Napi::CallbackInfo& info) {
  ::Memory::Memset(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU8(info[1]),  // value
    TypeConv::AsU32(info[2])  // size
  );

  return info.Env().Undefined();
}

static Napi::Value ReadU8(const Napi::CallbackInfo& info) {
  return TypeConv::FromU8(info.Env(), ::Memory::Read_U8(
    TypeConv::AsU32(info[0]) // address
  ));
}

static Napi::Value ReadU16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), Common::swap16(::Memory::Read_U16(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadU32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), Common::swap32(::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadU64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), Common::swap64(::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadF32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadF64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadU16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), ::Memory::Read_U16(
    TypeConv::AsU32(info[0]) // address
  ));
}

static Napi::Value ReadU32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), ::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  ));
}

static Napi::Value ReadU64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), ::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  ));
}

static Napi::Value ReadF32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(Common::swap32(::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  ))));
}

static Napi::Value ReadF64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(Common::swap64(::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  ))));
}

static Napi::Value ReadS8(const Napi::CallbackInfo& info) {
  return TypeConv::FromS8(info.Env(), static_cast<s8>(::Memory::Read_U8(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadS16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(Common::swap16(::Memory::Read_U16(
    TypeConv::AsU32(info[0]) // address
  ))));
}

static Napi::Value ReadS32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(Common::swap32(::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  ))));
}

static Napi::Value ReadS64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(Common::swap64(::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  ))));
}

static Napi::Value ReadS16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(::Memory::Read_U16(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadS32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadS64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(::Memory::Read_U64(
    TypeConv::AsU32(info[0]) // address
  )));
}

static Napi::Value ReadBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBufferU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // size
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadBitU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // bit_offset
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadBitsU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadBitsBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsBufferU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // size
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value WriteU8(const Napi::CallbackInfo& info) {
  ::Memory::Write_U8(
    TypeConv::AsU8(info[1]), // value
    TypeConv::AsU32(info[0]) // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU16LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U16(
    Common::swap16(TypeConv::AsU16(info[1])), // value
    TypeConv::AsU32(info[0])                  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU32LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::swap32(TypeConv::AsU32(info[1])), // value
    TypeConv::AsU32(info[0])                  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU64LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::swap64(TypeConv::AsU64(info[1])), // value
    TypeConv::AsU32(info[0])                  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteF32LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::BitCast<u32>(TypeConv::AsF32(info[1])), // value
    TypeConv::AsU32(info[0])                        // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteF64LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::BitCast<u64>(TypeConv::AsF64(info[1])), // value
    TypeConv::AsU32(info[0])                        // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU16BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U16(
    TypeConv::AsU16(info[1]), // value
    TypeConv::AsU32(info[0])  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU32BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    TypeConv::AsU32(info[1]), // value
    TypeConv::AsU32(info[0])  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteU64BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    TypeConv::AsU64(info[1]), // value
    TypeConv::AsU32(info[0])  // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteF32BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::swap32(Common::BitCast<u32>(TypeConv::AsF32(info[1]))), // value
    TypeConv::AsU32(info[0])                                        // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteF64BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::swap64(Common::BitCast<u64>(TypeConv::AsF64(info[1]))), // value
    TypeConv::AsU32(info[0])                                        // address
  );

  return info.Env().Undefined();
}

static Napi::Value WriteBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBufferU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WriteBitU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitU8(
    TypeConv::AsU32(info[0]),  // address
    TypeConv::AsU32(info[1]),  // bit_offset
    TypeConv::AsBool(info[2]), // set
    [](u32 address) { return ::Memory::Read_U8(address); },
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WriteBitsU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WriteBitsBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsBufferU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static u32 DerefPtrOffset(const Napi::CallbackInfo& info) {
  return ::Memory::Read_U32(
    TypeConv::AsU32(info[0])    // address
  ) + TypeConv::AsU32(info[1]); // offset
}

static Napi::Value DerefPtr(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), ::Memory::Read_U32(
    TypeConv::AsU32(info[0]) // address
  ));
}

static Napi::Value ReadPtrU8(const Napi::CallbackInfo& info) {
  return TypeConv::FromU8(info.Env(), ::Memory::Read_U8(
    DerefPtrOffset(info) // address
  ));
}

static Napi::Value ReadPtrU16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), Common::swap16(::Memory::Read_U16(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrU32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), Common::swap32(::Memory::Read_U32(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrU64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), Common::swap64(::Memory::Read_U64(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrF32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(::Memory::Read_U32(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrF64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(::Memory::Read_U64(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrU16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), ::Memory::Read_U16(
    DerefPtrOffset(info) // address
  ));
}

static Napi::Value ReadPtrU32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), ::Memory::Read_U32(
    DerefPtrOffset(info) // address
  ));
}

static Napi::Value ReadPtrU64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), ::Memory::Read_U64(
    DerefPtrOffset(info) // address
  ));
}

static Napi::Value ReadPtrF32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(Common::swap32(::Memory::Read_U32(
    DerefPtrOffset(info) // address
  ))));
}

static Napi::Value ReadPtrF64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(Common::swap64(::Memory::Read_U64(
    DerefPtrOffset(info) // address
  ))));
}

static Napi::Value ReadPtrS8(const Napi::CallbackInfo& info) {
  return TypeConv::FromS8(info.Env(), static_cast<s8>(::Memory::Read_U8(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrS16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(Common::swap16(::Memory::Read_U16(
    DerefPtrOffset(info) // address
  ))));
}

static Napi::Value ReadPtrS32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(Common::swap32(::Memory::Read_U32(
    DerefPtrOffset(info) // address
  ))));
}

static Napi::Value ReadPtrS64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(Common::swap64(::Memory::Read_U64(
    DerefPtrOffset(info) // address
  ))));
}

static Napi::Value ReadPtrS16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(::Memory::Read_U16(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrS32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(::Memory::Read_U32(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrS64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(::Memory::Read_U64(
    DerefPtrOffset(info) // address
  )));
}

static Napi::Value ReadPtrBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBufferU8(info.Env(),
    DerefPtrOffset(info),     // address
    TypeConv::AsU32(info[2]), // size
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadPtrBitU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitU8(info.Env(),
    DerefPtrOffset(info),     // address
    TypeConv::AsU32(info[2]), // bit_offset
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadPtrBitsU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsU8(info.Env(),
    DerefPtrOffset(info), // address
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value ReadPtrBitsBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsBufferU8(info.Env(),
    DerefPtrOffset(info),     // address
    TypeConv::AsU32(info[2]), // size
    [](u32 address) { return ::Memory::Read_U8(address); }
  );
}

static Napi::Value WritePtrU8(const Napi::CallbackInfo& info) {
  ::Memory::Write_U8(
    TypeConv::AsU8(info[2]), // value
    DerefPtrOffset(info)     // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU16LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U16(
    Common::swap16(TypeConv::AsU16(info[2])), // value
    DerefPtrOffset(info)                      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU32LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::swap32(TypeConv::AsU32(info[2])), // value
    DerefPtrOffset(info)                      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU64LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::swap64(TypeConv::AsU64(info[2])), // value
    DerefPtrOffset(info)                      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrF32LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::BitCast<u32>(TypeConv::AsF32(info[2])), // value
    DerefPtrOffset(info)                            // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrF64LE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::BitCast<u64>(TypeConv::AsF64(info[2])), // value
    DerefPtrOffset(info)                            // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU16BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U16(
    TypeConv::AsU16(info[2]), // value
    DerefPtrOffset(info)      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU32BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    TypeConv::AsU32(info[2]), // value
    DerefPtrOffset(info)      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrU64BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    TypeConv::AsU64(info[2]), // value
    DerefPtrOffset(info)      // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrF32BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U32(
    Common::swap32(Common::BitCast<u32>(TypeConv::AsF32(info[2]))), // value
    DerefPtrOffset(info)                                            // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrF64BE(const Napi::CallbackInfo& info) {
  ::Memory::Write_U64(
    Common::swap64(Common::BitCast<u64>(TypeConv::AsF64(info[2]))), // value
    DerefPtrOffset(info)                                            // address
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBufferU8(
    DerefPtrOffset(info),           // address
    info[2].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrBitU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitU8(
    DerefPtrOffset(info),      // address
    TypeConv::AsU32(info[2]),  // bit_offset
    TypeConv::AsBool(info[3]), // set
    [](u32 address) { return ::Memory::Read_U8(address); },
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrBitsU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsU8(
    DerefPtrOffset(info),           // address
    info[2].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

static Napi::Value WritePtrBitsBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsBufferU8(
    DerefPtrOffset(info),           // address
    info[2].As<Napi::Uint8Array>(), // data
    [](u32 address, u8 value) { ::Memory::Write_U8(value, address); }
  );

  return info.Env().Undefined();
}

};

Napi::FunctionReference Memmap::constructor;

namespace AddressSpace {

struct Accessors : Napi::ObjectWrap<Accessors> {

::AddressSpace::Accessors* m_this;

Accessors(const Napi::CallbackInfo& info) :
  Napi::ObjectWrap<Accessors>{info}
{
  m_this = reinterpret_cast<::AddressSpace::Accessors*>(TypeConv::AsPtr(info[0]));
}

static Napi::FunctionReference constructor;

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope{env};

  Napi::Function func =
  DefineClass(env, "Accessors", {
    InstanceMethod("isAddressValid", &Accessors::IsAddressValid),

    InstanceMethod("readU8", &Accessors::ReadU8),
    InstanceMethod("readU16LE", &Accessors::ReadU16LE),
    InstanceMethod("readU32LE", &Accessors::ReadU32LE),
    InstanceMethod("readU64LE", &Accessors::ReadU64LE),
    InstanceMethod("readF32LE", &Accessors::ReadF32LE),
    InstanceMethod("readF64LE", &Accessors::ReadF64LE),
    InstanceMethod("readU16BE", &Accessors::ReadU16BE),
    InstanceMethod("readU32BE", &Accessors::ReadU32BE),
    InstanceMethod("readU64BE", &Accessors::ReadU64BE),
    InstanceMethod("readF32BE", &Accessors::ReadF32BE),
    InstanceMethod("readF64BE", &Accessors::ReadF64BE),
    InstanceMethod("readS8", &Accessors::ReadS8),
    InstanceMethod("readS16LE", &Accessors::ReadS16LE),
    InstanceMethod("readS32LE", &Accessors::ReadS32LE),
    InstanceMethod("readS64LE", &Accessors::ReadS64LE),
    InstanceMethod("readS16BE", &Accessors::ReadS16BE),
    InstanceMethod("readS32BE", &Accessors::ReadS32BE),
    InstanceMethod("readS64BE", &Accessors::ReadS64BE),
    InstanceMethod("readBufferU8", &Accessors::ReadBufferU8),
    InstanceMethod("readBitU8", &Accessors::ReadBitU8),
    InstanceMethod("readBitsU8", &Accessors::ReadBitsU8),
    InstanceMethod("readBitsBufferU8", &Accessors::ReadBitsBufferU8),
    InstanceMethod("writeU8", &Accessors::WriteU8),
    InstanceMethod("writeU16LE", &Accessors::WriteU16LE),
    InstanceMethod("writeU32LE", &Accessors::WriteU32LE),
    InstanceMethod("writeU64LE", &Accessors::WriteU64LE),
    InstanceMethod("writeF32LE", &Accessors::WriteF32LE),
    InstanceMethod("writeF64LE", &Accessors::WriteF64LE),
    InstanceMethod("writeU16BE", &Accessors::WriteU16BE),
    InstanceMethod("writeU32BE", &Accessors::WriteU32BE),
    InstanceMethod("writeU64BE", &Accessors::WriteU64BE),
    InstanceMethod("writeF32BE", &Accessors::WriteF32BE),
    InstanceMethod("writeF64BE", &Accessors::WriteF64BE),
    InstanceMethod("writeBufferU8", &Accessors::WriteBufferU8),
    InstanceMethod("writeBitU8", &Accessors::WriteBitU8),
    InstanceMethod("writeBitsU8", &Accessors::WriteBitsU8),
    InstanceMethod("writeBitsBufferU8", &Accessors::WriteBitsBufferU8),

    InstanceMethod("get", &Accessors::Get),
    InstanceMethod("search", &Accessors::Search),
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Accessors", func);

  return exports;
}

Napi::Value IsAddressValid(const Napi::CallbackInfo& info) {
  return TypeConv::FromBool(info.Env(), m_this->IsValidAddress(
    TypeConv::AsU32(info[0]) // address
  ));
}

Napi::Value ReadU8(const Napi::CallbackInfo& info) {
  return TypeConv::FromU8(info.Env(), m_this->ReadU8(
    TypeConv::AsU32(info[0]) // address
  ));
}

Napi::Value ReadU16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), m_this->ReadU16(
    TypeConv::AsU32(info[0]) // address
  ));
}

Napi::Value ReadU32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  ));
}

Napi::Value ReadU64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  ));
}

Napi::Value ReadF32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadF64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadU16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU16(info.Env(), Common::swap16(m_this->ReadU16(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadU32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU32(info.Env(), Common::swap32(m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadU64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromU64(info.Env(), Common::swap64(m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadF32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF32(info.Env(), Common::BitCast<float>(Common::swap32(m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  ))));
}

Napi::Value ReadF64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromF64(info.Env(), Common::BitCast<double>(Common::swap64(m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  ))));
}

Napi::Value ReadS8(const Napi::CallbackInfo& info) {
  return TypeConv::FromS8(info.Env(), static_cast<s8>(m_this->ReadU8(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadS16LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(m_this->ReadU16(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadS32LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadS64LE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  )));
}

Napi::Value ReadS16BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS16(info.Env(), static_cast<s16>(Common::swap16(m_this->ReadU16(
    TypeConv::AsU32(info[0]) // address
  ))));
}

Napi::Value ReadS32BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS32(info.Env(), static_cast<s32>(Common::swap32(m_this->ReadU32(
    TypeConv::AsU32(info[0]) // address
  ))));
}

Napi::Value ReadS64BE(const Napi::CallbackInfo& info) {
  return TypeConv::FromS64(info.Env(), static_cast<s64>(Common::swap64(m_this->ReadU64(
    TypeConv::AsU32(info[0]) // address
  ))));
}

Napi::Value ReadBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBufferU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // size
    [this](u32 address) { return m_this->ReadU8(address); }
  );
}

Napi::Value ReadBitU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // bit_offset
    [this](u32 address) { return m_this->ReadU8(address); }
  );
}

Napi::Value ReadBitsU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    [this](u32 address) { return m_this->ReadU8(address); }
  );
}

Napi::Value ReadBitsBufferU8(const Napi::CallbackInfo& info) {
  return Helpers::ReadBitsBufferU8(info.Env(),
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1]), // size
    [this](u32 address) { return m_this->ReadU8(address); }
  );
}

Napi::Value WriteU8(const Napi::CallbackInfo& info) {
  m_this->WriteU8(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU8(info[1])   // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU16LE(const Napi::CallbackInfo& info) {
  m_this->WriteU16(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU16(info[1])  // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU32LE(const Napi::CallbackInfo& info) {
  m_this->WriteU32(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU32(info[1])  // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU64LE(const Napi::CallbackInfo& info) {
  m_this->WriteU64(
    TypeConv::AsU32(info[0]), // address
    TypeConv::AsU64(info[1])  // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteF32LE(const Napi::CallbackInfo& info) {
  m_this->WriteU32(
    TypeConv::AsU32(info[0]),                                      // address
    Common::swap32(Common::BitCast<u32>(TypeConv::AsF32(info[1]))) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteF64LE(const Napi::CallbackInfo& info) {
  m_this->WriteU64(
    TypeConv::AsU32(info[0]),                                      // address
    Common::swap64(Common::BitCast<u64>(TypeConv::AsF64(info[1]))) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU16BE(const Napi::CallbackInfo& info) {
  m_this->WriteU16(
    TypeConv::AsU32(info[0]),                // address
    Common::swap16(TypeConv::AsU16(info[1])) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU32BE(const Napi::CallbackInfo& info) {
  m_this->WriteU32(
    TypeConv::AsU32(info[0]),                // address
    Common::swap32(TypeConv::AsU32(info[1])) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteU64BE(const Napi::CallbackInfo& info) {
  m_this->WriteU64(
    TypeConv::AsU32(info[0]),                // address
    Common::swap64(TypeConv::AsU64(info[1])) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteF32BE(const Napi::CallbackInfo& info) {
  m_this->WriteU32(
    TypeConv::AsU32(info[0]),                      // address
    Common::BitCast<u32>(TypeConv::AsF32(info[1])) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteF64BE(const Napi::CallbackInfo& info) {
  m_this->WriteU64(
    TypeConv::AsU32(info[0]),                      // address
    Common::BitCast<u64>(TypeConv::AsF64(info[1])) // value
  );

  return info.Env().Undefined();
}

Napi::Value WriteBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBufferU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [this](u32 address, u8 value) { m_this->WriteU8(address, value); }
  );

  return info.Env().Undefined();
}

Napi::Value WriteBitU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitU8(
    TypeConv::AsU32(info[0]),  // address
    TypeConv::AsU32(info[1]),  // bit_offset
    TypeConv::AsBool(info[2]), // set
    [this](u32 address) { return m_this->ReadU8(address); },
    [this](u32 address, u8 value) { m_this->WriteU8(address, value); }
  );

  return info.Env().Undefined();
}

Napi::Value WriteBitsU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [this](u32 address, u8 value) { m_this->WriteU8(address, value); }
  );

  return info.Env().Undefined();
}

Napi::Value WriteBitsBufferU8(const Napi::CallbackInfo& info) {
  Helpers::WriteBitsBufferU8(
    TypeConv::AsU32(info[0]),       // address
    info[1].As<Napi::Uint8Array>(), // data
    [this](u32 address, u8 value) { m_this->WriteU8(address, value); }
  );

  return info.Env().Undefined();
}

Napi::Value Get(const Napi::CallbackInfo& info) {
  return Napi::ArrayBuffer::New(info.Env(), const_cast<u8*>(m_this->begin()), m_this->end() - m_this->begin());
}

Napi::Value Search(const Napi::CallbackInfo& info) {
  auto needle{info[1].As<Napi::Uint8Array>()};
  auto v{m_this->Search(
    TypeConv::AsU32(info[0]), // haystack_offset
    needle.Data(),            // needle_start
    needle.ByteLength(),      // needle_size
    TypeConv::AsBool(info[2]) // forward
  )};

  return v.has_value() ? TypeConv::FromU32(info.Env(), v.value()) : info.Env().Undefined();
}

};

Napi::FunctionReference Accessors::constructor;

Napi::Value GetAccessors(const Napi::CallbackInfo& info) {
  return Accessors::constructor.New({
    TypeConv::FromPtr(info.Env(), ::AddressSpace::GetAccessors(
      static_cast<::AddressSpace::Type>(TypeConv::AsS32(info[0])) // address_space
    ))
  });
}

}

Napi::Object BuildExports(Napi::Env env, Napi::Object exports) {
  JitInterface::Init(env, exports);
  Memmap::Init(env, exports);

  auto address_space{Napi::Object::New(env)};
  AddressSpace::Accessors::Init(env, address_space);
  address_space.Set("getAccessors", Napi::Function::New(env, &AddressSpace::GetAccessors));
  exports.Set("AddressSpace", address_space);

  return exports;
}

}
