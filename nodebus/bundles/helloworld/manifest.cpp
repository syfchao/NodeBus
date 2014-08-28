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

#include <nodebus/osgi/bundlemanifest.h>
#include "helloworld.h"

MANIFEST_BUNDLE_NAME			("Hello World");
MANIFEST_BUNDLE_SYMBOLIC_NAME	("org.mbedsys.helloworld");
MANIFEST_BUNDLE_VERSION			("1.0.0");
MANIFEST_BUNDLE_ACTIVATOR		(HelloWorld);
