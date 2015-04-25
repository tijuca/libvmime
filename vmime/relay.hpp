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

#ifndef VMIME_RELAY_HPP_INCLUDED
#define VMIME_RELAY_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"

#include "vmime/dateTime.hpp"


namespace vmime
{


/** Trace information about a relay (basic type).
  */

class relay : public headerFieldValue
{
public:

	relay();
	relay(const relay& r);

public:

	ref <component> clone() const;
	void copyFrom(const component& other);
	relay& operator=(const relay& other);

	const std::vector <ref <const component> > getChildComponents() const;

	const string& getFrom() const;
	void setFrom(const string& from);

	const string& getVia() const;
	void setVia(const string& via);

	const string& getBy() const;
	void setBy(const string& by);

	const string& getId() const;
	void setId(const string& id);

	const string& getFor() const;
	void setFor(const string& for_);

	const datetime& getDate() const;
	void setDate(const datetime& date);

	const std::vector <string>& getWithList() const;
	std::vector <string>& getWithList();

private:

	string m_from;
	string m_via;
	string m_by;
	string m_id;
	string m_for;
	std::vector <string> m_with;

	datetime m_date;

public:

	using component::parse;
	using component::generate;

	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_RELAY_HPP_INCLUDED
