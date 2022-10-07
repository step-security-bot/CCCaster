/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */

#ifdef HAVE_OPENSSL

#include <openssl/md5.h>

#elif !defined( _MD5_H ) // HAVE_OPENSSL

#define _MD5_H

#ifdef __cplusplus

extern "C" {

#endif // __cplusplus

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

typedef struct {
    MD5_u32plus   lo, hi;
    MD5_u32plus   a, b, c, d;
    unsigned char buffer[ 64 ];
    MD5_u32plus   block[ 16 ];
} MD5_CTX;

extern void MD5_Init( MD5_CTX* ctx );
extern void MD5_Update( MD5_CTX* ctx, const void* data, unsigned long size );
extern void MD5_Final( unsigned char* result, MD5_CTX* ctx);

#ifdef __cplusplus

}

#endif // __cplusplus

/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F( x, y, z ) (  ( z ) ^ ( ( x ) & ( ( y ) ^ ( z ) ) ) )
#define G( x, y, z ) ( ( y ) ^ ( ( z ) & ( ( x ) ^ ( y ) ) ) )
#define H( x, y, z ) ( ( x ) ^ ( y ) ^ ( z ) )
#define I( x, y, z ) ( ( y ) ^ ( ( x ) | ~( z ) ) )

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP( f, a, b, c, d, x, t, s ) \
    ( a ) += f( ( b ), ( c ), ( d ) ) + ( x ) + ( t ); \
    ( a ) = ( ( ( a ) << ( s ) ) | ( ( ( a ) & 0xffffffff ) >> ( 32 - ( s ) ) ) ); \
    ( a ) += ( b );

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( __vax__ )

#define SET( n ) \
    ( *(MD5_u32plus*)&ptr[ ( n ) * 4 ] )

#define GET( n ) \
    SET( n )

#else // __i386__ || __x86_64__ || __vax__

#define SET( n ) \
    ( ctx->block[ ( n ) ] = \
    (MD5_u32plus)ptr[ ( n ) * 4 ] | \
    ( (MD5_u32plus)ptr[ ( n ) * 4 + 1 ] << 8 ) | \
    ( (MD5_u32plus)ptr[ ( n ) * 4 + 2 ] << 16 ) | \
    ( (MD5_u32plus)ptr[ ( n ) * 4 + 3 ] << 24 ) )

#define GET( n ) \
    ( ctx->block[ ( n ) ] )

#endif // // __i386__ || __x86_64__ || __vax__

#endif // HAVE_OPENSSL
