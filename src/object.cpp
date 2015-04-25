//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/types.hpp"
#include "vmime/object.hpp"

#include <algorithm>  // std::find
#include <sstream>    // std::ostringstream
#include <stdexcept>  // std::runtime_error


#ifndef VMIME_BUILDING_DOC


namespace vmime
{


object::object()
	: m_strongCount(0)
{
}


object::object(const object&)
	: m_strongCount(0)
{
	// Not used
}


object::~object()
{
	for (std::vector <utility::weak_ref_base*>::iterator
	     it = m_weakRefs.begin() ; it != m_weakRefs.end() ; ++it)
	{
		(*it)->notifyObjectDestroyed();
	}

#if VMIME_DEBUG
	if (m_strongCount != 0)
	{
		std::ostringstream oss;
		oss << "ERROR: Deleting object and strong count != 0."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG
}


void object::addStrong() const
{
	++m_strongCount;
}


void object::addWeak(utility::weak_ref_base* w) const
{
	m_weakRefs.push_back(w);
}


void object::releaseStrong() const
{
	if (--m_strongCount == 0)
		delete this;
}


void object::releaseWeak(utility::weak_ref_base* w) const
{
	std::vector <utility::weak_ref_base*>::iterator
		it = std::find(m_weakRefs.begin(), m_weakRefs.end(), w);

	if (it != m_weakRefs.end())
		m_weakRefs.erase(it);
#if VMIME_DEBUG
	else
	{
		std::ostringstream oss;
		oss << "ERROR: weak ref does not exist anymore!"
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG
}


ref <object> object::thisRef()
{
#if VMIME_DEBUG
	if (m_strongCount == 0)
	{
		std::ostringstream oss;
		oss << "ERROR: thisRef() MUST NOT be called from the object constructor."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG

	return ref <object>::fromPtr(this);
}


ref <const object> object::thisRef() const
{
#if VMIME_DEBUG
	if (m_strongCount == 0)
	{
		std::ostringstream oss;
		oss << "ERROR: thisRef() MUST NOT be called from the object constructor."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG

	return ref <const object>::fromPtr(this);
}


weak_ref <object> object::thisWeakRef()
{
#if VMIME_DEBUG
	if (m_strongCount == 0)
	{
		std::ostringstream oss;
		oss << "ERROR: thisWeakRef() MUST NOT be called from the object constructor."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG

	return weak_ref <object>(thisRef());
}


weak_ref <const object> object::thisWeakRef() const
{
#if VMIME_DEBUG
	if (m_strongCount == 0)
	{
		std::ostringstream oss;
		oss << "ERROR: thisWeakRef() MUST NOT be called from the object constructor."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG

	return weak_ref <const object>(thisRef());
}


const int object::getStrongRefCount() const
{
	return m_strongCount;
}


const int object::getWeakRefCount() const
{
	return static_cast <const int>(m_weakRefs.size());
}


} // vmime


#endif // VMIME_BUILDING_DOC

