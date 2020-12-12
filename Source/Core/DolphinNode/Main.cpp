// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#ifdef _WIN32
#include <string>
#include <vector>

#include <Windows.h>
#include <cstdio>
#endif

#include <OptionParser.h>
#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QDir>
#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "Common/Config/Config.h"
#include "Common/MsgHandler.h"
#include "Common/ScopeGuard.h"

#include "Core/Analytics.h"
#include "Core/Boot/Boot.h"
#include "Core/Config/MainSettings.h"
#include "Core/Core.h"

#include "DolphinNode/Host.h"
#include "DolphinNode/MainWindow.h"
#include "DolphinNode/QtUtils/ModalMessageBox.h"
#include "DolphinNode/QtUtils/RunOnObject.h"
#include "DolphinNode/Resources.h"
#include "DolphinNode/Settings.h"
#include "DolphinNode/Translation.h"
#include "DolphinNode/Updater.h"

#include "UICommon/CommandLineParse.h"
#include "UICommon/UICommon.h"

static bool QtMsgAlertHandler(const char* caption, const char* text, bool yes_no,
                              Common::MsgType style)
{
  const bool called_from_cpu_thread = Core::IsCPUThread();

  std::optional<bool> r = RunOnObject(QApplication::instance(), [&] {
    Common::ScopeGuard scope_guard(&Core::UndeclareAsCPUThread);
    if (called_from_cpu_thread)
    {
      // Temporarily declare this as the CPU thread to avoid getting a deadlock if any DolphinQt
      // code calls RunAsCPUThread while the CPU thread is blocked on this function returning.
      // Notably, if the panic alert steals focus from RenderWidget, Host::SetRenderFocus gets
      // called, which can attempt to use RunAsCPUThread to get us out of exclusive fullscreen.
      Core::DeclareAsCPUThread();
    }
    else
    {
      scope_guard.Dismiss();
    }

    ModalMessageBox message_box(QApplication::activeWindow(), Qt::ApplicationModal);
    message_box.setWindowTitle(QString::fromUtf8(caption));
    message_box.setText(QString::fromUtf8(text));

    message_box.setStandardButtons(yes_no ? QMessageBox::Yes | QMessageBox::No : QMessageBox::Ok);
    if (style == Common::MsgType::Warning)
      message_box.addButton(QMessageBox::Ignore)->setText(QObject::tr("Ignore for this session"));

    message_box.setIcon([&] {
      switch (style)
      {
      case Common::MsgType::Information:
        return QMessageBox::Information;
      case Common::MsgType::Question:
        return QMessageBox::Question;
      case Common::MsgType::Warning:
        return QMessageBox::Warning;
      case Common::MsgType::Critical:
        return QMessageBox::Critical;
      }
      // appease MSVC
      return QMessageBox::NoIcon;
    }());

    const int button = message_box.exec();
    if (button == QMessageBox::Yes)
      return true;

    if (button == QMessageBox::Ignore)
    {
      Common::SetEnableAlert(false);
      return true;
    }

    return false;
  });
  if (r.has_value())
    return *r;
  return false;
}

int DolphinEntryPoint(const std::vector<std::string>& vargs)
{
  auto parser = CommandLineParse::CreateParser(CommandLineParse::ParserOptions::IncludeGUIOptions);
  const optparse::Values& options = CommandLineParse::ParseArguments(parser.get(), vargs);
  const std::vector<std::string> args = parser->args();

#ifdef _WIN32
  QCoreApplication::addLibraryPath(QStringLiteral("%1/%2").arg(QDir::currentPath(), QStringLiteral("QtPlugins")));
#endif

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QCoreApplication::setOrganizationName(QStringLiteral("Dolphin Emulator"));
  QCoreApplication::setOrganizationDomain(QStringLiteral("dolphin-emu.org"));
  QCoreApplication::setApplicationName(QStringLiteral("dolphin-emu"));

  static int argc = 1;
  static auto argv0 = QStringLiteral("%1/%2").arg(QDir::currentPath(), QStringLiteral("dolphin.node")).toStdString();
  static char* argv[] = { argv0.data() };

  QApplication app(argc, argv);

#ifdef _WIN32
  // On Windows, Qt 5's default system font (MS Shell Dlg 2) is outdated.
  // Interestingly, the QMenu font is correct and comes from lfMenuFont
  // (Segoe UI on English computers).
  // So use it for the entire application.
  // This code will become unnecessary and obsolete once we switch to Qt 6.
  QApplication::setFont(QApplication::font("QMenu"));
#endif

  UICommon::SetUserDirectory(static_cast<const char*>(options.get("user")));
  UICommon::CreateDirectories();
  UICommon::Init();
  Resources::Init();
  Settings::Instance().SetBatchModeEnabled(options.is_set("batch"));

  // Hook up alerts from core
  Common::RegisterMsgAlertHandler(QtMsgAlertHandler);

  // Hook up translations
  Translation::Initialize();

  // Whenever the event loop is about to go to sleep, dispatch the jobs
  // queued in the Core first.
  QObject::connect(QAbstractEventDispatcher::instance(), &QAbstractEventDispatcher::aboutToBlock,
                   &app, &Core::HostDispatchJobs);

  std::optional<std::string> save_state_path;
  if (options.is_set("save_state"))
  {
    save_state_path = static_cast<const char*>(options.get("save_state"));
  }

  std::unique_ptr<BootParameters> boot;
  bool game_specified = false;
  if (options.is_set("exec"))
  {
    const std::list<std::string> paths_list = options.all("exec");
    const std::vector<std::string> paths{std::make_move_iterator(std::begin(paths_list)),
                                         std::make_move_iterator(std::end(paths_list))};
    boot = BootParameters::GenerateFromFile(paths, save_state_path);
    game_specified = true;
  }
  else if (options.is_set("nand_title"))
  {
    const std::string hex_string = static_cast<const char*>(options.get("nand_title"));
    if (hex_string.length() == 16)
    {
      const u64 title_id = std::stoull(hex_string, nullptr, 16);
      boot = std::make_unique<BootParameters>(BootParameters::NANDTitle{title_id});
    }
    else
    {
      ModalMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Invalid title ID."));
    }
    game_specified = true;
  }
  else if (!args.empty())
  {
    boot = BootParameters::GenerateFromFile(args.front(), save_state_path);
    game_specified = true;
  }

  int retval;

  if (save_state_path && !game_specified)
  {
    ModalMessageBox::critical(
        nullptr, QObject::tr("Error"),
        QObject::tr("A save state cannot be loaded without specifying a game to launch."));
    retval = 1;
  }
  else if (Settings::Instance().IsBatchModeEnabled() && !game_specified)
  {
    ModalMessageBox::critical(
        nullptr, QObject::tr("Error"),
        QObject::tr("Batch mode cannot be used without specifying a game to launch."));
    retval = 1;
  }
  else if (!boot && (Settings::Instance().IsBatchModeEnabled() || save_state_path))
  {
    // A game to launch was specified, but it was invalid.
    // An error has already been shown by code above, so exit without showing another error.
    retval = 1;
  }
  else
  {
    MainWindow win{std::move(boot), static_cast<const char*>(options.get("movie"))};
    if (options.is_set("debugger"))
      Settings::Instance().SetDebugModeEnabled(true);
    win.Show();

    retval = app.exec();
  }

  Core::Shutdown();
  UICommon::Shutdown();
  Host::GetInstance()->deleteLater();

  return retval;
}

#include <napi.h>

Napi::Value NapiDolphinEntryPoint(const Napi::CallbackInfo& info)
{
  auto in_args = info[0].As<Napi::Array>();
  std::vector<std::string> args;

  for (unsigned i{}; i < in_args.Length(); ++i)
  {
    args.push_back(in_args.Get(i).As<Napi::String>().Utf8Value());
  }

  int retval = DolphinEntryPoint(args);

  return Napi::Number::New(info.Env(), retval);
}

Napi::Object ModuleEntryPoint(Napi::Env env, Napi::Object exports)
{
  exports.Set("main", Napi::Function::New(env, NapiDolphinEntryPoint));

  return exports;
}

NODE_API_MODULE(NAPI_DOLPHIN_NODE, ModuleEntryPoint)