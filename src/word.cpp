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

#include "vmime/word.hpp"
#include "vmime/text.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/smartPtr.hpp"
#include "vmime/parserHelpers.hpp"

#include "vmime/encoder.hpp"
#include "vmime/encoderB64.hpp"
#include "vmime/encoderQP.hpp"


namespace vmime
{


word::word()
	: m_charset(charset::getLocaleCharset())
{
}


word::word(const word& w)
	: headerFieldValue(), m_buffer(w.m_buffer), m_charset(w.m_charset)
{
}


word::word(const string& buffer) // Defaults to locale charset
	: m_buffer(buffer), m_charset(charset::getLocaleCharset())
{
}


word::word(const string& buffer, const charset& charset)
	: m_buffer(buffer), m_charset(charset)
{
}


ref <word> word::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition,
	bool prevIsEncoded, bool* isEncoded, bool isFirst)
{
	string::size_type pos = position;

	// Ignore white-spaces:
	//   - before the first word
	//   - between two encoded words
	//   - after the last word
	while (pos < end && parserHelpers::isSpace(buffer[pos]))
		++pos;

	string::size_type startPos = pos;
	string unencoded;

	while (pos < end)
	{
		// End of line: does not occur in the middle of an encoded word. This is
		// used to remove folding white-spaces from unencoded text.
		if (buffer[pos] == '\n')
		{
			string::size_type endPos = pos;

			if (pos > position && buffer[pos - 1] == '\r')
				--endPos;

			while (pos != end && parserHelpers::isSpace(buffer[pos]))
				++pos;

			unencoded += string(buffer.begin() + startPos, buffer.begin() + endPos);
			unencoded += ' ';

			startPos = pos;
		}
		// Start of an encoded word
		else if (pos + 8 < end &&  // 8 = "=?(.+)?(.+)?(.*)?="
		         buffer[pos] == '=' && buffer[pos + 1] == '?')
		{
			// Check whether there is some unencoded text before
			unencoded += string(buffer.begin() + startPos, buffer.begin() + pos);

			if (!unencoded.empty())
			{
				ref <word> w = vmime::create <word>(unencoded, charset(charsets::US_ASCII));
				w->setParsedBounds(position, pos);

				if (newPosition)
					*newPosition = pos;

				if (isEncoded)
					*isEncoded = false;

				return (w);
			}

			// ...else find the finish sequence '?=' and return an encoded word
			const string::size_type wordStart = pos;

			pos += 2;

			while (pos < end && buffer[pos] != '?')
				++pos;

			if (pos < end)
			{
				++pos; // skip '?' between charset and encoding

				while (pos < end && buffer[pos] != '?')
					++pos;

				if (pos < end)
				{
					++pos; // skip '?' between encoding and encoded data
				}
			}

			while (pos < end)
			{
				if (buffer[pos] == '\n')
				{
					// End of line not allowed in the middle of an encoded word:
					// treat this text as unencoded text (see *).
					break;
				}
				else if (buffer[pos] == '?' && pos + 1 < end && buffer[pos + 1] == '=')
				{
					// Found the finish sequence
					break;
				}

				++pos;
			}

			if (pos == end) // not a valid word (no finish sequence)
				continue;
			else if (buffer[pos] == '\n')  // (*)
				continue;

			pos += 2; // ?=

			ref <word> w = vmime::create <word>();
			w->parse(buffer, wordStart, pos, NULL);

			if (newPosition)
				*newPosition = pos;

			if (isEncoded)
				*isEncoded = true;

			return (w);
		}

		++pos;
	}

	// Treat unencoded text at the end of the buffer
	if (end != startPos)
	{
		if (startPos != pos && !isFirst && prevIsEncoded)
			unencoded += ' ';

		unencoded += string(buffer.begin() + startPos, buffer.begin() + end);

		ref <word> w = vmime::create <word>(unencoded, charset(charsets::US_ASCII));
		w->setParsedBounds(position, end);

		if (newPosition)
			*newPosition = end;

		if (isEncoded)
			*isEncoded = false;

		return (w);
	}

	return (null);
}


const std::vector <ref <word> > word::parseMultiple(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	std::vector <ref <word> > res;
	ref <word> w;

	string::size_type pos = position;

	bool prevIsEncoded = false;

	while ((w = word::parseNext(buffer, pos, end, &pos, prevIsEncoded, &prevIsEncoded, (w == NULL))) != NULL)
		res.push_back(w);

	if (newPosition)
		*newPosition = pos;

	return (res);
}


void word::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	if (position + 6 < end && // 6 = "=?(.+)?(.*)?="
	    buffer[position] == '=' && buffer[position + 1] == '?')
	{
		string::const_iterator p = buffer.begin() + position + 2;
		const string::const_iterator pend = buffer.begin() + end;

		const string::const_iterator charsetPos = p;

		for ( ; p != pend && *p != '?' ; ++p);

		if (p != pend) // a charset is specified
		{
			const string::const_iterator charsetEnd = p;
			const string::const_iterator encPos = ++p; // skip '?'

			for ( ; p != pend && *p != '?' ; ++p);

			if (p != pend) // an encoding is specified
			{
				//const string::const_iterator encEnd = p;
				const string::const_iterator dataPos = ++p; // skip '?'

				for ( ; p != pend && !(*p == '?' && *(p + 1) == '=') ; ++p);

				if (p != pend) // some data is specified
				{
					const string::const_iterator dataEnd = p;
					p += 2; // skip '?='

					encoder* theEncoder = NULL;

					// Base-64 encoding
					if (*encPos == 'B' || *encPos == 'b')
					{
						theEncoder = new encoderB64;
					}
					// Quoted-Printable encoding
					else if (*encPos == 'Q' || *encPos == 'q')
					{
						theEncoder = new encoderQP;
						theEncoder->getProperties()["rfc2047"] = true;
					}

					if (theEncoder)
					{
						// Decode text
						string decodedBuffer;

						utility::inputStreamStringAdapter ein(string(dataPos, dataEnd));
						utility::outputStreamStringAdapter eout(decodedBuffer);

						theEncoder->decode(ein, eout);
						delete (theEncoder);

						m_buffer = decodedBuffer;
						m_charset = charset(string(charsetPos, charsetEnd));

						setParsedBounds(position, p - buffer.begin());

						if (newPosition)
							*newPosition = (p - buffer.begin());

						return;
					}
				}
			}
		}
	}

	// Unknown encoding or malformed encoded word: treat the buffer as ordinary text (RFC-2047, Page 9).
	m_buffer = string(buffer.begin() + position, buffer.begin() + end);
	m_charset = charsets::US_ASCII;

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void word::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	generate(os, maxLineLength, curLinePos, newLinePos, 0, true);
}


void word::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos, const int flags,
	const bool isFirstWord) const
{
	string::size_type curLineLength = curLinePos;

	// Calculate the number of ASCII chars to check whether encoding is needed
	// and _which_ encoding to use.
	const string::size_type asciiCount =
		utility::stringUtils::countASCIIchars(m_buffer.begin(), m_buffer.end());

	bool noEncoding = (flags & text::FORCE_NO_ENCODING) ||
	    (!(flags & text::FORCE_ENCODING) && asciiCount == m_buffer.length());

	if (noEncoding)
	{
		// We will fold lines without encoding them.

		string::const_iterator lastWSpos = m_buffer.end(); // last white-space position
		string::const_iterator curLineStart = m_buffer.begin(); // current line start

		string::const_iterator p = m_buffer.begin();
		const string::const_iterator end = m_buffer.end();

		bool finished = false;
		bool newLine = false;

		while (!finished)
		{
			for ( ; p != end ; ++p, ++curLineLength)
			{
				// Exceeded maximum line length, but we have found a white-space
				// where we can cut the line...
				if (curLineLength >= maxLineLength && lastWSpos != end)
					break;

				if (*p == ' ' || *p == '\t')
				{
					// Remember the position of this white-space character
					lastWSpos = p;
				}
			}

			if (p != end)
				++curLineLength;

			if (p == end || lastWSpos == end)
			{
				// If we are here, it means that we have found no whitespace
				// before the first "maxLineLength" characters. In this case,
				// we write the full line no matter of the max line length...

				if (!newLine && p != end && lastWSpos == end &&
				    !isFirstWord && curLineStart == m_buffer.begin())
				{
					// Here, we are continuing on the line of previous encoded
					// word, but there is not even enough space to put the
					// first word of this line, so we start a new line.
					if (flags & text::NO_NEW_LINE_SEQUENCE)
					{
						os << CRLF;
						curLineLength = 0;
					}
					else
					{
						os << NEW_LINE_SEQUENCE;
						curLineLength = NEW_LINE_SEQUENCE_LENGTH;
					}

					p = curLineStart;
					lastWSpos = end;
					newLine = true;
				}
				else
				{
					os << string(curLineStart, p);

					if (p == end)
					{
						finished = true;
					}
					else
					{
						if (flags & text::NO_NEW_LINE_SEQUENCE)
						{
							os << CRLF;
							curLineLength = 0;
						}
						else
						{
							os << NEW_LINE_SEQUENCE;
							curLineLength = NEW_LINE_SEQUENCE_LENGTH;
						}

						curLineStart = p;
						lastWSpos = end;
						newLine = true;
					}
				}
			}
			else
			{
				// In this case, there will not be enough space on the line for all the
				// characters _after_ the last white-space; so we cut the line at this
				// last white-space.

#if 1
				if (curLineLength != 1 && !isFirstWord)
					os << " "; // Separate from previous word
#endif

				os << string(curLineStart, lastWSpos);

				if (flags & text::NO_NEW_LINE_SEQUENCE)
				{
					os << CRLF;
					curLineLength = 0;
				}
				else
				{
					os << NEW_LINE_SEQUENCE;
					curLineLength = NEW_LINE_SEQUENCE_LENGTH;
				}

				curLineStart = lastWSpos + 1;

				p = lastWSpos + 1;
				lastWSpos = end;
				newLine = true;
			}
		}
	}
	/*
		RFC #2047:
		4. Encodings

		Initially, the legal values for "encoding" are "Q" and "B".  These
		encodings are described below.  The "Q" encoding is recommended for
		use when most of the characters to be encoded are in the ASCII
		character set; otherwise, the "B" encoding should be used.
		Nevertheless, a mail reader which claims to recognize 'encoded-word's
		MUST be able to accept either encoding for any character set which it
		supports.
	*/
	else
	{
		// We will encode _AND_ fold lines

		/*
			RFC #2047:
			2. Syntax of encoded-words

			" While there is no limit to the length of a multiple-line header
			  field, each line of a header field that contains one or more
			  'encoded-word's is limited to 76 characters. "
		*/

		const string::size_type maxLineLength3 =
			(maxLineLength == lineLengthLimits::infinite)
				? maxLineLength
				: std::min(maxLineLength, static_cast <string::size_type>(76));

		// Base64 if more than 60% non-ascii, quoted-printable else (default)
		const string::size_type asciiPercent = (m_buffer.length() == 0 ? 100 : (100 * asciiCount) / m_buffer.length());
		const string::value_type encoding = (asciiPercent <= 40) ? 'B' : 'Q';

		string wordStart("=?" + m_charset.getName() + "?" + encoding + "?");
		string wordEnd("?=");

		const string::size_type minWordLength = wordStart.length() + wordEnd.length();
		const string::size_type maxLineLength2 = (maxLineLength3 < minWordLength + 1)
			? maxLineLength3 + minWordLength + 1 : maxLineLength3;

		// Checks whether remaining space on this line is usable. If too few
		// characters can be encoded, start a new line.
		bool startNewLine = true;

		if (curLineLength + 2 < maxLineLength2)
		{
			const string::size_type remainingSpaceOnLine = maxLineLength2 - curLineLength - 2;

			if (remainingSpaceOnLine < minWordLength + 10)
			{
				// Space for no more than 10 encoded chars!
				// It is not worth while to continue on this line...
				startNewLine = true;
			}
			else
			{
				// OK, there is enough usable space on the current line.
				startNewLine = false;
			}
		}

		if (startNewLine)
		{
			os << NEW_LINE_SEQUENCE;
			curLineLength = NEW_LINE_SEQUENCE_LENGTH;
		}

		// Encode and fold input buffer
		string::const_iterator pos = m_buffer.begin();
		string::size_type remaining = m_buffer.length();

		encoder* theEncoder = NULL;

		if (encoding == 'B') theEncoder = new encoderB64;
		else theEncoder = new encoderQP;

		string qpEncodedBuffer;

		if (encoding == 'Q')
		{
			theEncoder->getProperties()["rfc2047"] = true;

			// In the case of Quoted-Printable encoding, we cannot simply encode input
			// buffer line by line. So, we encode the whole buffer and we will fold it
			// in the next loop...
			utility::inputStreamStringAdapter in(m_buffer);
			utility::outputStreamStringAdapter out(qpEncodedBuffer);

			theEncoder->encode(in, out);

			pos = qpEncodedBuffer.begin();
			remaining = qpEncodedBuffer.length();
		}

#if 1
		if (curLineLength != 1 && !isFirstWord)
		{
			os << " "; // Separate from previous word
			++curLineLength;
		}
#endif

		for ( ; remaining ; )
		{
			// Start a new encoded word
			os << wordStart;
			curLineLength += minWordLength;

			// Compute the number of encoded chars that will fit on this line
			const string::size_type fit = maxLineLength2 - curLineLength;

			// Base-64 encoding
			if (encoding == 'B')
			{
				// TODO: WARNING! "Any encoded word which encodes a non-integral
				// number of characters or octets is incorrectly formed."

				// Here, we have a formula to compute the maximum number of source
				// characters to encode knowing the maximum number of encoded chars
				// (with Base64, 3 bytes of input provide 4 bytes of output).
				string::size_type count = (fit > 1) ? ((fit - 1) * 3) / 4 : 1;
				if (count > remaining) count = remaining;

				utility::inputStreamStringAdapter in
					(m_buffer, pos - m_buffer.begin(), pos - m_buffer.begin() + count);

				curLineLength += theEncoder->encode(in, os);

				pos += count;
				remaining -= count;
			}
			// Quoted-Printable encoding
			else
			{
				// TODO: WARNING! "Any encoded word which encodes a non-integral
				// number of characters or octets is incorrectly formed."

				// All we have to do here is to take a certain number of character
				// (that is less than or equal to "fit") from the QP encoded buffer,
				// but we also make sure not to fold a "=XY" encoded char.
				const string::const_iterator qpEnd = qpEncodedBuffer.end();
				string::const_iterator lastFoldPos = pos;
				string::const_iterator p = pos;
				string::size_type n = 0;

				while (n < fit && p != qpEnd)
				{
					if (*p == '=')
					{
						if (n + 3 >= fit)
						{
							lastFoldPos = p;
							break;
						}

						p += 3;
						n += 3;
					}
					else
					{
						++p;
						++n;
					}
				}

				if (lastFoldPos == pos)
					lastFoldPos = p;

				os << string(pos, lastFoldPos);

				curLineLength += (lastFoldPos - pos) + 1;

				pos += n;
				remaining -= n;
			}

			// End of the encoded word
			os << wordEnd;

			if (remaining)
			{
				os << NEW_LINE_SEQUENCE;
				curLineLength = NEW_LINE_SEQUENCE_LENGTH;
			}
		}

		delete (theEncoder);
	}

	if (newLinePos)
		*newLinePos = curLineLength;
}


#if VMIME_WIDE_CHAR_SUPPORT

const wstring word::getDecodedText() const
{
	wstring out;

	charset::decode(m_buffer, out, m_charset);

	return (out);
}

#endif


word& word::operator=(const word& w)
{
	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
	return (*this);
}


word& word::operator=(const string& s)
{
	m_buffer = s;
	return (*this);
}


void word::copyFrom(const component& other)
{
	const word& w = dynamic_cast <const word&>(other);

	m_buffer = w.m_buffer;
	m_charset = w.m_charset;
}


const bool word::operator==(const word& w) const
{
	return (m_charset == w.m_charset && m_buffer == w.m_buffer);
}


const bool word::operator!=(const word& w) const
{
	return (m_charset != w.m_charset || m_buffer != w.m_buffer);
}


const string word::getConvertedText(const charset& dest) const
{
	string out;

	charset::convert(m_buffer, out, m_charset, dest);

	return (out);
}


ref <component> word::clone() const
{
	return vmime::create <word>(m_buffer, m_charset);
}


const charset& word::getCharset() const
{
	return (m_charset);
}


void word::setCharset(const charset& ch)
{
	m_charset = ch;
}


const string& word::getBuffer() const
{
	return (m_buffer);
}


string& word::getBuffer()
{
	return (m_buffer);
}


void word::setBuffer(const string& buffer)
{
	m_buffer = buffer;
}


const std::vector <ref <const component> > word::getChildComponents() const
{
	return std::vector <ref <const component> >();
}


} // vmime
