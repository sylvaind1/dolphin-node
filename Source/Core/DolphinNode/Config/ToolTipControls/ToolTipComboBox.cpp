// Copyright 2020 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinNode/Config/ToolTipControls/ToolTipComboBox.h"

QPoint ToolTipComboBox::GetToolTipPosition() const
{
  return pos() + QPoint(width() / 2, height() / 2);
}
