/*
 *   Copyright 2012-2014 Emeric Verschuur <emericv@openihs.org>
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

#include <globals.h>
#include <scanner.h>
#include <jsonbus/core/abstractchannel.h>

using namespace JSONBus;

namespace jsonparser {

Scanner::Scanner(const ChannelPtr& channel)
        : jsonparserFlexLexer(nullptr, nullptr), m_channel(channel) {
}

Scanner::~Scanner() {
}

void Scanner::set_debug(bool b) {
    yy_flex_debug = b;
}

int Scanner::LexerInput( char* buf, int max_size ) {
	try {
		buf[0] = m_channel->read();
		return 1;
	} catch (...) {
		return -1;
	}
}

}

#ifdef yylex
#undef yylex
#endif

int jsonparserFlexLexer::yylex() {
    std::cerr << "call jsonparserFlexLexer::yylex()!" << std::endl;
    return 0;
}
