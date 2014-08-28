/*
 *   Copyright 2012-2014 Emeric Verschuur <emericv@mbedsys.org>
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *		   http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/**
 * @brief NodeBus : Exception management.
 * 
 * @author <a href="mailto:emericv@mbedsys.org">Emeric Verschuur</a>
 * @date 2014
 * @copyright Apache License, Version 2.0
 */
#ifndef NODEBUS_EXCEPTION_H
#define NODEBUS_EXCEPTION_H

#ifndef NODEBUS_EXPORT
#define NODEBUS_EXPORT
#endif

#define NODEBUS_EXCEPTION_BACKTRACE_SIZE 32

#include <QString>
#include <QObject>
#include <QtCore>
#include <execinfo.h>

#define nodebus_declare_exception(ename, eparent) \
class NODEBUS_EXPORT ename:public eparent {\
public:\
    inline ename(const QString &msg = ""):eparent(msg) {}\
	\
	inline ename(const ExceptionDataPtr &data):eparent(data) {}\
	\
    inline ename(const ename &exception):eparent(exception) {}\
	\
    inline virtual ~ename() throw() {}\
	\
	inline virtual void raise() const {\
		throw *this;\
	}\
	\
	inline virtual ename *clone() const {\
		return new ename(*this);\
	}\
};

#include <nodebus/core/sharedptr.h>

namespace NodeBus {

class Logger;

class NODEBUS_EXPORT ExceptionData: public SharedData {
	ExceptionData(const SharedData&);
public:
	/// @brief Message
	QString message;
	
	/// @brief backtrace address fonction table
	void *backtrace[NODEBUS_EXCEPTION_BACKTRACE_SIZE];
	
	/// @brief backtrace address fonction table size
	uint backtraceSize;
	
	/**
	 * @brief ExceptionData contructor from a message
	 * 
	 * @param message Message
	 */
	ExceptionData(const QString &message);
    virtual ~ExceptionData();
};

typedef SharedPtr<ExceptionData> ExceptionDataPtr;

inline ExceptionData::ExceptionData(const QString &message): message(message), 
	backtraceSize(::backtrace(backtrace, NODEBUS_EXCEPTION_BACKTRACE_SIZE)) {}
inline ExceptionData::~ExceptionData() {
}

/**
 * This class can manage exceptions.
 * @brief NodeBus : Exceptions.
 */
class NODEBUS_EXPORT Exception : public QtConcurrent::Exception {
	friend class Logger;
	ExceptionDataPtr d;
protected:
	template<class X>
	SharedPtr<X> data();
public:
	/**
	 * @brief Exception constructor.
	 * @param message exeption message.
	 */
	Exception(const QString &message = "");
	
	/**
	 * @brief Exception constructor.
	 * @param message exeption message.
	 */
	Exception(const ExceptionDataPtr &data);
	
	/**
	 * @brief Exception constructor.
	 * @param message exeption message.
	 */
	Exception(const Exception &exception);

	/**
	 * @brief Exception destructor.
	 */
	virtual ~Exception() throw();

	/**
	 * @brief Get the exeption message.
	 * @return QString message.
	 */
	const QString message() const;

	/**
	 * @brief Get the exeption message.
	 * @return QString message.
	 */
	virtual const char *what() const throw();
	
	/**
	 * @brief Rethrow this exception
	 */
	void raise();
	
	/**
	 * @brief Clone this exception
	 * 
	 * @return an address to the new exception
	 */
	virtual Exception *clone() const;
};

template <typename X>
inline SharedPtr< X > Exception::data() {
	return d;
}

nodebus_declare_exception(PointerException, Exception);
nodebus_declare_exception(NullPointerException, PointerException);
nodebus_declare_exception(InvalidClassException, PointerException);
nodebus_declare_exception(IllegalOperationException, Exception);
nodebus_declare_exception(UnsupportedOperationException, Exception);
nodebus_declare_exception(IOException, Exception);
nodebus_declare_exception(IOTimeoutException, IOException);
nodebus_declare_exception(EOFException, IOException);

inline Exception::Exception(const QString& message): d(new ExceptionData(message)) {}
inline Exception::Exception(const ExceptionDataPtr &data): d(data) {}
inline Exception::Exception(const Exception& exception): d(exception.d) {}
inline Exception::~Exception() throw() {}
inline Exception* Exception::clone() const {
	return new Exception(*this);
}
inline const QString Exception::message() const {
	return d->message;
}
inline void Exception::raise() {
	throw *this;
}

}

#endif
