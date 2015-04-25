//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
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
//
// Derived from cryptoapi implementation, originally based on the
// public domain implementation written by Colin Plumb in 1993.
//
// Copyright (C) Cryptoapi developers.
//
// Algorithm Copyright:
//
//     Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
//     rights reserved.
//
//     License to copy and use this software is granted provided that it
//     is identified as the "RSA Data Security, Inc. MD5 Message-Digest
//     Algorithm" in all material mentioning or referencing this software
//     or this function.
//
//     License is also granted to make and use derivative works provided
//     that such works are identified as "derived from the RSA Data
//     Security, Inc. MD5 Message-Digest Algorithm" in all material
//     mentioning or referencing the derived work.
//
//     RSA Data Security, Inc. makes no representations concerning either
//     the merchantability of this software or the suitability of this
//     software forany particular purpose. It is provided "as is"
//     without express or implied warranty of any kind.
//     These notices must be retained in any copies of any part of this
//     documentation and/or software.

#include "vmime/security/digest/md5/md5MessageDigest.hpp"

#include <cstring>


namespace vmime {
namespace security {
namespace digest {
namespace md5 {


md5MessageDigest::md5MessageDigest()
{
	init();
}


void md5MessageDigest::reset()
{
	init();
}


void md5MessageDigest::init()
{
	m_hash[0] = 0x67452301;
	m_hash[1] = 0xefcdab89;
	m_hash[2] = 0x98badcfe;
	m_hash[3] = 0x10325476;

	m_byteCount = 0;
	m_finalized = false;
}


static void copyUint8Array(vmime_uint8* dest, const vmime_uint8* src, unsigned long count)
{
	for ( ; count >= 4 ; count -= 4, dest += 4, src += 4)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
	}

	for ( ; count ; --count, ++dest, ++src)
		dest[0] = src[0];
}


static inline vmime_uint32 swapUint32(const vmime_uint32 D)
{
	return ((D << 24) | ((D << 8) & 0x00FF0000) | ((D >> 8) & 0x0000FF00) | (D >> 24));
}


static inline void swapUint32Array(vmime_uint32* buf, unsigned long words)
{
	for ( ; words >= 4 ; words -= 4, buf += 4)
	{
		buf[0] = swapUint32(buf[0]);
		buf[1] = swapUint32(buf[1]);
		buf[2] = swapUint32(buf[2]);
		buf[3] = swapUint32(buf[3]);
	}

	for ( ; words ; --words, ++buf)
		buf[0] = swapUint32(buf[0]);
}


void md5MessageDigest::update(const byte_t b)
{
	update(&b, 1);
}


void md5MessageDigest::update(const string& s)
{
	update(reinterpret_cast <const byte_t*>(s.data()), s.length());
}


void md5MessageDigest::update(const byte_t* data, const unsigned long offset,
	const unsigned long len)
{
	update(data + offset, len);
}


void md5MessageDigest::update(const byte_t* data, const unsigned long length)
{
	const unsigned long avail = 64 - (m_byteCount & 0x3f);
	unsigned long len = length;

	m_byteCount += len;

	if (avail > len)
	{
		copyUint8Array(m_block + (64 - avail), data, len);
		return;
	}

	copyUint8Array(m_block + (64 - avail), data, avail);
	transformHelper();

	data += avail;
	len -= avail;

	while (len >= 64)
	{
		copyUint8Array(m_block, data, 64);
		transformHelper();

		data += 64;
		len -= 64;
	}

	copyUint8Array(m_block, data, len);
}


void md5MessageDigest::finalize(const string& s)
{
	update(s);
	finalize();
}


void md5MessageDigest::finalize(const byte_t* buffer, const unsigned long len)
{
	update(buffer, len);
	finalize();
}


void md5MessageDigest::finalize(const byte_t* buffer,
	const unsigned long offset, const unsigned long len)
{
	update(buffer, offset, len);
	finalize();
}


void md5MessageDigest::finalize()
{
	const long offset = m_byteCount & 0x3f;

	vmime_uint8* p = m_block + offset;
	long padding = 56 - (offset + 1);

	*p++ = 0x80;

	if (padding < 0)
	{
		memset(p, 0x00, padding + 8);
		transformHelper();
		p = m_block;
		padding = 56;
	}

	memset(p, 0, padding);

	reinterpret_cast <vmime_uint32*>(m_block)[14] = (m_byteCount << 3);
	reinterpret_cast <vmime_uint32*>(m_block)[15] = (m_byteCount >> 29);

#if VMIME_BYTE_ORDER_BIG_ENDIAN
	swapUint32Array((vmime_uint32*) m_block, (64 - 8) / 4);
#endif

	transform();

#if VMIME_BYTE_ORDER_BIG_ENDIAN
	swapUint32Array((vmime_uint32*) m_hash, 4);
#endif

	m_finalized = true;
}


void md5MessageDigest::transformHelper()
{
#if VMIME_BYTE_ORDER_BIG_ENDIAN
	swapUint32Array((vmime_uint32*) m_block, 64 / 4);
#endif
	transform();
}


void md5MessageDigest::transform()
{
	const vmime_uint32* const in = reinterpret_cast <vmime_uint32*>(m_block);

	vmime_uint32 a = m_hash[0];
	vmime_uint32 b = m_hash[1];
	vmime_uint32 c = m_hash[2];
	vmime_uint32 d = m_hash[3];

#define F1(x, y, z)	(z ^ (x & (y ^ z)))
#define F2(x, y, z)	F1(z, x, y)
#define F3(x, y, z)	(x ^ y ^ z)
#define F4(x, y, z)	(y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, in, s) \
	(w += f(x, y, z) + in, w = (w<<s | w>>(32-s)) + x)

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	m_hash[0] += a;
	m_hash[1] += b;
	m_hash[2] += c;
	m_hash[3] += d;
}


int md5MessageDigest::getDigestLength() const
{
	return 16;
}


const byte_t* md5MessageDigest::getDigest() const
{
	return reinterpret_cast <const byte_t*>(m_hash);
}


} // md5
} // digest
} // security
} // vmime

