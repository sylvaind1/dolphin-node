#pragma once

#include <napi.h>

namespace Js::Memory {

Napi::Object BuildExports(Napi::Env env, Napi::Object exports);

}
