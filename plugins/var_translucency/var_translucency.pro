#
#   Digital Clock: variable translucency plugin
#   Copyright (C) 2013-2018  Nick Korotysh <nick.korotysh@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#-------------------------------------------------
#
# Project created by QtCreator 2013-07-19T00:06:16
#
#-------------------------------------------------

QT       += core

include(../common.pri)

TARGET = var_translucency

SOURCES += var_translucency.cpp

HEADERS += var_translucency.h

TRANSLATIONS += \
    var_translucency_en.ts \
    var_translucency_ru.ts

include(../../qm_gen.pri)

RESOURCES += var_translucency.qrc

OTHER_FILES += var_translucency.json

win32:RC_FILE = var_translucency.rc
