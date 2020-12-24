#pragma once

#include <QApplication>
#include <QObject>

#include <array>
#include <atomic>
#include <optional>
#include <string>

#include <napi.h>

class MainWindow;
class Settings;

namespace Js {

class Frontend : public QObject, public Napi::ObjectWrap<Frontend> {
  Q_OBJECT

public:
  struct CreateInfo {
    std::string org_name;
    std::string org_domain;
    std::string app_name;
    std::string app_display_name;
    std::string base_dir;
    std::string user_dir;
  };

  struct BootInfo {
    std::string path;
    bool is_nand_title;
    std::optional<std::string> savestate_path;
  };

  enum CallbackIndex {
    CallbackIndex_StateChanged,
    CallbackIndex_StopRequested,
    CallbackIndex_StopComplete,
    CallbackIndex_ExitRequested,
    CallbackIndex_Count
  };

  enum AtomicState {
    AtomicState_MtCbEnabled,
    AtomicState_FreeOnTick,
    AtomicState_PendingOnTick,
    AtomicState_HandlingOnTick,
    AtomicState_PendingOnImGui,
    AtomicState_HandlingOnImGui,
    AtomicState_Count
  };

  static Napi::FunctionReference constructor;
  static Napi::Object Init(Napi::Env env, Napi::Object exports);

  Frontend(const Napi::CallbackInfo& info);

  Napi::Value Initialize(const Napi::CallbackInfo& info);
  Napi::Value Startup(const Napi::CallbackInfo& info);
  Napi::Value Shutdown(const Napi::CallbackInfo& info);
  Napi::Value ProcessEvents(const Napi::CallbackInfo& info);
  Napi::Value On(const Napi::CallbackInfo& info);
  Napi::Value DisplayMessage(const Napi::CallbackInfo& info);
  Napi::Value RequestStop(const Napi::CallbackInfo& info);

  Napi::Value EnableMtCallbacks(const Napi::CallbackInfo& info);
  Napi::Value DisableMtCallbacks(const Napi::CallbackInfo& info);
  Napi::Value IsOnTickPending(const Napi::CallbackInfo& info);
  Napi::Value SignalHandlingOnTick(const Napi::CallbackInfo& info);
  Napi::Value UnlockOnTick(const Napi::CallbackInfo& info);
  Napi::Value IsOnImGuiPending(const Napi::CallbackInfo& info);
  Napi::Value SignalHandlingOnImGui(const Napi::CallbackInfo& info);
  Napi::Value UnlockOnImGui(const Napi::CallbackInfo& info);

  Napi::Value Button(const Napi::CallbackInfo& info);

private:
  QScopedPointer<QApplication> m_app;
  MainWindow* m_mw;
  Settings* m_settings;
  std::array<Napi::FunctionReference, CallbackIndex_Count> m_callbacks;
  std::array<std::atomic_bool, AtomicState_Count> m_states;
};

}
