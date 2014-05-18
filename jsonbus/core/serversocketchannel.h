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

#ifndef JSONPARSER_SERVERSOCKETCHANNEL_H
#define JSONPARSER_SERVERSOCKETCHANNEL_H

#include <jsonbus/core/exception.h>
#include <jsonbus/core/selectablechannel.h>
#include <jsonbus/core/abstractchannel.h>

/**
 * @namespace
 */
namespace JSONBus {

/**
 * @brief Abstract channel
 * 
 * @author <a href="mailto:emericv@openihs.org">Emeric Verschuur</a>
 * @date 2014
 * @copyright Apache License, Version 2.0
 */
class ServerSocketChannel: public SelectableChannel {
public:
	/**
	 * @brief Socket constructor
	 */
	ServerSocketChannel(const QString &host, int port);
	
	/**
	 * @brief Socket destructor
	 */
	virtual ~ServerSocketChannel();
	
	/**
	 * @brief Close the channel
	 * @throw IOException on error
	 */
	virtual void close();
	
	/**
	 * @brief Connect
	 */
	ChannelPtr accept();
	
	/**
	 * @brief Get the inner file descriptor if supported
	 * @return the inner file descriptor or -1 if not supported
	 */
	virtual int getFd();
	
protected:
	int m_fd;
};

inline int ServerSocketChannel::getFd() {
	return m_fd;
}

}

#endif // JSONPARSER_SERVERSOCKETCHANNEL_H
