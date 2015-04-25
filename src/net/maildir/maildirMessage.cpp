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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/maildir/maildirMessage.hpp"
#include "vmime/net/maildir/maildirFolder.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"
#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/message.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"


namespace vmime {
namespace net {
namespace maildir {


//
// maildirPart
//

class maildirStructure;

class maildirPart : public part
{
public:

	maildirPart(weak_ref <maildirPart> parent, const int number, const bodyPart& part);
	~maildirPart();


	ref <const structure> getStructure() const;
	ref <structure> getStructure();

	weak_ref <const maildirPart> getParent() const { return (m_parent); }

	const mediaType& getType() const { return (m_mediaType); }
	const int getSize() const { return (m_size); }
	const int getNumber() const { return (m_number); }

	ref <const header> getHeader() const
	{
		if (m_header == NULL)
			throw exceptions::unfetched_object();
		else
			return m_header;
	}

	header& getOrCreateHeader()
	{
		if (m_header != NULL)
			return (*m_header);
		else
			return (*(m_header = vmime::create <header>()));
	}

	const int getHeaderParsedOffset() const { return (m_headerParsedOffset); }
	const int getHeaderParsedLength() const { return (m_headerParsedLength); }

	const int getBodyParsedOffset() const { return (m_bodyParsedOffset); }
	const int getBodyParsedLength() const { return (m_bodyParsedLength); }

private:

	ref <maildirStructure> m_structure;
	weak_ref <maildirPart> m_parent;
	ref <header> m_header;

	int m_number;
	int m_size;
	mediaType m_mediaType;

	int m_headerParsedOffset;
	int m_headerParsedLength;

	int m_bodyParsedOffset;
	int m_bodyParsedLength;
};



//
// maildirStructure
//

class maildirStructure : public structure
{
public:

	maildirStructure()
	{
	}

	maildirStructure(weak_ref <maildirPart> parent, const bodyPart& part)
	{
		m_parts.push_back(vmime::create <maildirPart>(parent, 0, part));
	}

	maildirStructure(weak_ref <maildirPart> parent, const std::vector <ref <const vmime::bodyPart> >& list)
	{
		int number = 0;

		for (unsigned int i = 0 ; i < list.size() ; ++i)
			m_parts.push_back(vmime::create <maildirPart>(parent, number, *list[i]));
	}


	ref <const part> getPartAt(const int x) const
	{
		return m_parts[x];
	}

	ref <part> getPartAt(const int x)
	{
		return m_parts[x];
	}

	const int getPartCount() const
	{
		return m_parts.size();
	}


	static ref <maildirStructure> emptyStructure()
	{
		return m_emptyStructure;
	}

private:

	static ref <maildirStructure> m_emptyStructure;

	std::vector <ref <maildirPart> > m_parts;
};


ref <maildirStructure> maildirStructure::m_emptyStructure = vmime::create <maildirStructure>();



maildirPart::maildirPart(weak_ref <maildirPart> parent, const int number, const bodyPart& part)
	: m_parent(parent), m_header(NULL), m_number(number)
{
	if (part.getBody()->getPartList().size() == 0)
		m_structure = NULL;
	else
	{
		m_structure = vmime::create <maildirStructure>
			(thisWeakRef().dynamicCast <maildirPart>(),
			 part.getBody()->getPartList());
	}

	m_headerParsedOffset = part.getHeader()->getParsedOffset();
	m_headerParsedLength = part.getHeader()->getParsedLength();

	m_bodyParsedOffset = part.getBody()->getParsedOffset();
	m_bodyParsedLength = part.getBody()->getParsedLength();

	m_size = part.getBody()->getContents()->getLength();

	m_mediaType = part.getBody()->getContentType();
}


maildirPart::~maildirPart()
{
}


ref <const structure> maildirPart::getStructure() const
{
	if (m_structure != NULL)
		return m_structure;
	else
		return maildirStructure::emptyStructure();
}


ref <structure> maildirPart::getStructure()
{
	if (m_structure != NULL)
		return m_structure;
	else
		return maildirStructure::emptyStructure();
}



//
// maildirMessage
//

maildirMessage::maildirMessage(weak_ref <maildirFolder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_structure(NULL)
{
	m_folder->registerMessage(this);
}


maildirMessage::~maildirMessage()
{
	if (m_folder)
		m_folder->unregisterMessage(this);
}


void maildirMessage::onFolderClosed()
{
	m_folder = NULL;
}


const int maildirMessage::getNumber() const
{
	return (m_num);
}


const message::uid maildirMessage::getUniqueId() const
{
	return (m_uid);
}


const int maildirMessage::getSize() const
{
	if (m_size == -1)
		throw exceptions::unfetched_object();

	return (m_size);
}


const bool maildirMessage::isExpunged() const
{
	return (m_expunged);
}


ref <const structure> maildirMessage::getStructure() const
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <structure> maildirMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <const header> maildirMessage::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


const int maildirMessage::getFlags() const
{
	if (m_flags == FLAG_UNDEFINED)
		throw exceptions::unfetched_object();

	return (m_flags);
}


void maildirMessage::setFlags(const int flags, const int mode)
{
	if (!m_folder)
		throw exceptions::folder_not_found();

	m_folder->setMessageFlags(m_num, m_num, flags, mode);
}


void maildirMessage::extract(utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool peek) const
{
	extractImpl(os, progress, 0, m_size, start, length, peek);
}


void maildirMessage::extractPart(ref <const part> p, utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool peek) const
{
	const maildirPart& mp = dynamic_cast <const maildirPart&>(p);

	extractImpl(os, progress, mp.getBodyParsedOffset(), mp.getBodyParsedLength(),
		start, length, peek);
}


void maildirMessage::extractImpl(utility::outputStream& os, utility::progressListener* progress,
	const int start, const int length, const int partialStart, const int partialLength,
	const bool /* peek */) const
{
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = m_folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	ref <utility::fileReader> reader = file->getFileReader();
	ref <utility::inputStream> is = reader->getInputStream();

	is->skip(start + partialStart);

	utility::stream::value_type buffer[8192];
	utility::stream::size_type remaining = (partialLength == -1 ? length
		: std::min(partialLength, length));

	const int total = remaining;
	int current = 0;

	if (progress)
		progress->start(total);

	while (!is->eof() && remaining > 0)
	{
		const utility::stream::size_type read =
			is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;
		current += read;

		os.write(buffer, read);

		if (progress)
			progress->progress(current, total);
	}

	if (progress)
		progress->stop(total);

	// TODO: mark as read unless 'peek' is set
}


void maildirMessage::fetchPartHeader(ref <part> p)
{
	ref <maildirPart> mp = p.dynamicCast <maildirPart>();

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = m_folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	ref <utility::fileReader> reader = file->getFileReader();
	ref <utility::inputStream> is = reader->getInputStream();

	is->skip(mp->getHeaderParsedOffset());

	utility::stream::value_type buffer[1024];
	utility::stream::size_type remaining = mp->getHeaderParsedLength();

	string contents;
	contents.reserve(remaining);

	while (!is->eof() && remaining > 0)
	{
		const utility::stream::size_type read =
			is->read(buffer, std::min(remaining, sizeof(buffer)));

		remaining -= read;

		contents.append(buffer, read);
	}

	mp->getOrCreateHeader().parse(contents);
}


void maildirMessage::fetch(weak_ref <maildirFolder> folder, const int options)
{
	if (m_folder != folder)
		throw exceptions::folder_not_found();

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	const utility::file::path path = folder->getMessageFSPath(m_num);
	ref <utility::file> file = fsf->create(path);

	if (options & folder::FETCH_FLAGS)
		m_flags = maildirUtils::extractFlags(path.getLastComponent());

	if (options & folder::FETCH_SIZE)
		m_size = file->getLength();

	if (options & folder::FETCH_UID)
		m_uid = maildirUtils::extractId(path.getLastComponent()).getBuffer();

	if (options & (folder::FETCH_ENVELOPE | folder::FETCH_CONTENT_INFO |
	               folder::FETCH_FULL_HEADER | folder::FETCH_STRUCTURE |
	               folder::FETCH_IMPORTANCE))
	{
		string contents;

		ref <utility::fileReader> reader = file->getFileReader();
		ref <utility::inputStream> is = reader->getInputStream();

		// Need whole message contents for structure
		if (options & folder::FETCH_STRUCTURE)
		{
			utility::stream::value_type buffer[16384];

			contents.reserve(file->getLength());

			while (!is->eof())
			{
				const utility::stream::size_type read = is->read(buffer, sizeof(buffer));
				contents.append(buffer, read);
			}
		}
		// Need only header
		else
		{
			utility::stream::value_type buffer[1024];

			contents.reserve(4096);

			while (!is->eof())
			{
				const utility::stream::size_type read = is->read(buffer, sizeof(buffer));
				contents.append(buffer, read);

				const string::size_type sep1 = contents.rfind("\r\n\r\n");
				const string::size_type sep2 = contents.rfind("\n\n");

				if (sep1 != string::npos)
				{
					contents.erase(contents.begin() + sep1 + 4, contents.end());
					break;
				}
				else if (sep2 != string::npos)
				{
					contents.erase(contents.begin() + sep2 + 2, contents.end());
					break;
				}
			}
		}

		vmime::message msg;
		msg.parse(contents);

		// Extract structure
		if (options & folder::FETCH_STRUCTURE)
		{
			m_structure = vmime::create <maildirStructure>(null, msg);
		}

		// Extract some header fields or whole header
		if (options & (folder::FETCH_ENVELOPE |
		               folder::FETCH_CONTENT_INFO |
		               folder::FETCH_FULL_HEADER |
		               folder::FETCH_IMPORTANCE))
		{
			getOrCreateHeader()->copyFrom(*(msg.getHeader()));
		}
	}
}


ref <header> maildirMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (m_header);
	else
		return (m_header = vmime::create <header>());
}


} // maildir
} // net
} // vmime
