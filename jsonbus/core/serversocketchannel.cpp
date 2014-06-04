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

#include "serversocketchannel.h"
#include "logger.h"
#include "iochannel.h"
#include "socketchannel.h"
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <QString>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define THROW_IOEXP_ON_ERR(exp) \
	if ((exp) == -1) throw IOException(QString() + __FILE__ + ":" + QString::number(__LINE__) + ": " + QString::fromLocal8Bit(strerror(errno)))

#define THROW_IOEXP(msg) \
	throw IOException(QString() + __FILE__ + ":" + QString::number(__LINE__) + ": " + QString::fromLocal8Bit(msg == NULL ? strerror(errno) : msg))

namespace JSONBus {

static int __bind(const QString &host, int port, uint opts) {
	addrinfo *addrinfo = NULL, *it;
	bool bound = false;
	const char* error = NULL;
	int fd = -1;
	int optval = 1;
	QRegExp regex("^\\s*\\[([a-fA-F0-9:]+)\\]\\s*$");
	do {
		if (::getaddrinfo((regex.indexIn(host) > -1 ? regex.cap(1).toStdString().c_str() : 
			host.toStdString().c_str()), NULL, NULL, &addrinfo) == -1) {
			error = ::hstrerror(h_errno);
			break;
		}
		it = addrinfo;
		do {
			if ((fd = ::socket(it->ai_family, SOCK_STREAM, 0)) == -1) break;
			if ((opts & ServerSocketChannel::OPT_REUSEADDR) && 
				(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1)) break;
			if ((opts & ServerSocketChannel::OPT_REUSEPORT) && 
				(setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval) == -1)) break;
			((struct sockaddr_in *)(it->ai_addr))->sin_port = htons(port);
			bound |= (::bind(fd, it->ai_addr, it->ai_addrlen) == 0);
			if (bound) {
				break;
			}
			::close(fd);
			it = it->ai_next;
		} while(it);
	} while (false);
	if (addrinfo) {
		freeaddrinfo(addrinfo);
	}
	if (bound) {
		THROW_IOEXP_ON_ERR(listen(fd, opts & ServerSocketChannel::MASK_BACKLOG));
		return fd;
	}
	if (error) {
		THROW_IOEXP(error);
	} else {
		THROW_IOEXP(nullptr);
	}
}

ServerSocketChannel::ServerSocketChannel(const QString& host, int port, uint opts)
: m_fd(__bind(host, port, opts)), m_name(host + ":" + QString::number(port)), 
m_keepAlive(0), m_keepIntlv(0), m_keepIdle(0), m_keepCnt(0) {
	logFiner() << "ServerSocketChannel::start listening on " << m_name;
}

ServerSocketChannel::~ServerSocketChannel() {
	if (isOpen()) {
		close();
	}
}

void ServerSocketChannel::closeFd() {
	logFiner() << "ServerSocketChannel::stop listening on " << m_name;
	::close(m_fd);
}

#define CLIENT_HOST_MAXLEN 256
#define CLIENT_SERV_MAXLEN 32

void ServerSocketChannel::s_accept(int &cldf, QString &name) {
	char client_host[CLIENT_HOST_MAXLEN + 1];
	client_host[0] = '\0';
	char client_serv[CLIENT_SERV_MAXLEN + 1];
	client_serv[0] = '\0';
	struct sockaddr sockaddr_client;
	socklen_t sockaddr_len = sizeof(struct sockaddr);
	cldf = ::accept(m_fd, &sockaddr_client, &sockaddr_len);
	THROW_IOEXP_ON_ERR(cldf);
	if (getnameinfo(&sockaddr_client, sockaddr_len, client_host, CLIENT_HOST_MAXLEN, client_serv, CLIENT_SERV_MAXLEN, 0) == -1) {
		THROW_IOEXP(hstrerror(h_errno));
	}
	name = QString(client_host) + ":" + client_serv;
	if (m_keepAlive) {
		THROW_IOEXP_ON_ERR(setsockopt(cldf, SOL_SOCKET, SO_KEEPALIVE, &m_keepAlive, sizeof m_keepAlive));
	}
	if (m_keepIntlv) {
		THROW_IOEXP_ON_ERR(setsockopt(cldf, IPPROTO_TCP, TCP_KEEPINTVL, &m_keepIntlv, sizeof m_keepIntlv));
	}
	if (m_keepIdle) {
		THROW_IOEXP_ON_ERR(setsockopt(cldf, IPPROTO_TCP, TCP_KEEPIDLE, &m_keepIdle, sizeof m_keepIdle));
	}
	if (m_keepCnt) {
		THROW_IOEXP_ON_ERR(setsockopt(cldf, IPPROTO_TCP, TCP_KEEPCNT, &m_keepCnt, sizeof m_keepCnt));
	}
}

SocketChannelPtr ServerSocketChannel::accept() {
	int cldf;
	QString name;
	s_accept(cldf, name);
	return new SocketChannel(cldf, name);
}

void ServerSocketChannel::updateStatus(int events) {
}


}
