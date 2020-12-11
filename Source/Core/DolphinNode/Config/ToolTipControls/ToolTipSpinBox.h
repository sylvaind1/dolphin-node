// Copyright 2020 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "DolphinNode/Config/ToolTipControls/ToolTipWidget.h"

#include <QSpinBox>

class ToolTipSpinBox : public ToolTipWidget<QSpinBox>
{
private:
  QPoint GetToolTipPosition() const override;
};
