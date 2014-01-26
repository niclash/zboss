/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: general-purpose typedefs
*/
#ifndef ZB_TYPES_H
#define ZB_TYPES_H 1

/**
   @addtogroup base_types
   @{
*/

#ifdef ZB8051
#define ZB_8BIT_WORD
#else
#define ZB_32BIT_WORD
#endif

/* Really need xdata declaration here, not in osif: don't want to include osig
 * .h here */
#ifdef ZB_IAR
#define ZB_XDATA
#define ZB_CODE
#define ZB_IAR_CODE  __code
//#define ZB_IAR_CODE
#elif defined KEIL
#define ZB_XDATA xdata
#define ZB_CODE  code
#define ZB_IAR_CODE code
#else
#define ZB_XDATA
#define ZB_CODE
#define ZB_IAR_CODE code
#endif

/* register modifier for variables. Can be defined to "register". Will it help to the compiler? */
#define ZB_REGISTER
/* #define ZB_REGISTER register */

#define ZB_VOID_ARGLIST void

/* Not sure all C compilers support 'const'. Let's add some conditional
 * compilation when we will find such compiler  */
#define ZB_CONST const

/* inline directive for functions placed into .h files in C */
#if defined __GNUC__ || defined __SUNPRO_C || defined  __cplusplus
#define ZB_INLINE inline
#define ZB_INLINE_IN_C
#define ZB_BITFIELD_CAST(x) (x)
#elif defined _MSC_VER
#define ZB_INLINE __inline
#define ZB_BITFIELD_CAST(x) (zb_uint_t)(x)
#else
/* which other C compiler knows about inline? */
#define ZB_INLINE
#define ZB_BITFIELD_CAST(x) (x)
#endif


/**
   General purpose boolean type
*/
typedef enum zb_bool_e
{
  ZB_FALSE = 0,
  ZB_TRUE = 1
}
zb_bool_t;


#ifndef UNIX

/* base types */

/**
   project-local char type
*/
typedef char               zb_char_t;
/**
   project-local unsigned char type
*/
typedef unsigned char      zb_uchar_t;
/**
   project-local 1-byte unsigned int type
*/
typedef unsigned char      zb_uint8_t;
/**
   project-local 1-byte signed int type
*/
typedef signed char        zb_int8_t;
/**
   project-local 2-byte unsigned int type
*/
typedef unsigned short     zb_uint16_t;
/**
   project-local 2-byte signed int type
*/
typedef signed short       zb_int16_t;
#ifdef ZB8051
/**
   project-local 4-byte unsigned int type
*/
typedef unsigned long      zb_uint32_t;
/**
   project-local 4-byte signed int type
*/
typedef signed long        zb_int32_t;

/**
   type to be used for unsigned bit fields inside structure
*/
typedef zb_uint16_t        zb_bitfield_t;
/**
   type to be used for signed bit fields inside structure
*/
typedef zb_int16_t         zb_sbitfield_t;

/**
   project-local size_t type
*/

typedef zb_uint16_t        zb_size_t;


#else
/**
   project-local 4-byte unsigned int type
*/
typedef unsigned int       zb_uint32_t;
/**
   project-local 4-byte signed int type
*/
typedef signed int         zb_int32_t;
/**
   type to be used for unsigned bit fields inside structure
*/
typedef zb_uint32_t        zb_bitfield_t;
/**
   type to be used for signed bit fields inside structure
*/
typedef zb_int32_t         zb_sbitfield_t;


#endif  /* ZB8051 */


#else  /* ifndef UNIX */
/* Unix */

#include <inttypes.h>

typedef uint8_t            zb_uint8_t;
typedef int8_t             zb_int8_t;
typedef uint16_t           zb_uint16_t;
typedef int16_t            zb_int16_t;
typedef uint32_t           zb_uint32_t;
typedef int32_t            zb_int32_t;

typedef char               zb_char_t;
typedef unsigned char      zb_uchar_t;

typedef unsigned           zb_bitfield_t;
typedef signed             zb_sbitfield_t;

/* Integer with size equal to the pointer size. */
typedef zb_uint32_t        zb_size_t;
typedef zb_size_t          zb_ptrsize_uint_t;


#endif  /* UNIX */


/*
  Our short and int definition.
  Short size == 8051 register size, int size is 2 bytes.
 */
#ifdef ZB_8BIT_WORD

/**
    short int (can fit into single CPU register)
 */
typedef zb_int8_t          zb_short_t;
/**
    short unsigned int (can fit into single CPU register)
 */
typedef zb_uint8_t         zb_ushort_t;
/**
    int (at least 2 bytes)
 */
typedef zb_int16_t         zb_int_t;
/**
    unsigned int (at least 2 bytes)
 */
typedef zb_uint16_t        zb_uint_t;
/**
    long int (at least 4 bytes)
 */
typedef zb_int32_t         zb_long_t;
/**
    unsigned long int (at least 4 bytes)
 */
typedef zb_uint32_t        zb_ulong_t;
#else
/**
    short int (can fit into single CPU register)
 */
typedef int                zb_short_t;
/**
    unsigned short int (can fit into single CPU register)
 */
typedef unsigned int       zb_ushort_t;
/**
    int (at least 2 bytes)
 */
typedef int                zb_int_t;
/**
    unsigned int (at least 2 bytes)
 */
typedef unsigned int       zb_uint_t;
/**
    long int (at least 4 bytes)
 */
typedef zb_int_t           zb_long_t;
/**
    unsigned long int (at least 4 bytes)
 */
typedef zb_uint_t          zb_ulong_t;
#endif

/**
   ptr to void
 */
typedef void *             zb_voidp_t;
typedef void               zb_void_t;

#define ZB_INT8_MIN       (-127 - 1)
#define ZB_INT8_MAX       127
#define ZB_UINT8_MIN      0
#define ZB_UINT8_MAX      255

#define ZB_INT16_MIN       (-32767 - 1)
#define ZB_INT16_MAX       32767
#define ZB_UINT16_MIN      0
#define ZB_UINT16_MAX      65535

/* 2147483648 is unsigned indeed - can't change its sign. prevent warning from
 * msvc 8 */
#define ZB_INT32_MIN       (-2147483647L - 1)
#define ZB_INT32_MAX       2147483647L
#define ZB_UINT32_MIN      0UL
#define ZB_UINT32_MAX      4294967295UL

#define ZB_UINT_MIN      0UL

/*
  Short defined as register size, int as large enough int on this platform.
  32 bit on PC, 8 and 16 bit on 8051
 */


/**
  Max value constants per type
 */
#ifdef ZB_32BIT_WORD

#define ZB_SHORT_MIN       ZB_INT32_MIN
#define ZB_SHORT_MAX       ZB_INT32_MAX
#define ZB_USHORT_MAX      ZB_UINT32_MAX

#define ZB_INT_MIN       ZB_INT32_MIN
#define ZB_INT_MAX       ZB_INT32_MAX
#define ZB_UINT_MAX      ZB_UINT32_MAX

#define ZB_INT_MASK      0x7fffffff

#elif defined ZB_8BIT_WORD

#define ZB_SHORT_MIN       ZB_INT8_MIN
#define ZB_SHORT_MAX       ZB_INT8_MAX
#define ZB_USHORT_MAX      ZB_UINT8_MAX

#define ZB_INT_MIN       ZB_INT16_MIN
#define ZB_INT_MAX       ZB_INT16_MAX
#define ZB_UINT_MAX      ZB_UINT16_MAX

#define ZB_INT_MASK      0x7fff

#else
#error Portme
#endif

/**
   8-bytes address (xpanid or long device address) base type
 */
typedef zb_uint8_t zb_64bit_addr_t[8];


#ifdef SDCC
extern __xdata zb_64bit_addr_t g_zero_addr;
#else
extern zb_64bit_addr_t g_zero_addr;
#endif

/**
   Return true if long address is zero
 */
#define ZB_IS_64BIT_ADDR_ZERO(addr) (!ZB_MEMCMP((addr), g_zero_addr, 8))

/**
   Clear long address
 */
#define ZB_64BIT_ADDR_ZERO(addr)                \
  ZB_MEMSET((addr), 0, 8)


/**
   Copy long address
 */
#define ZB_64BIT_ADDR_COPY(dst, src) ZB_MEMCPY(dst, src, sizeof(zb_64bit_addr_t))

/**
   Return 1 if long addresses are equal
 */
#define ZB_64BIT_ADDR_CMP(one, two) ((zb_bool_t)!ZB_MEMCMP((one), (two), 8))

/**
  Long (64-bit) device address
 */
typedef zb_64bit_addr_t zb_ieee_addr_t;
/**
  Long (64-bit) Extented pan id
 */
typedef zb_64bit_addr_t zb_ext_pan_id_t;

#define ZB_EXTPANID_IS_ZERO ZB_IS_64BIT_ADDR_ZERO
#define ZB_EXTPANID_ZERO ZB_64BIT_ADDR_ZERO
#define ZB_EXTPANID_COPY ZB_64BIT_ADDR_COPY
#define ZB_EXTPANID_CMP ZB_64BIT_ADDR_CMP

#define ZB_IEEE_ADDR_IS_ZERO ZB_IS_64BIT_ADDR_ZERO
#define ZB_IEEE_ADDR_ZERO ZB_64BIT_ADDR_ZERO
#define ZB_IEEE_ADDR_COPY ZB_64BIT_ADDR_COPY
#define ZB_IEEE_ADDR_CMP ZB_64BIT_ADDR_CMP

#define ZB_ADDR_CMP(addr_mode, addr1, addr2)                            \
  ((addr_mode == ZB_ADDR_16BIT_DEV_OR_BROADCAST) ?                      \
   (addr1.addr_short == addr2.addr_short) : ZB_64BIT_ADDR_CMP(addr1.addr_long, addr2.addr_long))

/**
   Union to address either long or short address
 */
union zb_addr_u
{
  zb_uint16_t  addr_short;
  zb_ieee_addr_t addr_long;
};

/**
 definitions for constants of given type
*/
#define ZB_INT8_C(c)  c
#define ZB_UINT8_C(c) c ## U

#define ZB_INT16_C(c)  c
#define ZB_UINT16_C(c) c ## U

#define ZB_INT32_C(c)  c ## L
#define ZB_UINT32_C(c) c ## UL

#define ZB_OFFSETOF(t, f) (zb_size_t)(&((t *)0)->f)

#define ZB_OFFSETOF_VAR(s, f) (zb_size_t)(((zb_int8_t *)(&(s)->f)) - ((zb_int8_t *)(s)))

#define ZB_SIZEOF_FIELD(type, field) (sizeof(((type*)0)->field))

#define ZB_ARRAY_SIZE(arr) (sizeof((arr))/sizeof((arr)[0]))


#define ZB_SIGNED_SHIFT(v, s) ((zb_int_t)(v) >> (s))

#ifdef __GNUC__
  #define ZB_PACKED_STRUCT __attribute__ ((packed))
#else
  #define ZB_PACKED_STRUCT
#endif

/**
   \par macros to change words endian and access words at potentially
   non-aligned pointers.

   ZigBee uses little endian - see 1.2.1.3.
 */


#ifdef ZB_LITTLE_ENDIAN

#ifdef ZB_NEED_ALIGN
/**
   Convert 16-bits integer from the host endian to the little endian

   @param ptr - destination pointer. It is ok if it not aligned to 2.
   @param val - source pointer. It is ok if it not aligned to 2.
*/

#define ZB_HTOLE16(ptr, val)   ZB_MEMCPY((ptr), (val), 2)
#define ZB_HTOLE32(ptr, val)   ZB_MEMCPY((ptr), (val), 4)
#define ZB_HTOLE64(ptr, val)   ZB_MEMCPY((ptr), (val), 8)
#else

#define ZB_HTOLE16(ptr, val)   (((zb_uint16_t *)(ptr))[0] = *((zb_uint16_t *)(val)))
#define ZB_HTOLE32(ptr, val)   (((zb_uint32_t *)(ptr))[0] = *((zb_uint32_t *)(val)))
/*
#define ZB_HTOLE64(ptr, val)                              \
  (((zb_uint32_t *)(ptr))[0] = ((zb_uint32_t *)(val))[0], \
   ((zb_uint32_t *)(ptr))[1] = ((zb_uint32_t *)(val))[1])
*/
#define ZB_HTOLE64(ptr, val) ZB_MEMCPY(ptr, val, sizeof(zb_64bit_addr_t))
#endif  /* need_align */
#define ZB_HTOBE16(ptr, val)                            \
  (((zb_uint8_t *)(ptr))[0] = ((zb_uint8_t *)(val))[1], \
   ((zb_uint8_t *)(ptr))[1] = ((zb_uint8_t *)(val))[0]  \
  )


#define ZB_HTOBE16_VAL(ptr, val)                \
{                                               \
  zb_uint16_t _v = (val);                       \
  ZB_HTOBE16((ptr), &_v);                       \
}

#else  /* !little endian */

#define ZB_HTOLE16(ptr, val)                            \
  (((zb_uint8_t *)(ptr))[0] = ((zb_uint8_t *)(val))[1], \
   ((zb_uint8_t *)(ptr))[1] = ((zb_uint8_t *)(val))[0]  \
  )

void zb_htole32(zb_uint32_t ZB_XDATA *ptr, zb_uint32_t ZB_XDATA *val);
#define ZB_HTOLE32(ptr, val) zb_htole32((zb_uint32_t*)(ptr), (zb_uint32_t*)(val))

#define ZB_HTOBE16(ptr, val) (*(zb_uint16_t *)(ptr)) = *((zb_uint16_t *)(val))

#ifdef ZB_NEED_ALIGN

#define ZB_HTOBE16_VAL(ptr, val)                \
{                                               \
  zb_uint16_t _v = (val);                       \
  ZB_MEMCPY((ptr), &_v, 2);                     \
}

#else

#define ZB_HTOBE16_VAL(ptr, val) ((zb_uint16_t *)(ptr))[0] = (val)

#endif  /* ZB_NEED_ALIGN */

#if 0                           /* let's not rotate 64-bit address: store it as
                                 * 8-byte array in the order it transmitted */

#define ZB_HTOLE64(ptr, val)                                \
        *((zb_uint8_t*)(ptr)+7) = *((zb_uint8_t*)(val)  ),  \
        *((zb_uint8_t*)(ptr)+6) = *((zb_uint8_t*)(val)+1),  \
        *((zb_uint8_t*)(ptr)+5) = *((zb_uint8_t*)(val)+2),  \
        *((zb_uint8_t*)(ptr)+4) = *((zb_uint8_t*)(val)+3),  \
        *((zb_uint8_t*)(ptr)+3) = *((zb_uint8_t*)(val)+4),  \
        *((zb_uint8_t*)(ptr)+2) = *((zb_uint8_t*)(val)+5),  \
        *((zb_uint8_t*)(ptr)+1) = *((zb_uint8_t*)(val)+6),  \
        *((zb_uint8_t*)(ptr)  ) = *((zb_uint8_t*)(val)+7)
#endif

#define ZB_HTOLE64(ptr, val)   ZB_MEMCPY((ptr), (val), 8)

#endif


/**
   Put next 2-bute value into buffer, move pointer

   To be used for headers compose.

   @param dst - (in/out) address os the buffer pointer
          As a side effect it will be incremented by 2.
 */
void zb_put_next_htole16(zb_uint8_t **dst, zb_uint16_t val);

void zb_get_next_letoh16(zb_uint16_t *dst, zb_uint8_t **src);


#define ZB_LETOH64 ZB_HTOLE64

/**
   Convert 16-bits integer from the little endian to the host endian

   @param ptr - destination pointer. It is ok if it not aligned to 2.
   @param val - source pointer. It is ok if it not aligned to 2.
*/
#define ZB_LETOH16 ZB_HTOLE16
#define ZB_LETOH32 ZB_HTOLE32
#define ZB_BETOH16 ZB_HTOBE16


#define ZB_GET_LOW_BYTE(val) ((val) & 0xFF)
#define ZB_GET_HI_BYTE(val)  (((val) >> 8) & 0xFF)


#define ZB_PKT_16B_ZERO_BYTE 0
#define ZB_PKT_16B_FIRST_BYTE 1


/*! @} */

#endif /* ZB_TYPES_H */
