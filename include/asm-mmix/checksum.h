/*
 * include/asm-mmix/checksum.h
 *
 *   Copyright (C) 2008 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _ASM_MMIX_CHECKSUM_H
#define _ASM_MMIX_CHECKSUM_H
#ifdef __KERNEL__

extern unsigned short ip_fast_csum(unsigned char * iph, unsigned int ihl);

extern unsigned short int csum_tcpudp_magic(unsigned long saddr,
					   unsigned long daddr,
					   unsigned short len,
					   unsigned short proto,
					   unsigned int sum);

unsigned int csum_tcpudp_nofold(unsigned long saddr, unsigned long daddr,
				unsigned short len, unsigned short proto,
				unsigned int sum);

extern unsigned int csum_partial(const unsigned char * buff, int len,
				 unsigned int sum);

unsigned int csum_partial_copy_from_user(const unsigned char __user *src, unsigned char *dst, int len, unsigned int sum, int *errp);

unsigned int csum_partial_copy_nocheck(const unsigned char *src, unsigned char *dst, int len, unsigned int sum);

static inline unsigned short csum_fold(unsigned int sum)
{
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}
#endif /* __KERNEL__ */
#endif
