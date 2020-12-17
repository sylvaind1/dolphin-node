// Copyright 2015 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <QApplication>
#include <QPushButton>

#include <napi.h>

#include "Common/MsgHandler.h"
#include "Common/ScopeGuard.h"

#include "Core/Core.h"

#include "DolphinNode/Js/Frontend.h"
#include "DolphinNode/QtUtils/ModalMessageBox.h"
#include "DolphinNode/QtUtils/RunOnObject.h"

bool QtMsgAlertHandler(const char* caption, const char* text, bool yes_no,
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

Napi::Object ModuleEntryPoint(Napi::Env env, Napi::Object exports) {
  Js::Frontend::Init(env, exports);

  return exports;
}

NODE_API_MODULE(NAPI_DOLPHIN_NODE, ModuleEntryPoint)
