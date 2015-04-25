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

#ifndef VMIME_BODYPARTATTACHMENT_HPP_INCLUDED
#define VMIME_BODYPARTATTACHMENT_HPP_INCLUDED


#include "vmime/attachment.hpp"

#include "vmime/contentDispositionField.hpp"
#include "vmime/contentTypeField.hpp"


namespace vmime
{


/** An attachment related to a local body part.
  */
class bodyPartAttachment : public attachment
{
	friend class creator;

protected:

	bodyPartAttachment(ref <const bodyPart> part);

public:

	const mediaType getType() const;
	const word getName() const;
	const text getDescription() const;
	const encoding getEncoding() const;

	const ref <const contentHandler> getData() const;

	ref <const object> getPart() const;
	ref <const header> getHeader() const;

private:

	void generateIn(bodyPart& parent) const;

	ref <const contentDispositionField> getContentDisposition() const;
	ref <const contentTypeField> getContentType() const;


	ref <const bodyPart> m_part;
};


} // vmime


#endif // VMIME_BODYPARTATTACHMENT_HPP_INCLUDED

