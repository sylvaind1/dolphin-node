// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinNode/Config/Graphics/GraphicsWidget.h"

#include <QEvent>
#include <QLabel>

#include "DolphinNode/Config/Graphics/GraphicsWindow.h"

GraphicsWidget::GraphicsWidget(GraphicsWindow* parent)
{
  parent->RegisterWidget(this);
}

void GraphicsWidget::AddDescription(QWidget* widget, const char* description)
{
  emit DescriptionAdded(widget, description);
}
