//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED


#include <winsock2.h>
#include "vmime/net/socket.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


namespace vmime {
namespace platforms {
namespace windows {


class windowsSocket : public vmime::net::socket
{
public:
	windowsSocket();
	~windowsSocket();

public:

	void connect(const vmime::string& address, const vmime::port_t port);
	const bool isConnected() const;
	void disconnect();

	void receive(vmime::string& buffer);
	const int receiveRaw(char* buffer, const int count);

	void send(const vmime::string& buffer);
	void sendRaw(const char* buffer, const int count);

private:

	char m_buffer[65536];
	SOCKET m_desc;
};



class windowsSocketFactory : public vmime::net::socketFactory
{
public:

	ref <vmime::net::socket> create();
};


} // windows
} // platforms
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED
