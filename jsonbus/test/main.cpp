/*
 *   Copyright 2012-2013 Emeric Verschuur <emericv@openihs.org>
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

#include <jsonbus/core/sharedptr.h>
#include <jsonbus/core/logger.h>

using namespace JSONBus;
using namespace std;

class A: public SharedData {};

class B: public A {
public:
	inline B(int i): i(i) {}
	inline ~B() {logFiner() << "destroy B";}
	int i;
};

class C: public A {
public:
	inline C(int i): i(i) {}
	inline ~C() {logFiner() << "destroy C";}
	int i;
};

void dump(SharedPtr<B> b) {
	logInfo() << "b: " << b->i;
}

int main(int argc, char **argv) {
	
	try {
		SharedPtr<C> c = null;
		SharedPtr<B> b = new B(2);
		SharedPtr<A> ab = b, ac = new C(1);
		
// 		c = ab; // throw InvalidClassException
		c = ac;
		
		dump(ab);
		ab = b = nullptr;
		
		logInfo() << "(  ab == nullptr) is " << (ab == nullptr);
		logInfo() << "(null == nullptr) is " << (null == nullptr);
		
		logFine() << "ac => " << ac;
		logFine() << " c => " << c;
		logInfo() << "(ac == c) is " << (ac == c);
		
		
		logFine() << null;
		logFine() << b;
		
// 		b->i = 0; // throw NullPointerException
		
	} catch (Exception &e) {
		logCrit() << "terminate called after throwing an " << e;
	}
	
	return 0;
}
