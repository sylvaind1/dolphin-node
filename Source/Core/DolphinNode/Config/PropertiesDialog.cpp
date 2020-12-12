// Copyright 2016 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <memory>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "DiscIO/Enums.h"
#include "DiscIO/Volume.h"

#include "DolphinNode/Config/ARCodeWidget.h"
#include "DolphinNode/Config/FilesystemWidget.h"
#include "DolphinNode/Config/GameConfigWidget.h"
#include "DolphinNode/Config/GeckoCodeWidget.h"
#include "DolphinNode/Config/InfoWidget.h"
#include "DolphinNode/Config/PatchesWidget.h"
#include "DolphinNode/Config/PropertiesDialog.h"
#include "DolphinNode/Config/VerifyWidget.h"
#include "DolphinNode/QtUtils/WrapInScrollArea.h"

#include "UICommon/GameFile.h"

PropertiesDialog::PropertiesDialog(QWidget* parent, const UICommon::GameFile& game)
    : QDialog(parent)
{
  setWindowTitle(QStringLiteral("%1: %2 - %3")
                     .arg(QString::fromStdString(game.GetFileName()),
                          QString::fromStdString(game.GetGameID()),
                          QString::fromStdString(game.GetLongName())));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QVBoxLayout* layout = new QVBoxLayout();

  QTabWidget* tab_widget = new QTabWidget(this);
  InfoWidget* info = new InfoWidget(game);

  ARCodeWidget* ar = new ARCodeWidget(game);
  GeckoCodeWidget* gecko = new GeckoCodeWidget(game);
  PatchesWidget* patches = new PatchesWidget(game);
  GameConfigWidget* game_config = new GameConfigWidget(game);

  connect(gecko, &GeckoCodeWidget::OpenGeneralSettings, this,
          &PropertiesDialog::OpenGeneralSettings);

  connect(ar, &ARCodeWidget::OpenGeneralSettings, this, &PropertiesDialog::OpenGeneralSettings);

  const int padding_width = 120;
  const int padding_height = 100;
  tab_widget->addTab(GetWrappedWidget(game_config, this, padding_width, padding_height),
                     tr("Game Config"));
  tab_widget->addTab(GetWrappedWidget(patches, this, padding_width, padding_height), tr("Patches"));
  tab_widget->addTab(GetWrappedWidget(ar, this, padding_width, padding_height), tr("AR Codes"));
  tab_widget->addTab(GetWrappedWidget(gecko, this, padding_width, padding_height),
                     tr("Gecko Codes"));
  tab_widget->addTab(GetWrappedWidget(info, this, padding_width, padding_height), tr("Info"));

  if (game.GetPlatform() != DiscIO::Platform::ELFOrDOL)
  {
    std::shared_ptr<DiscIO::Volume> volume = DiscIO::CreateVolume(game.GetFilePath());
    if (volume)
    {
      VerifyWidget* verify = new VerifyWidget(volume);
      tab_widget->addTab(GetWrappedWidget(verify, this, padding_width, padding_height),
                         tr("Verify"));

      if (DiscIO::IsDisc(game.GetPlatform()))
      {
        FilesystemWidget* filesystem = new FilesystemWidget(volume);
        tab_widget->addTab(GetWrappedWidget(filesystem, this, padding_width, padding_height),
                           tr("Filesystem"));
      }
    }
  }

  layout->addWidget(tab_widget);

  QDialogButtonBox* close_box = new QDialogButtonBox(QDialogButtonBox::Close);

  connect(close_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

  layout->addWidget(close_box);

  setLayout(layout);
}