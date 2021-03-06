/*
    Digital Clock - beautiful customizable clock with plugins
    Copyright (C) 2013-2018  Nick Korotysh <nick.korotysh@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "plugin_manager.h"

#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QApplication>
#include <QPluginLoader>

#include "iplugin_init.h"
#include "iskin_user_plugin.h"

#include "core/clock_settings.h"

#include "gui/clock_widget.h"
#include "gui/clock_window.h"

namespace digital_clock {
namespace core {

PluginManager::PluginManager(QObject* parent) : QObject(parent)
{
#ifdef Q_OS_MACOS
  search_paths_.append(qApp->applicationDirPath() + "/../PlugIns");
#else
  search_paths_.append(qApp->applicationDirPath() + "/plugins");
#endif
#ifdef Q_OS_LINUX
  search_paths_.append("/usr/share/digital_clock/plugins");
  search_paths_.append("/usr/local/share/digital_clock/plugins");
  search_paths_.append(QDir::homePath() + "/.local/share/digital_clock/plugins");
#endif
  timer_.setInterval(500);
  timer_.setSingleShot(false);
  timer_.start();
}

PluginManager::~PluginManager()
{
  timer_.stop();
}

void PluginManager::SetInitData(const TPluginData& data)
{
  data_ = data;
}

void PluginManager::ListAvailable()
{
  available_.clear();
  QList<QPair<TPluginInfo, bool> > plugins;
  for (auto& path : search_paths_) {
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);
    for (auto& file : files) {
      QString abs_path = dir.filePath(file);
      QPluginLoader loader(abs_path);
      IClockPlugin* plugin = qobject_cast<IClockPlugin*>(loader.instance());
      if (!plugin) continue;
      QJsonObject metadata = loader.metaData().value("MetaData").toObject();
      TPluginInfo info;
      QString c_name = metadata.value("name").toString();
      // *INDENT-OFF*
      auto iter = std::find_if(plugins.cbegin(), plugins.cend(),
          [&] (const QPair<TPluginInfo, bool>& i) -> bool {
            return i.first.metadata[PI_NAME] == c_name;
          });
      // *INDENT-ON*
      if (iter == plugins.cend()) {
        info.metadata[PI_NAME] = c_name;
        info.metadata[PI_VERSION] = metadata.value("version").toString();
        info.metadata[PI_AUTHOR] = metadata.value("author").toString();
        info.metadata[PI_EMAIL] = metadata.value("email").toString();
        info.gui_info = plugin->GetInfo();
        available_[c_name] = abs_path;
        plugins.append(qMakePair(info, metadata.value("configurable").toBool()));
      }
      loader.unload();
    }
  }
  emit SearchFinished(plugins);
}

void PluginManager::LoadPlugins(const QStringList& names)
{
  for (auto& name : names) LoadPlugin(name);
}

void PluginManager::UnloadPlugins(const QStringList& names)
{
  if (!names.isEmpty()) {
    for (auto& name : names) UnloadPlugin(name);
  } else {
    QList<QString> plugins = loaded_.keys();
    for (auto& plugin : plugins) UnloadPlugin(plugin);
  }
}

void PluginManager::EnablePlugin(const QString& name, bool enable)
{
  enable ? LoadPlugin(name) : UnloadPlugin(name);
}

void PluginManager::ConfigurePlugin(const QString& name)
{
  auto iter = loaded_.find(name);
  if (iter != loaded_.end()) {
    IClockPlugin* plugin = qobject_cast<IClockPlugin*>(iter.value()->instance());
    if (plugin) plugin->Configure();
  } else {
    auto iter = available_.constFind(name);
    if (iter == available_.constEnd()) return;
    const QString& file = iter.value();
    if (!QFile::exists(file)) return;
    QPluginLoader* loader = new QPluginLoader(file, this);
    if (!loader->instance()) {
      delete loader;
      return;
    }
    connect(loader->instance(), &IClockPlugin::destroyed, loader, &QPluginLoader::deleteLater);
    IClockPlugin* plugin = qobject_cast<IClockPlugin*>(loader->instance());
    if (plugin) {
      connect(plugin, &IClockPlugin::configured, loader, &QPluginLoader::unload, Qt::QueuedConnection);
      plugin->InitSettings(data_.settings->GetBackend(), name);
      InitPlugin(plugin, false);
      plugin->Configure();
    } else {
      loader->unload();
    }
  }
}

void PluginManager::LoadPlugin(const QString& name)
{
  if (loaded_.contains(name)) return;
  auto iter = available_.constFind(name);
  if (iter == available_.constEnd()) return;
  const QString& file = iter.value();
  if (!QFile::exists(file)) return;
  QPluginLoader* loader = new QPluginLoader(file, this);
  IClockPlugin* plugin = qobject_cast<IClockPlugin*>(loader->instance());
  if (plugin) {
    plugin->InitSettings(data_.settings->GetBackend(), name);
    InitPlugin(plugin, true);
    plugin->Start();
    loaded_[name] = loader;
  } else {
    loader->unload();
    delete loader;
  }
}

void PluginManager::UnloadPlugin(const QString& name)
{
  auto iter = loaded_.find(name);
  if (iter == loaded_.end()) return;
  QPluginLoader* loader = iter.value();
  Q_ASSERT(loader);
  IClockPlugin* plugin = qobject_cast<IClockPlugin*>(loader->instance());
  if (plugin) {
    disconnect(&timer_, SIGNAL(timeout()), plugin, SLOT(TimeUpdateListener()));
    plugin->Stop();
  }
  loader->unload();
  loader->deleteLater();
  loaded_.erase(iter);
}

void PluginManager::InitPlugin(IClockPlugin* plugin, bool connected)
{
  // connect slots which are common for all plugins
  if (connected) {
    connect(&timer_, SIGNAL(timeout()), plugin, SLOT(TimeUpdateListener()));
    connect(this, SIGNAL(UpdateSettings(Option,QVariant)),
            plugin, SLOT(SettingsListener(Option,QVariant)));
  }
  // init skin user plugins
  ISkinUserPlugin* su = qobject_cast<ISkinUserPlugin*>(plugin);
  if (su) {
    // TODO: what about if each window will have own settings in future?
    // for now (and first release with "multiwindow support") all windows will have same settings
    if (connected)
      connect(data_.windows[0]->clockWidget(), &gui::ClockWidget::SkinChanged, su, &ISkinUserPlugin::SetSkin);
    su->SetSkin(data_.windows[0]->clockWidget()->skin());
  }
  // init settings plugins
  ISettingsPlugin* sp = qobject_cast<ISettingsPlugin*>(plugin);
  if (sp && connected) {
    for (auto& w : data_.windows) {
      connect(sp, SIGNAL(OptionChanged(Option,QVariant)),
              w, SLOT(ApplyOption(Option,QVariant)));
    }
    connect(sp, SIGNAL(OptionChanged(Option,QVariant)),
            this, SIGNAL(UpdateSettings(Option,QVariant)));
  }
  ISettingsPluginInit* spi = qobject_cast<ISettingsPluginInit*>(plugin);
  if (spi) spi->Init(data_.settings->GetSettings());
  // init tray plugins
  ITrayPluginInit* tpi = qobject_cast<ITrayPluginInit*>(plugin);
  if (tpi) tpi->Init(data_.tray);
  // init widget plugins
  IWidgetPluginInit* wpi = qobject_cast<IWidgetPluginInit*>(plugin);
  if (wpi) for (auto& w : data_.windows) wpi->Init(w);
}

} // namespace core
} // namespace digital_clock
