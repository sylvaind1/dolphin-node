#include <QApplication>
#include <QDir>

#include <unordered_map>

#include <imgui.h>

#include "Common/FileUtil.h"

#include "Core/Boot/Boot.h"
#include "Core/Config/MainSettings.h"
#include "Core/Core.h"

#include "DolphinNode/Host.h"
#include "DolphinNode/Js/Frontend.h"
#include "DolphinNode/MainWindow.h"
#include "DolphinNode/RenderWidget.h"
#include "DolphinNode/Resources.h"
#include "DolphinNode/Settings.h"
#include "DolphinNode/Translation.h"

#include "UICommon/UICommon.h"

#include "VideoCommon/RenderBase.h"

bool QtMsgAlertHandler(const char* caption, const char* text, bool yes_no, Common::MsgType style);

namespace Js {

Napi::FunctionReference Frontend::constructor;

Napi::Object Frontend::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope{env};

  Napi::Function func =
  DefineClass(env, "Frontend", {
    InstanceMethod("initialize", &Frontend::Initialize),
    InstanceMethod("startup", &Frontend::Startup),
    InstanceMethod("shutdown", &Frontend::Shutdown),
    InstanceMethod("processEvents", &Frontend::ProcessEvents),
    InstanceMethod("on", &Frontend::On),
    InstanceMethod("displayMessage", &Frontend::DisplayMessage),
    InstanceMethod("requestStop", &Frontend::RequestStop),

    InstanceMethod("enableMtCallbacks", &Frontend::EnableMtCallbacks),
    InstanceMethod("disableMtCallbacks", &Frontend::DisableMtCallbacks),
    InstanceMethod("isOnTickPending", &Frontend::IsOnTickPending),
    InstanceMethod("signalHandlingOnTick", &Frontend::SignalHandlingOnTick),
    InstanceMethod("unlockOnTick", &Frontend::UnlockOnTick),
    InstanceMethod("isOnImGuiPending", &Frontend::IsOnImGuiPending),
    InstanceMethod("signalHandlingOnImGui", &Frontend::SignalHandlingOnImGui),
    InstanceMethod("unlockOnImGui", &Frontend::UnlockOnImGui),

    InstanceMethod("button", &Frontend::Button)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Frontend", func);

  return exports;
}

Frontend::Frontend(const Napi::CallbackInfo& info) :
  Napi::ObjectWrap<Frontend>{info}
{}

static Frontend::CreateInfo GetCreateInfoFromObject(const Napi::Object& obj) {
  const auto make_path = [&obj](const std::string& property) {
    return QStringLiteral("%1/").arg(QDir::fromNativeSeparators(
      QString::fromStdString(obj.Get(property).As<Napi::String>().Utf8Value()))).toStdString();
  };

  Frontend::CreateInfo info;
  info.org_name = obj.Get("orgName").As<Napi::String>().Utf8Value();
  info.org_domain = obj.Get("orgDomain").As<Napi::String>().Utf8Value();
  info.app_name = obj.Get("appName").As<Napi::String>().Utf8Value();
  info.app_display_name = obj.Get("appDisplayName").As<Napi::String>().Utf8Value();
  info.base_dir = make_path("baseDir");
  info.user_dir = make_path("userDir");

  return info;
}

static Frontend::BootInfo GetBootInfoFromObject(const Napi::Object& obj) {
  Frontend::BootInfo info;
  info.path = obj.Get("path").As<Napi::String>().Utf8Value();
  info.is_nand_title = obj.Get("isNandTitle").As<Napi::Boolean>().Value();

  const auto savestate_path = obj.Get("savestatePath");
  if (!savestate_path.IsUndefined())
    info.savestate_path = savestate_path.As<Napi::String>().Utf8Value();

  return info;
}

static u64 GetNandTitleIdFromString(const std::string& str) {
  if (str.length() != 16) return 0;
  return std::stoull(str, nullptr, 16);
}

Napi::Value Frontend::Initialize(const Napi::CallbackInfo& info) {
  const auto start_info = GetCreateInfoFromObject(info[0].As<Napi::Object>());

#ifdef _WIN32
  QCoreApplication::addLibraryPath(QStringLiteral("%1%2").arg(QString::fromStdString(start_info.base_dir), QStringLiteral("QtPlugins")));
#endif

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QCoreApplication::setOrganizationName(QString::fromStdString(start_info.org_name));
  QCoreApplication::setOrganizationDomain(QString::fromStdString(start_info.org_domain));
  QCoreApplication::setApplicationName(QString::fromStdString(start_info.app_name));
  QGuiApplication::setApplicationDisplayName(QString::fromStdString(start_info.app_display_name));

  static int argc = 0;
  static char** argv;
  m_app.reset(new QApplication{argc, argv});

#ifdef _WIN32
  QApplication::setFont(QApplication::font("QMenu"));
#endif

  Js::SetBaseDirectory(start_info.base_dir);
  UICommon::SetUserDirectory(start_info.user_dir);
  UICommon::CreateDirectories();
  UICommon::Init();
  Resources::Init();
  Settings::Instance().SetBatchModeEnabled(false);
  Common::RegisterMsgAlertHandler(QtMsgAlertHandler);
  Translation::Initialize();

  return info.Env().Undefined();
}

Napi::Value Frontend::Startup(const Napi::CallbackInfo& info) {
  const auto boot_info = GetBootInfoFromObject(info[0].As<Napi::Object>());
  std::unique_ptr<BootParameters> boot;

  if (boot_info.is_nand_title) {
    const auto title_id = GetNandTitleIdFromString(boot_info.path);

    if (title_id == 0)
      throw Napi::Error(info.Env(), Napi::String::New(info.Env(), "invalid title id"));

    boot = std::make_unique<BootParameters>(BootParameters::NANDTitle{title_id});
  }
  else {
    boot = BootParameters::GenerateFromFile(boot_info.path, boot_info.savestate_path);
  }

  if (boot_info.path.empty() && (boot_info.savestate_path.has_value() && !boot_info.savestate_path.value().empty()))
    throw Napi::Error(info.Env(), Napi::String::New(info.Env(), "a save state cannot be loaded without specifying a game to launch"));

  m_mw = new MainWindow{nullptr, {}};

  QObject::connect(m_mw, &MainWindow::JsStopRequested, this, [this]() {
    return m_callbacks[CallbackIndex_StopRequested].Call({}).As<Napi::Boolean>().Value();
  });
  QObject::connect(m_mw, &MainWindow::JsStopComplete, this, [this]() {
    m_callbacks[CallbackIndex_StopComplete].Call({});
  });

  m_settings = &Settings::Instance();
  m_settings->SetDebugModeEnabled(false);

  QObject::connect(m_settings, &Settings::EmulationStateChanged, this, [this](Core::State new_state) {
    m_callbacks[CallbackIndex_StateChanged].Call({ Napi::Number::New(m_callbacks[CallbackIndex_StateChanged].Env(), static_cast<double>(new_state)) });
  });

  JsCallbacks::SetOnStateChangeBegin([this]() {
    m_states[AtomicState_FreeOnTick] = true;

    if (m_states[AtomicState_PendingOnTick] && !m_states[AtomicState_HandlingOnTick])
      m_states[AtomicState_PendingOnTick] = false;
    else if (m_states[AtomicState_PendingOnTick] && m_states[AtomicState_HandlingOnTick])
      while (m_states[AtomicState_PendingOnTick]);

    if (m_states[AtomicState_PendingOnImGui] && !m_states[AtomicState_HandlingOnImGui])
      m_states[AtomicState_PendingOnImGui] = false;
    else if (m_states[AtomicState_PendingOnImGui] && m_states[AtomicState_HandlingOnImGui])
      while (m_states[AtomicState_PendingOnImGui]);
    });
  JsCallbacks::SetOnStateChangeEnd([this]() {
    m_states[AtomicState_FreeOnTick] = false;
  });
  JsCallbacks::SetOnTick([this]() {
    if (m_states[AtomicState_MtCbEnabled] && !m_states[AtomicState_FreeOnTick]) {
      m_states[AtomicState_PendingOnTick] = true;
      while (m_states[AtomicState_PendingOnTick]);
    }
  });
  JsCallbacks::SetOnImGui([this]() {
    if (m_states[AtomicState_MtCbEnabled] && !m_states[AtomicState_FreeOnTick]) {
      m_states[AtomicState_PendingOnImGui] = true;
      while (m_states[AtomicState_PendingOnImGui]);
    }
  });
  JsCallbacks::SetPassEventToImGui([this]() {
    return !(m_states[AtomicState_PendingOnImGui] && !m_states[AtomicState_HandlingOnImGui]);
  });

  m_mw->show();
  m_mw->StartGame(std::move(boot));

  return info.Env().Undefined();
}

Napi::Value Frontend::Shutdown(const Napi::CallbackInfo& info) {
  JsCallbacks::SetOnStateChangeBegin({});
  JsCallbacks::SetOnStateChangeEnd({});
  JsCallbacks::SetOnTick({});
  JsCallbacks::SetOnImGui({});
  JsCallbacks::SetPassEventToImGui({});

  m_mw->hide();
  delete m_mw;

  Core::Shutdown();
  UICommon::Shutdown();

  Host::GetInstance()->deleteLater();

  return info.Env().Undefined();
}

Napi::Value Frontend::ProcessEvents(const Napi::CallbackInfo& info) {
  const auto maxtime = info[0].As<Napi::Number>().Int32Value();

  m_app->processEvents(QEventLoop::AllEvents, maxtime);
  Core::HostDispatchJobs();

  if (m_mw->JsIsExitRequested())
    m_callbacks[CallbackIndex_ExitRequested].Call({});

  return info.Env().Undefined();
}

Napi::Value Frontend::On(const Napi::CallbackInfo& info) {
  static const std::unordered_map<std::string, CallbackIndex> cb_map{
    {"state-changed", CallbackIndex_StateChanged},
    {"stop-requested", CallbackIndex_StopRequested},
    {"stop-complete", CallbackIndex_StopComplete},
    {"exit-requested", CallbackIndex_ExitRequested},
  };

  const auto name = info[0].As<Napi::String>().Utf8Value();
  const auto cb_index_it = cb_map.find(name);

  if (cb_index_it != cb_map.end())
    m_callbacks[cb_index_it->second] = Napi::Persistent(info[1].As<Napi::Function>());
  else
    throw Napi::Error(info.Env(), Napi::String::New(info.Env(), "invalid callback name"));

  return info.Env().Undefined();
}

Napi::Value Frontend::DisplayMessage(const Napi::CallbackInfo& info) {
  const auto message = info[0].As<Napi::String>().Utf8Value();
  const auto time_in_ms = info[1].As<Napi::Number>().Int32Value();

  Core::DisplayMessage(message, time_in_ms);

  return info.Env().Undefined();
}

Napi::Value Frontend::RequestStop(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), m_mw->ActualRequestStop());
}

Napi::Value Frontend::EnableMtCallbacks(const Napi::CallbackInfo& info) {
  m_states[AtomicState_MtCbEnabled] = true;

  return info.Env().Undefined();
}

Napi::Value Frontend::DisableMtCallbacks(const Napi::CallbackInfo& info) {
  m_states[AtomicState_MtCbEnabled] = false;

  return info.Env().Undefined();
}

Napi::Value Frontend::IsOnTickPending(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), m_states[AtomicState_PendingOnTick]);
}

Napi::Value Frontend::SignalHandlingOnTick(const Napi::CallbackInfo& info) {
  m_states[AtomicState_HandlingOnTick] = true;

  return info.Env().Undefined();
}

Napi::Value Frontend::UnlockOnTick(const Napi::CallbackInfo& info) {
  m_states[AtomicState_PendingOnTick] = false;
  m_states[AtomicState_HandlingOnTick] = false;

  return info.Env().Undefined();
}

Napi::Value Frontend::IsOnImGuiPending(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), m_states[AtomicState_PendingOnImGui]);
}

Napi::Value Frontend::SignalHandlingOnImGui(const Napi::CallbackInfo& info) {
  m_states[AtomicState_HandlingOnImGui] = true;

  return info.Env().Undefined();
}

Napi::Value Frontend::UnlockOnImGui(const Napi::CallbackInfo& info) {
  m_states[AtomicState_PendingOnImGui] = false;
  m_states[AtomicState_HandlingOnImGui] = false;

  return info.Env().Undefined();
}

Napi::Value Frontend::Button(const Napi::CallbackInfo& info) {
  const auto label = info[0].As<Napi::String>().Utf8Value();

  return Napi::Boolean::New(info.Env(), ImGui::Button(label.c_str()));
}

}
