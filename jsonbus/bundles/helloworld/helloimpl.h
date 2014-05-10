/*
 *   Copyright 2013 Emeric Verschuur <emericv@openihs.org>
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef HELLOWORLDSERVICEIMPL_H
#define HELLOWORLDSERVICEIMPL_H

#include <jsonbus/service/hello/hello.h>

using namespace org::openihs;

class HelloImpl : public Hello {

public:
	HelloImpl();
	virtual void sayHello(const QString &name);
};

#endif // HELLOWORLDSERVICEIMPL_H