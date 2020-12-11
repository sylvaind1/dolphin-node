// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinNode/Config/Graphics/AdvancedWidget.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include "Core/Config/GraphicsSettings.h"
#include "Core/Config/SYSCONFSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"

#include "DolphinNode/Config/Graphics/GraphicsBool.h"
#include "DolphinNode/Config/Graphics/GraphicsChoice.h"
#include "DolphinNode/Config/Graphics/GraphicsInteger.h"
#include "DolphinNode/Config/Graphics/GraphicsWindow.h"
#include "DolphinNode/Config/ToolTipControls/ToolTipCheckBox.h"
#include "DolphinNode/Settings.h"

#include "VideoCommon/VideoConfig.h"

AdvancedWidget::AdvancedWidget(GraphicsWindow* parent)
{
  CreateWidgets();
  LoadSettings();
  ConnectWidgets();
  AddDescriptions();

  connect(parent, &GraphicsWindow::BackendChanged, this, &AdvancedWidget::OnBackendChanged);
  connect(&Settings::Instance(), &Settings::EmulationStateChanged, this,
          [=](Core::State state) { OnEmulationStateChanged(state != Core::State::Uninitialized); });

  OnBackendChanged();
  OnEmulationStateChanged(Core::GetState() != Core::State::Uninitialized);
}

void AdvancedWidget::CreateWidgets()
{
  auto* main_layout = new QVBoxLayout;

  // Debugging
  auto* debugging_box = new QGroupBox(tr("Debugging"));
  auto* debugging_layout = new QGridLayout();
  debugging_box->setLayout(debugging_layout);

  m_enable_wireframe = new GraphicsBool(tr("Enable Wireframe"), Config::GFX_ENABLE_WIREFRAME);
  m_show_statistics = new GraphicsBool(tr("Show Statistics"), Config::GFX_OVERLAY_STATS);
  m_enable_format_overlay =
      new GraphicsBool(tr("Texture Format Overlay"), Config::GFX_TEXFMT_OVERLAY_ENABLE);
  m_enable_api_validation =
      new GraphicsBool(tr("Enable API Validation Layers"), Config::GFX_ENABLE_VALIDATION_LAYER);

  debugging_layout->addWidget(m_enable_wireframe, 0, 0);
  debugging_layout->addWidget(m_show_statistics, 0, 1);
  debugging_layout->addWidget(m_enable_format_overlay, 1, 0);
  debugging_layout->addWidget(m_enable_api_validation, 1, 1);

  // Utility
  auto* utility_box = new QGroupBox(tr("Utility"));
  auto* utility_layout = new QGridLayout();
  utility_box->setLayout(utility_layout);

  m_load_custom_textures = new GraphicsBool(tr("Load Custom Textures"), Config::GFX_HIRES_TEXTURES);
  m_prefetch_custom_textures =
      new GraphicsBool(tr("Prefetch Custom Textures"), Config::GFX_CACHE_HIRES_TEXTURES);
  m_dump_efb_target = new GraphicsBool(tr("Dump EFB Target"), Config::GFX_DUMP_EFB_TARGET);
  m_disable_vram_copies =
      new GraphicsBool(tr("Disable EFB VRAM Copies"), Config::GFX_HACK_DISABLE_COPY_TO_VRAM);

  utility_layout->addWidget(m_load_custom_textures, 0, 0);
  utility_layout->addWidget(m_prefetch_custom_textures, 0, 1);

  utility_layout->addWidget(m_disable_vram_copies, 1, 0);

  utility_layout->addWidget(m_dump_efb_target, 1, 1);

  // Freelook
  auto* freelook_box = new QGroupBox(tr("Free Look"));
  auto* freelook_layout = new QGridLayout();
  freelook_box->setLayout(freelook_layout);

  m_enable_freelook = new GraphicsBool(tr("Enable"), Config::GFX_FREE_LOOK);
  m_freelook_control_type = new GraphicsChoice({tr("Six Axis"), tr("First Person"), tr("Orbital")},
                                               Config::GFX_FREE_LOOK_CONTROL_TYPE);

  freelook_layout->addWidget(m_enable_freelook, 0, 0);
  freelook_layout->addWidget(new QLabel(tr("Control Type:")), 1, 0);
  freelook_layout->addWidget(m_freelook_control_type, 1, 1);

  // Texture dumping
  auto* texture_dump_box = new QGroupBox(tr("Texture Dumping"));
  auto* texture_dump_layout = new QGridLayout();
  texture_dump_box->setLayout(texture_dump_layout);
  m_dump_textures = new GraphicsBool(tr("Enable"), Config::GFX_DUMP_TEXTURES);
  m_dump_base_textures = new GraphicsBool(tr("Dump Base Textures"), Config::GFX_DUMP_BASE_TEXTURES);
  m_dump_mip_textures = new GraphicsBool(tr("Dump Mip Maps"), Config::GFX_DUMP_MIP_TEXTURES);

  texture_dump_layout->addWidget(m_dump_textures, 0, 0);

  texture_dump_layout->addWidget(m_dump_base_textures, 1, 0);
  texture_dump_layout->addWidget(m_dump_mip_textures, 1, 1);

  // Frame dumping
  auto* dump_box = new QGroupBox(tr("Frame Dumping"));
  auto* dump_layout = new QGridLayout();
  dump_box->setLayout(dump_layout);

  m_use_fullres_framedumps = new GraphicsBool(tr("Dump at Internal Resolution"),
                                              Config::GFX_INTERNAL_RESOLUTION_FRAME_DUMPS);
  m_dump_use_ffv1 = new GraphicsBool(tr("Use Lossless Codec (FFV1)"), Config::GFX_USE_FFV1);
  m_dump_bitrate = new GraphicsInteger(0, 1000000, Config::GFX_BITRATE_KBPS, 1000);

  dump_layout->addWidget(m_use_fullres_framedumps, 0, 0);
#if defined(HAVE_FFMPEG)
  dump_layout->addWidget(m_dump_use_ffv1, 0, 1);
  dump_layout->addWidget(new QLabel(tr("Bitrate (kbps):")), 1, 0);
  dump_layout->addWidget(m_dump_bitrate, 1, 1);
#endif

  // Misc.
  auto* misc_box = new QGroupBox(tr("Misc"));
  auto* misc_layout = new QGridLayout();
  misc_box->setLayout(misc_layout);

  m_enable_cropping = new GraphicsBool(tr("Crop"), Config::GFX_CROP);
  m_enable_prog_scan = new ToolTipCheckBox(tr("Enable Progressive Scan"));
  m_backend_multithreading =
      new GraphicsBool(tr("Backend Multithreading"), Config::GFX_BACKEND_MULTITHREADING);

  misc_layout->addWidget(m_enable_cropping, 0, 0);
  misc_layout->addWidget(m_enable_prog_scan, 0, 1);
  misc_layout->addWidget(m_backend_multithreading, 1, 0);
#ifdef _WIN32
  m_borderless_fullscreen =
      new GraphicsBool(tr("Borderless Fullscreen"), Config::GFX_BORDERLESS_FULLSCREEN);

  misc_layout->addWidget(m_borderless_fullscreen, 1, 1);
#endif

  // Experimental.
  auto* experimental_box = new QGroupBox(tr("Experimental"));
  auto* experimental_layout = new QGridLayout();
  experimental_box->setLayout(experimental_layout);

  m_defer_efb_access_invalidation =
      new GraphicsBool(tr("Defer EFB Cache Invalidation"), Config::GFX_HACK_EFB_DEFER_INVALIDATION);

  experimental_layout->addWidget(m_defer_efb_access_invalidation, 0, 0);

  main_layout->addWidget(debugging_box);
  main_layout->addWidget(utility_box);
  main_layout->addWidget(freelook_box);
  main_layout->addWidget(texture_dump_box);
  main_layout->addWidget(dump_box);
  main_layout->addWidget(misc_box);
  main_layout->addWidget(experimental_box);
  main_layout->addStretch();

  setLayout(main_layout);
}

void AdvancedWidget::ConnectWidgets()
{
  connect(m_load_custom_textures, &QCheckBox::toggled, this, &AdvancedWidget::SaveSettings);
  connect(m_dump_use_ffv1, &QCheckBox::toggled, this, &AdvancedWidget::SaveSettings);
  connect(m_enable_prog_scan, &QCheckBox::toggled, this, &AdvancedWidget::SaveSettings);
  connect(m_enable_freelook, &QCheckBox::toggled, this, &AdvancedWidget::SaveSettings);
  connect(m_dump_textures, &QCheckBox::toggled, this, &AdvancedWidget::SaveSettings);
}

void AdvancedWidget::LoadSettings()
{
  m_prefetch_custom_textures->setEnabled(Config::Get(Config::GFX_HIRES_TEXTURES));
  m_dump_bitrate->setEnabled(!Config::Get(Config::GFX_USE_FFV1));

  m_enable_prog_scan->setChecked(Config::Get(Config::SYSCONF_PROGRESSIVE_SCAN));

  m_freelook_control_type->setEnabled(Config::Get(Config::GFX_FREE_LOOK));
  m_dump_mip_textures->setEnabled(Config::Get(Config::GFX_DUMP_TEXTURES));
  m_dump_base_textures->setEnabled(Config::Get(Config::GFX_DUMP_TEXTURES));
}

void AdvancedWidget::SaveSettings()
{
  m_prefetch_custom_textures->setEnabled(Config::Get(Config::GFX_HIRES_TEXTURES));
  m_dump_bitrate->setEnabled(!Config::Get(Config::GFX_USE_FFV1));

  Config::SetBase(Config::SYSCONF_PROGRESSIVE_SCAN, m_enable_prog_scan->isChecked());

  m_freelook_control_type->setEnabled(Config::Get(Config::GFX_FREE_LOOK));
  m_dump_mip_textures->setEnabled(Config::Get(Config::GFX_DUMP_TEXTURES));
  m_dump_base_textures->setEnabled(Config::Get(Config::GFX_DUMP_TEXTURES));
}

void AdvancedWidget::OnBackendChanged()
{
}

void AdvancedWidget::OnEmulationStateChanged(bool running)
{
  m_enable_prog_scan->setEnabled(!running);
}

void AdvancedWidget::AddDescriptions()
{
  static const char TR_WIREFRAME_DESCRIPTION[] =
      QT_TR_NOOP("Renders the scene as a wireframe.<br><br><dolphin_emphasis>If unsure, leave "
                 "this unchecked.</dolphin_emphasis>");
  static const char TR_SHOW_STATS_DESCRIPTION[] =
      QT_TR_NOOP("Shows various rendering statistics.<br><br><dolphin_emphasis>If unsure, "
                 "leave this unchecked.</dolphin_emphasis>");
  static const char TR_TEXTURE_FORMAT_DESCRIPTION[] =
      QT_TR_NOOP("Modifies textures to show the format they're encoded in.<br><br>May require "
                 "an emulation "
                 "reset to apply.<br><br><dolphin_emphasis>If unsure, leave this "
                 "unchecked.</dolphin_emphasis>");
  static const char TR_VALIDATION_LAYER_DESCRIPTION[] =
      QT_TR_NOOP("Enables validation of API calls made by the video backend, which may assist in "
                 "debugging graphical issues.<br><br><dolphin_emphasis>If unsure, leave this "
                 "unchecked.</dolphin_emphasis>");
  static const char TR_DUMP_TEXTURE_DESCRIPTION[] =
      QT_TR_NOOP("Dumps decoded game textures based on the other flags to "
                 "User/Dump/Textures/<game_id>/.<br><br><dolphin_emphasis>If unsure, leave "
                 "this unchecked.</dolphin_emphasis>");
  static const char TR_DUMP_MIP_TEXTURE_DESCRIPTION[] = QT_TR_NOOP(
      "Whether to dump mipmapped game textures to "
      "User/Dump/Textures/<game_id>/.  This includes arbitrary mipmapped textures if 'Arbitrary "
      "Mipmap Detection' is enabled in Enhancements.<br><br><dolphin_emphasis>If unsure, leave "
      "this checked.</dolphin_emphasis>");
  static const char TR_DUMP_BASE_TEXTURE_DESCRIPTION[] = QT_TR_NOOP(
      "Whether to dump base game textures to "
      "User/Dump/Textures/<game_id>/.  This includes arbitrary base textures if 'Arbitrary "
      "Mipmap Detection' is enabled in Enhancements.<br><br><dolphin_emphasis>If unsure, leave "
      "this checked.</dolphin_emphasis>");
  static const char TR_LOAD_CUSTOM_TEXTURE_DESCRIPTION[] =
      QT_TR_NOOP("Loads custom textures from User/Load/Textures/<game_id>/ and "
                 "User/Load/DynamicInputTextures/<game_id>/.<br><br><dolphin_emphasis>If "
                 "unsure, leave this "
                 "unchecked.</dolphin_emphasis>");
  static const char TR_CACHE_CUSTOM_TEXTURE_DESCRIPTION[] = QT_TR_NOOP(
      "Caches custom textures to system RAM on startup.<br><br>This can require exponentially "
      "more RAM but fixes possible stuttering.<br><br><dolphin_emphasis>If unsure, leave this "
      "unchecked.</dolphin_emphasis>");
  static const char TR_DUMP_EFB_DESCRIPTION[] =
      QT_TR_NOOP("Dumps the contents of EFB copies to User/Dump/Textures/.<br><br "
                 "/><dolphin_emphasis>If unsure, leave this "
                 "unchecked.</dolphin_emphasis>");
  static const char TR_DISABLE_VRAM_COPIES_DESCRIPTION[] =
      QT_TR_NOOP("Disables the VRAM copy of the EFB, forcing a round-trip to RAM. Inhibits all "
                 "upscaling.<br><br><dolphin_emphasis>If unsure, leave this "
                 "unchecked.</dolphin_emphasis>");
  static const char TR_INTERNAL_RESOLUTION_FRAME_DUMPING_DESCRIPTION[] = QT_TR_NOOP(
      "Creates frame dumps and screenshots at the internal resolution of the renderer, rather than "
      "the size of the window it is displayed within.<br><br>If the aspect ratio is "
      "widescreen, the "
      "output image will be scaled horizontally to preserve the vertical resolution.<br><br "
      "/><dolphin_emphasis>If "
      "unsure, leave this unchecked.</dolphin_emphasis>");
#if defined(HAVE_FFMPEG)
  static const char TR_USE_FFV1_DESCRIPTION[] =
      QT_TR_NOOP("Encodes frame dumps using the FFV1 codec.<br><br><dolphin_emphasis>If "
                 "unsure, leave this unchecked.</dolphin_emphasis>");
#endif
  static const char TR_FREE_LOOK_DESCRIPTION[] = QT_TR_NOOP(
      "Allows manipulation of the in-game camera. Move the mouse while holding the right button "
      "to pan or middle button to roll.<br><br>Use the WASD keys while holding SHIFT to move "
      "the "
      "camera. Press SHIFT+2 to increase speed or SHIFT+1 to decrease speed. Press SHIFT+R "
      "to reset the camera or SHIFT+F to reset the speed.<br><br><dolphin_emphasis>If unsure, "
      "leave this unchecked.</dolphin_emphasis>");
  static const char TR_FREE_LOOK_CONTROL_TYPE_DESCRIPTION[] = QT_TR_NOOP(
      "Changes the in-game camera type during freelook.<br><br>"
      "Six Axis: Offers full camera control on all axes, akin to moving a spacecraft in zero "
      "gravity. This is the most powerful freelook option but is the most challenging to use.<br "
      "/><br>"
      "First Person: Controls the free camera similarly to a first person video game. The camera "
      "can rotate and travel, but roll is impossible. Easy to use, but limiting.<br><br>"
      "Orbital: Rotates the free camera around the original camera. Has no lateral movement, only "
      "rotation and you may zoom up to the camera's origin point.");
  static const char TR_CROPPING_DESCRIPTION[] = QT_TR_NOOP(
      "Crops the picture from its native aspect ratio to 4:3 or "
      "16:9.<br><br><dolphin_emphasis>If unsure, leave this unchecked.</dolphin_emphasis>");
  static const char TR_PROGRESSIVE_SCAN_DESCRIPTION[] = QT_TR_NOOP(
      "Enables progressive scan if supported by the emulated software. Most games don't have "
      "any issue with this.<br><br><dolphin_emphasis>If unsure, leave this "
      "unchecked.</dolphin_emphasis>");
  static const char TR_BACKEND_MULTITHREADING_DESCRIPTION[] =
      QT_TR_NOOP("Enables multithreaded command submission in backends where supported. Enabling "
                 "this option may result in a performance improvement on systems with more than "
                 "two CPU cores. Currently, this is limited to the Vulkan backend.<br><br "
                 "/><dolphin_emphasis>If unsure, "
                 "leave this checked.</dolphin_emphasis>");
  static const char TR_DEFER_EFB_ACCESS_INVALIDATION_DESCRIPTION[] = QT_TR_NOOP(
      "Defers invalidation of the EFB access cache until a GPU synchronization command "
      "is executed. If disabled, the cache will be invalidated with every draw call. "
      "<br><br>May improve performance in some games which rely on CPU EFB Access at the cost "
      "of stability.<br><br><dolphin_emphasis>If unsure, leave this "
      "unchecked.</dolphin_emphasis>");

#ifdef _WIN32
  static const char TR_BORDERLESS_FULLSCREEN_DESCRIPTION[] = QT_TR_NOOP(
      "Implements fullscreen mode with a borderless window spanning the whole screen instead of "
      "using exclusive mode. Allows for faster transitions between fullscreen and windowed mode, "
      "but slightly increases input latency, makes movement less smooth and slightly decreases "
      "performance.<br><br><dolphin_emphasis>If unsure, leave this "
      "unchecked.</dolphin_emphasis>");
#endif

  m_enable_wireframe->SetDescription(tr(TR_WIREFRAME_DESCRIPTION));
  m_show_statistics->SetDescription(tr(TR_SHOW_STATS_DESCRIPTION));
  m_enable_format_overlay->SetDescription(tr(TR_TEXTURE_FORMAT_DESCRIPTION));
  m_enable_api_validation->SetDescription(tr(TR_VALIDATION_LAYER_DESCRIPTION));
  m_dump_textures->SetDescription(tr(TR_DUMP_TEXTURE_DESCRIPTION));
  m_dump_mip_textures->SetDescription(tr(TR_DUMP_MIP_TEXTURE_DESCRIPTION));
  m_dump_base_textures->SetDescription(tr(TR_DUMP_BASE_TEXTURE_DESCRIPTION));
  m_load_custom_textures->SetDescription(tr(TR_LOAD_CUSTOM_TEXTURE_DESCRIPTION));
  m_prefetch_custom_textures->SetDescription(tr(TR_CACHE_CUSTOM_TEXTURE_DESCRIPTION));
  m_dump_efb_target->SetDescription(tr(TR_DUMP_EFB_DESCRIPTION));
  m_disable_vram_copies->SetDescription(tr(TR_DISABLE_VRAM_COPIES_DESCRIPTION));
  m_use_fullres_framedumps->SetDescription(tr(TR_INTERNAL_RESOLUTION_FRAME_DUMPING_DESCRIPTION));
#ifdef HAVE_FFMPEG
  m_dump_use_ffv1->SetDescription(tr(TR_USE_FFV1_DESCRIPTION));
#endif
  m_enable_cropping->SetDescription(tr(TR_CROPPING_DESCRIPTION));
  m_enable_prog_scan->SetDescription(tr(TR_PROGRESSIVE_SCAN_DESCRIPTION));
  m_enable_freelook->SetDescription(tr(TR_FREE_LOOK_DESCRIPTION));
  m_freelook_control_type->SetTitle(tr("Free Look Control Type"));
  m_freelook_control_type->SetDescription(tr(TR_FREE_LOOK_CONTROL_TYPE_DESCRIPTION));
  m_backend_multithreading->SetDescription(tr(TR_BACKEND_MULTITHREADING_DESCRIPTION));
#ifdef _WIN32
  m_borderless_fullscreen->SetDescription(tr(TR_BORDERLESS_FULLSCREEN_DESCRIPTION));
#endif
  m_defer_efb_access_invalidation->SetDescription(tr(TR_DEFER_EFB_ACCESS_INVALIDATION_DESCRIPTION));
}
