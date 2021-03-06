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

#include "plugin_info_dialog.h"
#include "ui_plugin_info_dialog.h"

namespace digital_clock {
namespace gui {

PluginInfoDialog::PluginInfoDialog(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::PluginInfoDialog)
{
  ui->setupUi(this);
}

PluginInfoDialog::~PluginInfoDialog()
{
  delete ui;
}

void PluginInfoDialog::SetInfo(const TPluginInfo& info)
{
  ui->name_value->setText(info.gui_info.display_name);
  ui->version_value->setText(tr("version: %1").arg(info.metadata[PI_VERSION]));
  if (info.gui_info.icon.isNull()) ui->icon_label->hide();
  else ui->icon_label->setPixmap(info.gui_info.icon);
  ui->description_value->setText(info.gui_info.description);
  ui->author_value->setText(info.metadata[PI_AUTHOR]);
  ui->email_value->setText(info.metadata[PI_EMAIL]);
}

} // namespace gui
} // namespace digital_clock
