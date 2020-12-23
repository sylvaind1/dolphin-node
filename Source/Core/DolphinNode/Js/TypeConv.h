#include <cstdint>
#include <filesystem>
#include <string>

#include <napi.h>

namespace Js::TypeConv {

inline Napi::Object AsObject(const Napi::Value& value) {
  return value.As<Napi::Object>();
}

inline Napi::Array AsArray(const Napi::Value& value) {
  return value.As<Napi::Array>();
}

inline std::uint8_t AsU8(const Napi::Value& value) {
  return static_cast<std::uint8_t>(value.As<Napi::Number>().Uint32Value());
}

inline std::uint8_t AsU8Or(const Napi::Value& value, std::uint8_t default_value) {
  return value.IsUndefined() ? default_value : AsU8(value);
}

inline Napi::Value FromU8(Napi::Env env, std::uint8_t value) {
  return Napi::Number::New(env, value);
}

inline std::uint16_t AsU16(const Napi::Value& value) {
  return static_cast<std::uint16_t>(value.As<Napi::Number>().Uint32Value());
}

inline std::uint16_t AsU16Or(const Napi::Value& value, std::uint16_t default_value) {
  return value.IsUndefined() ? default_value : AsU16(value);
}

inline Napi::Value FromU16(Napi::Env env, std::uint16_t value) {
  return Napi::Number::New(env, value);
}

inline std::uint32_t AsU32(const Napi::Value& value) {
  return static_cast<std::uint32_t>(value.As<Napi::Number>().Uint32Value());
}

inline std::uint32_t AsU32Or(const Napi::Value& value, std::uint32_t default_value) {
  return value.IsUndefined() ? default_value : AsU32(value);
}

inline Napi::Value FromU32(Napi::Env env, std::uint32_t value) {
  return Napi::Number::New(env, value);
}

inline std::uint64_t AsU64(const Napi::Value& value) {
  bool lossless;
  return static_cast<std::uint64_t>(value.As<Napi::BigInt>().Uint64Value(&lossless));
}

inline std::uint64_t AsU64Or(const Napi::Value& value, std::uint64_t default_value) {
  return value.IsUndefined() ? default_value : AsU64(value);
}

inline Napi::Value FromU64(Napi::Env env, std::uint64_t value) {
  return Napi::BigInt::New(env, value);
}

inline std::int8_t AsS8(const Napi::Value& value) {
  return static_cast<std::int8_t>(value.As<Napi::Number>().Int32Value());
}

inline std::int8_t AsS8Or(const Napi::Value& value, std::int8_t default_value) {
  return value.IsUndefined() ? default_value : AsS8(value);
}

inline Napi::Value FromS8(Napi::Env env, std::int8_t value) {
  return Napi::Number::New(env, value);
}

inline std::int16_t AsS16(const Napi::Value& value) {
  return static_cast<std::int16_t>(value.As<Napi::Number>().Int32Value());
}

inline std::int16_t AsS16Or(const Napi::Value& value, std::int16_t default_value) {
  return value.IsUndefined() ? default_value : AsS16(value);
}

inline Napi::Value FromS16(Napi::Env env, std::int16_t value) {
  return Napi::Number::New(env, value);
}

inline std::int32_t AsS32(const Napi::Value& value) {
  return static_cast<std::int32_t>(value.As<Napi::Number>().Int32Value());
}

inline std::int32_t AsS32Or(const Napi::Value& value, std::int32_t default_value) {
  return value.IsUndefined() ? default_value : AsS32(value);
}

inline Napi::Value FromS32(Napi::Env env, std::int32_t value) {
  return Napi::Number::New(env, value);
}

inline std::int64_t AsS64(const Napi::Value& value) {
  bool lossless;
  return static_cast<std::int64_t>(value.As<Napi::BigInt>().Int64Value(&lossless));
}

inline std::int64_t AsS64Or(const Napi::Value& value, std::int64_t default_value) {
  return value.IsUndefined() ? default_value : AsS64(value);
}

inline Napi::Value FromS64(Napi::Env env, std::int64_t value) {
  return Napi::BigInt::New(env, value);
}

inline float AsF32(const Napi::Value& value) {
  return value.As<Napi::Number>().FloatValue();
}

inline float AsF32Or(const Napi::Value& value, float default_value) {
  return value.IsUndefined() ? default_value : AsF32(value);
}

inline Napi::Value FromF32(Napi::Env env, float value) {
  return Napi::Number::New(env, value);
}

inline double AsF64(const Napi::Value& value) {
  return value.As<Napi::Number>().DoubleValue();
}

inline double AsF64Or(const Napi::Value& value, double default_value) {
  return value.IsUndefined() ? default_value : AsF64(value);
}

inline Napi::Value FromF64(Napi::Env env, double value) {
  return Napi::Number::New(env, value);
}

inline bool AsBool(const Napi::Value& value) {
  return value.As<Napi::Boolean>().Value();
}

inline bool AsBoolOr(const Napi::Value& value, bool default_value) {
  return value.IsUndefined() ? default_value : AsBool(value);
}

inline bool ToBool(const Napi::Value& value) {
  return value.ToBoolean().Value();
}

inline Napi::Value FromBool(Napi::Env env, bool value) {
  return Napi::Boolean::New(env, value);
}

inline void* AsPtr(const Napi::Value& value) {
  return reinterpret_cast<void*>(AsU64(value));
}

inline void* AsPtrOr(const Napi::Value& value, void* default_value) {
  return value.IsUndefined() ? default_value : AsPtr(value);
}

inline Napi::Value FromPtr(Napi::Env env, void* value) {
  return FromU64(env, reinterpret_cast<std::uint64_t>(value));
}

inline std::string AsStrUtf8(const Napi::Value& value) {
  return value.As<Napi::String>().Utf8Value();
}

inline std::string AsStrUtf8Or(const Napi::Value& value, const std::string& default_value) {
  return value.IsUndefined() ? default_value : AsStrUtf8(value);
}

inline Napi::Value FromStrUtf8(Napi::Env env, const std::string& value) {
  return Napi::String::New(env, value);
}

inline std::u16string AsStrUtf16(const Napi::Value& value) {
  return value.As<Napi::String>().Utf16Value();
}

inline std::u16string AsStrUtf16Or(const Napi::Value& value, const std::u16string& default_value) {
  return value.IsUndefined() ? default_value : AsStrUtf16(value);
}

inline Napi::Value FromStrUtf16(Napi::Env env, const std::u16string& value) {
  return Napi::String::New(env, value);
}

inline std::filesystem::path AsPath(const Napi::Value& value) {
#ifdef _WIN32
  return AsStrUtf16(value);
#else
  return AsStrUtf8(value);
#endif
}

inline std::filesystem::path AsPathOr(const Napi::Value& value, const std::filesystem::path& default_value) {
  return value.IsUndefined() ? default_value : AsPath(value);
}

inline Napi::Value FromPath(Napi::Env env, const std::filesystem::path& value) {
#ifdef _WIN32
  return FromStrUtf16(env, value.u16string());
#else
  return FromStrUtf8(env, value.string());
#endif
}

}
