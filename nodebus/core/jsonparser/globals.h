/*
 * Copyright (C) 2012-2014 Emeric Verschuur <emericv@mbedsys.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

namespace jsonparser {

#ifndef JSONPARSER_DRIVER_H_TYPES
#define JSONPARSER_DRIVER_H_TYPES
typedef QVariant variant_t;
typedef QString string_t;
typedef QMap<string_t, variant_t> object_t;
typedef QList<variant_t> array_t;
#endif // JSONPARSER_DRIVER_H_TYPES

class Driver;

}

#endif // GLOBALS_H
