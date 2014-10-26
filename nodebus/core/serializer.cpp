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

#include "common.h"
#include "serializer.h"
#include <QVariant>

#define DEBUG_TK(t) 

#define BCON_TOKEN_END		((quint8)0x00)
#define BCON_TOKEN_NULL		((quint8)0x01)
#define BCON_TOKEN_TRUE		((quint8)0x02)
#define BCON_TOKEN_FALSE	((quint8)0x03)
#define BCON_TOKEN_BYTE		((quint8)0x04)
#define BCON_TOKEN_INT16	((quint8)0x05)
#define BCON_TOKEN_UINT16	((quint8)0x06)
#define BCON_TOKEN_INT32	((quint8)0x07)
#define BCON_TOKEN_UINT32	((quint8)0x08)
#define BCON_TOKEN_INT64	((quint8)0x09)
#define BCON_TOKEN_UINT64	((quint8)0x0A)
#define BCON_TOKEN_DOUBLE	((quint8)0x0B)
#define BCON_TOKEN_DATETIME	((quint8)0x0C)
#define BCON_TOKEN_LIST		((quint8)0x0E)
#define BCON_TOKEN_MAP		((quint8)0x0F)
#define BCON_TOKEN_DATA6	((quint8)0xA0)
#define BCON_TOKEN_STRING6	((quint8)0xC0)
#define BCON_TOKEN_DATA12	((quint8)0x10)
#define BCON_TOKEN_DATA20	((quint8)0x20)
#define BCON_TOKEN_DATA36	((quint8)0x30)
#define BCON_TOKEN_STRING12	((quint8)0x50)
#define BCON_TOKEN_STRING20	((quint8)0x60)
#define BCON_TOKEN_STRING36	((quint8)0x70)

#define LENGTH2P6		64
#define LENGTH2P12		4096
#define LENGTH2P20		1048576
#define LENGTH2P36		68719476736

#define BSON_TOKEN_END		((quint8)0x00)
#define BSON_TOKEN_NULL		((quint8)0x0A)
#define BSON_TOKEN_TRUE		((quint8)0x00)
#define BSON_TOKEN_FALSE	((quint8)0x01)
#define BSON_TOKEN_INT32	((quint8)0x10)
#define BSON_TOKEN_INT64	((quint8)0x12)
#define BSON_TOKEN_DOUBLE	((quint8)0x01)
#define BSON_TOKEN_DATETIME	((quint8)0x09)
#define BSON_TOKEN_STRING	((quint8)0x02)
#define BSON_TOKEN_DATA		((quint8)0x05)
#define BSON_TOKEN_BOOL		((quint8)0x08)
#define BSON_TOKEN_MAP		((quint8)0x03)
#define BSON_TOKEN_LIST		((quint8)0x04)
#define BSON_TOKEN_GENERIC	((quint8)0x00)

#define JSON_NULL		QString("null")
#define JSON_TRUE		QString("true")
#define JSON_FALSE		QString("false")
#define JSON_OBJECT_BEGIN	QString("{")
#define JSON_OBJECT_END		QString("}")
#define JSON_MEMBER_SEP		QString(":")
#define JSON_ELEMENT_SEP	QString(",")
#define JSON_ARRAY_BEGIN	QString("[")
#define JSON_ARRAY_END		QString("]")

namespace NodeBus {

quint32 Serializer::FORMAT_COMPACT = 0x00000020u;


QString Serializer::toJSONString(const QVariant& variant, quint32 flags) {
	QByteArray data;
	QDataStream dataStream(data);
	serialize(dataStream, variant, JSON, flags);
	return QString::fromLocal8Bit(data);
}

void Serializer::serialize(QDataStream& dataStream, const QVariant& variant, FileFormat format, uint32_t flags) {
	return Serializer(dataStream, format).serialize(variant, flags);
}

Serializer::Serializer(QDataStream &dataStream, FileFormat format)
: m_dataStream(dataStream), m_format(format) {
	m_dataStream.setByteOrder(QDataStream::LittleEndian);
}

Serializer::~Serializer() {
}

void Serializer::toFile(const QString &fileName, const QVariant &variant, FileFormat format, uint32_t flags) {
	switch (format) {
		case FileFormat::BCON:
		case FileFormat::BSON:
		case FileFormat::JSON: {
			QFile file(fileName);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				throw IOException(file.errorString());
			}
			QDataStream stream(&file);
			return Serializer(stream, format).serialize(variant, flags);
		}
		case FileFormat::IDL:
			throw Exception("Unsupported IDL format");
	}
}

void Serializer::serialize(const QVariant& variant, quint32 flags) {
	switch (m_format) {
		case FileFormat::BCON:
			serializeBCON(variant);
			break;
		case FileFormat::BSON:
			m_dataStream << serializeBSONDocument(variant);
			break;
		case FileFormat::JSON:
			serializeJSON(variant, flags);
			break;
		case FileFormat::IDL:
			throw Exception("Unsupported IDL format");
	}
}

static QString sanitizeString( QString str ) {
	str.replace( QLatin1String( "\\" ), QLatin1String( "\\\\" ) );
	str.replace( QLatin1String( "\"" ), QLatin1String( "\\\"" ) );
	str.replace( QLatin1String( "\b" ), QLatin1String( "\\b" ) );
	str.replace( QLatin1String( "\f" ), QLatin1String( "\\f" ) );
	str.replace( QLatin1String( "\n" ), QLatin1String( "\\n" ) );
	str.replace( QLatin1String( "\r" ), QLatin1String( "\\r" ) );
	str.replace( QLatin1String( "\t" ), QLatin1String( "\\t" ) );
	return QString( QLatin1String( "\"%1\"" ) ).arg( str );
}

template <typename T>
inline void Serializer::write(QByteArray& output, T value) {
	QDataStream(output) << value;
}

template <typename T>
inline void Serializer::write(char type, T value) {
	m_dataStream << type << value;
}

void Serializer::serializeBCON(const QVariant &variant, const QString *key) {
	switch (variant.type()) {
		case QVariant::Invalid:
			DEBUG_TK("BCON_TOKEN_NULL");
			m_dataStream << BCON_TOKEN_NULL;
			break;
		case QVariant::Bool: // Case of BCON boolean
			DEBUG_TK(variant.toBool() ? "BCON_TOKEN_TRUE": "BCON_TOKEN_FALSE");
			m_dataStream << (variant.toBool() ? BCON_TOKEN_TRUE: BCON_TOKEN_FALSE);
			break;
		case QVariant::Char: // Case of BCON boolean
			DEBUG_TK("BCON_TOKEN_BYTE");
			m_dataStream << BCON_TOKEN_BYTE
					<< variant.toChar().toAscii();
			break;
		case QVariant::Int:
		{
			qint32 num = variant.toInt();
			if ((num & 0x7FFFFF80) != 0) {
				DEBUG_TK("BCON_TOKEN_INT8");
				m_dataStream << BCON_TOKEN_BYTE
					<< variant.toChar().toAscii();
			} else if ((num & 0x7FFF8000) != 0) {
				DEBUG_TK("BCON_TOKEN_INT16");
				write<qint16>(BCON_TOKEN_INT16, num);
			} else {
				DEBUG_TK("BCON_TOKEN_INT32");
				write<qint32>(BCON_TOKEN_INT32, num);
			}
			break;
		}
		case QVariant::UInt: {
			quint32 num = variant.toUInt();
			if ((num & 0xFFFF0000u) != 0) {
				DEBUG_TK("BCON_TOKEN_UINT32");
				write<quint32>(BCON_TOKEN_UINT32, num);
			} else {
				DEBUG_TK("BCON_TOKEN_UINT16");
				write<quint16>(BCON_TOKEN_UINT16, num);
			}
			break;
		}
		case QVariant::LongLong:
		{
			DEBUG_TK("BCON_TOKEN_INT64");
			write<qint64>(BCON_TOKEN_INT64, variant.toLongLong());
			break;
		}
		case QVariant::ULongLong:
		{
			DEBUG_TK("BCON_TOKEN_UINT64");
			write<quint64>(BCON_TOKEN_UINT64, variant.toULongLong());
			break;
		}
		case QVariant::Double:
		{
			DEBUG_TK("BCON_TOKEN_DOUBLE");
			write<double>(BCON_TOKEN_DOUBLE, variant.toDouble());
			break;
		}
		case QVariant::DateTime:
		case QVariant::Date:
		case QVariant::Time:
		{
			DEBUG_TK("BCON_TOKEN_DATETIME");
			write<qint64>(BCON_TOKEN_DATETIME, variant.toDateTime().toMSecsSinceEpoch());
			break;
		}
		case QVariant::List: // Case of BCON array
		{
			DEBUG_TK("BCON_TOKEN_LIST");
			m_dataStream << BCON_TOKEN_LIST;
			const QVariantList elements = variant.toList();
			for (auto it = elements.begin(); it != elements.end(); it++) {
				serializeBCON(*it);
			}
			m_dataStream << BCON_TOKEN_END;
			break;
		}
		case QVariant::Map: // Case of BCON object
		{
			DEBUG_TK("BCON_TOKEN_MAP");
			m_dataStream << BCON_TOKEN_MAP;
			const QVariantMap elements = variant.toMap();
			for (auto it = elements.begin(); it != elements.end(); it++) {
				serializeBCON(it.value(), &(it.key()));
			}
			m_dataStream << BCON_TOKEN_END;
			break;
		}
		case QVariant::String:
		{
			QByteArray data = variant.toString().toLocal8Bit();
			int len = data.length();
			if (len < (LENGTH2P6)) {
				DEBUG_TK("BCON_TOKEN_STRING6");
				m_dataStream << ((BCON_TOKEN_STRING6) | (len & 0x3F));
				m_dataStream << data;
			} else if (len < (LENGTH2P12)) {
				DEBUG_TK("BCON_TOKEN_STRING12");
				m_dataStream << ((BCON_TOKEN_STRING12) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4);
				m_dataStream << data;
			} else if (len < (LENGTH2P20)) {
				DEBUG_TK("BCON_TOKEN_STRING20");
				m_dataStream << ((BCON_TOKEN_STRING20) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4)
						<< ((len & 0xFF000) >> 12);
				m_dataStream << data;
			} else if (len < (LENGTH2P36)) {
				DEBUG_TK("BCON_TOKEN_STRING36");
				m_dataStream << ((BCON_TOKEN_STRING36) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4)
						<< ((len & 0xFF000) >> 12)
						<< (len >> 20);
				m_dataStream << data;
			} else {
				throw SerializerException("Fatal: too big String (length=" + QString::number(len) + ")");
			}
			break;
		}
		case QVariant::ByteArray: // Case of BCON string
		{
			DEBUG_TK();
			QByteArray data = variant.toByteArray();
			int len = data.length();
			if (len < (LENGTH2P6)) {
				DEBUG_TK("BCON_TOKEN_DATA6");
				m_dataStream << ((BCON_TOKEN_DATA6) | (len & 0x3F));
				m_dataStream << data;
			} else if (len < (LENGTH2P12)) {
				DEBUG_TK("BCON_TOKEN_DATA12");
				m_dataStream << ((BCON_TOKEN_DATA12) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4);
				m_dataStream << data;
			} else if (len < (LENGTH2P20)) {
				DEBUG_TK("BCON_TOKEN_DATA20");
				m_dataStream << ((BCON_TOKEN_DATA20) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4)
						<< ((len & 0xFF000) >> 12);
				m_dataStream << data;
			} else if (len < (LENGTH2P36)) {
				DEBUG_TK("BCON_TOKEN_DATA36");
				m_dataStream << ((BCON_TOKEN_DATA36) | (len & 0x0F))
						<< ((len & 0xFF0) >> 4)
						<< ((len & 0xFF000) >> 12)
						<< (len >> 20);
				m_dataStream << data;
			} else {
				throw SerializerException("Fatal: too big byte array (length=" + QString::number(len) + ")");
			}
			break;
		}
		default:
			throw SerializerException("Fatal: QVariant type not managed.");
		
	}
		
	if (key) m_dataStream << (*key).toLocal8Bit() << '\0';
}

void Serializer::serializeJSON(const QVariant &variant, quint32 flags) {
	switch (variant.type()) {
		case QVariant::Invalid:
		{
			m_dataStream << JSON_NULL;
			break;
		}
		case QVariant::Bool: // Case of JSON boolean
		{
			m_dataStream << (variant.toBool() ? JSON_TRUE: JSON_FALSE);
			break;
		}
		case QVariant::Map: // Case of JSON object
		{
			bool compact = (flags & FORMAT_COMPACT) != 0;
			quint8 indentStep = INDENT(flags);
			quint32 indentOff = (flags >> 16) + indentStep;
			flags = (flags & 0xFFFF) | (indentOff << 16);
			m_dataStream << JSON_OBJECT_BEGIN;
			const QVariantMap elements = variant.toMap();
			auto it = elements.begin();
			if (it != elements.end()) {
				if (indentOff != 0) m_dataStream << '\n' << QString(indentOff, ' ');
				m_dataStream << sanitizeString(it.key()) << JSON_MEMBER_SEP;
				if (!compact) m_dataStream << ' ';
				serializeJSON(it.value(), flags);
				it++;
				while (it != elements.end()) {
					m_dataStream << JSON_ELEMENT_SEP;
					if (indentOff != 0) m_dataStream << '\n' << QString(indentOff, ' ');
					else if (!compact) m_dataStream << ' ';
					m_dataStream << sanitizeString(it.key()) << JSON_MEMBER_SEP;
					if (!compact) m_dataStream << ' ';
					serializeJSON(it.value(), flags);
					it++;
				}
				if (indentOff != 0) m_dataStream << '\n' << QString(indentOff - indentStep, ' ');
			}
			m_dataStream << JSON_OBJECT_END;
			break;
		}
		case QVariant::List: // Case of JSON array
		{
			bool compact = (flags & FORMAT_COMPACT) != 0;
			quint8 indentStep = INDENT(flags);
			quint32 indentOff = (flags >> 16) + indentStep;
			flags = (flags & 0xFFFF) | (indentOff << 16);
			m_dataStream << JSON_ARRAY_BEGIN;
			const QVariantList elements = variant.toList();
			auto it = elements.begin();
			if (it != elements.end()) {
				if (indentOff != 0) m_dataStream << '\n' << QString(indentOff, ' ');
				serializeJSON(*it, flags);
				it++;
				while (it != elements.end()) {
					m_dataStream << JSON_ELEMENT_SEP;
					if (indentOff != 0) m_dataStream << '\n' << QString(indentOff, ' ');
					else if (!compact) m_dataStream << ' ';
					serializeJSON(*it, flags);
					it++;
				}
				if (indentOff != 0) m_dataStream << '\n' << QString(indentOff - indentStep, ' ');
			}
			m_dataStream << JSON_ARRAY_END;
			break;
		}
		case QVariant::String:
		case QVariant::ByteArray: // Case of JSON string
		{
			m_dataStream << sanitizeString(variant.toString());
			break;
		}
		case QVariant::Char:
		case QVariant::Int:
		case QVariant::UInt:
		case QVariant::LongLong:
		case QVariant::ULongLong:
		{
			m_dataStream << QString::number(variant.toInt()).replace("inf", "infinity");
			break;
		}
		case QVariant::Double:
		{
			m_dataStream << QString::number(variant.toDouble()).replace("inf", "infinity");
			break;
		}
		default:
			throw SerializerException("Fatal: QVariant type not managed.");
	}
}

QByteArray Serializer::serializeBSONDocument(const QVariant &variant) {
	QByteArray payload;
	switch (variant.type()) {
		case QVariant::Map: // Case of BSON object
		{
			const QVariantMap elements = variant.toMap();
			for (auto it = elements.begin(); it != elements.end(); it++) {
				payload.append(serializeBSONElt(it.value(), it.key()));
			}
			break;
		}
		case QVariant::List: // Case of BSON array
		{
			const QVariantList elements = variant.toList();
			uint i = 0;
			for (auto it = elements.begin(); it != elements.end(); it++, i++) {
				payload.append(serializeBSONElt(*it, QString::number(i)));
			}
			break;
		}
		default:
			throw SerializerException("Fatal: Invalid document.");
	}
	QByteArray ret;
	write<quint32>(ret, payload.length() + 5);
	ret.append(payload);
	ret.append((char)BSON_TOKEN_END);
	return ret;
}

QByteArray Serializer::serializeBSONElt(const QVariant& variant, const QString &key) {
	QByteArray ret;
	switch (variant.type()) {
		case QVariant::Invalid:
			ret.append((char)BSON_TOKEN_NULL);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			break;
		case QVariant::Bool: // Case of BSON boolean
			ret.append((char)BSON_TOKEN_BOOL);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			ret.append(variant.toBool() ? BSON_TOKEN_TRUE: BSON_TOKEN_FALSE);
			break;
		case QVariant::UInt:
		case QVariant::Int:
		case QVariant::Char:
		{
			ret.append((char)BSON_TOKEN_INT32);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint32>(ret, variant.toUInt());
			break;
		}
		case QVariant::ULongLong:
		case QVariant::LongLong:
		{
			ret.append((char)BSON_TOKEN_INT64);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint64>(ret, variant.toUInt());
			break;
		}
		case QVariant::Double:
		{
			ret.append((char)BSON_TOKEN_DOUBLE);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint64>(ret, variant.toDouble());
			break;
		}
		case QVariant::DateTime:
		case QVariant::Date:
		case QVariant::Time:
		{
			ret.append((char)BSON_TOKEN_DATETIME);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint64>(ret, variant.toDateTime().toMSecsSinceEpoch());
			break;
		}
		case QVariant::Map: // Case of BSON object
		{
			ret.append((char)BSON_TOKEN_MAP);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			ret.append(serializeBSONDocument(variant));
			break;
		}
		case QVariant::List: // Case of BSON array
		{
			ret.append((char)BSON_TOKEN_LIST);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			ret.append(serializeBSONDocument(variant));
			break;
		}
		case QVariant::String:
		{
			QByteArray data = variant.toString().toLocal8Bit();
			ret.append((char)BSON_TOKEN_STRING);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint32>(ret, data.length() + 1);
			ret.append(data);
			ret.append('\0');
			break;
		}
		case QVariant::ByteArray: // Case of BSON string
		{
			QByteArray data = variant.toByteArray();
			ret.append((char)BSON_TOKEN_DATA);
			ret.append(key.toLocal8Bit());
			ret.append('\0');
			write<quint32>(ret, data.length());
			ret.append((char)BSON_TOKEN_GENERIC);
			ret.append(data);
			break;
		}
		default:
			throw SerializerException("Fatal: QVariant type not managed.");
		
	}
	return ret;
}

}
