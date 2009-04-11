//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2008 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/security/cert/certificateChain.hpp"


namespace vmime {
namespace security {
namespace cert {


certificateChain::certificateChain(const std::vector <ref <certificate> >& certs)
	: m_certs(certs)
{
}


unsigned int certificateChain::getCount() const
{
	return static_cast <unsigned int>(m_certs.size());
}


ref <certificate> certificateChain::getAt(const unsigned int index)
{
	return m_certs[index];
}


} // cert
} // security
} // vmime

