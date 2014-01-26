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
PURPOSE: Tests and debug macros
*/
#ifndef ZB_DEBUG_H
#define ZB_DEBUG_H 1

/*! \addtogroup ZB_DEBUG */
/*! @{ */


/**
   Trace current location, abort current program execution, with core dump if possible.

   @param caller_file - source file name
   @param caller_line - line in the source
 */
void zb_abort(char *caller_file, int caller_line);

#if defined DEBUG || defined USE_ASSERT


#ifdef SDCC
/*
  SDCC warns about unreachable code in the constructions like ZB_ASSERT(2 > 1)
  which is possible when called from another macros like ZB_BUF_INITIAL_ALLOC.
  Switch it off.
*/
#pragma disable_warning 126
#endif


/**
   Assert: print diagnostic and force core dump

   @param expr - expression to check.
*/
#if 0
zb_void_t zb_assert(zb_char_t *file_name, zb_int_t line_number, zb_char_t *expr_name);
#define ZB_ASSERT(expr) {if(!(expr)) zb_assert(__FILE__, __LINE__, #expr);}
#else
zb_void_t zb_assert(zb_char_t *file_name, zb_int_t line_number/*, zb_char_t *expr_name*/);
#define ZB_ASSERT(expr) {if(!(expr)) zb_assert(__FILE__, __LINE__);}
#endif

#define ZB_INLINE_ASSERT_SIMPLE(expr)  ((expr) ? 1 : (zb_abort(__FILE__, __LINE__), 1))

/**
   Assert which can be used inside expression.

   @param expr - expression to check.
   @return always 1 (true)
 */
#define ZB_INLINE_ASSERT(expr)  ZB_INLINE_ASSERT_SIMPLE((expr)),

#else  /* release */

#ifndef KEIL
#define ZB_ASSERT(expr) ((void)0)
#else
#define ZB_ASSERT(expr)
#endif

#define ZB_INLINE_ASSERT_SIMPLE(expr) -1
#define ZB_INLINE_ASSERT(expr)

#endif


/**
   \par Compile-time asserts
 */

/* Tricks to force preprocessor to substitute __LINE__ in macros */
#define ZB_ASSERT_CAT_(a, b) a##b
#define ZB_ASSERT_CAT(a, b) ZB_ASSERT_CAT_(a, b)

/**
   Check condition at compile time in the code (not declaration block)

   @param expr - expression to check.
 */
#if defined SDCC || defined KEIL
#define ZB_ASSERT_COMPILE_TIME(expr)
#else
#define ZB_ASSERT_COMPILE_TIME(expr) ((void)(zb_int_t (*)[(expr) ? 1 : -1])0)
#endif
/**
   Check condition at compile time in the code (not declaration block), return 0
   To be used inside expressions.

   @param expr - expression to check.
 */
#define ZB_ASSERT_COMPILE_TIME_EXPR(expr) ((zb_int_t)((zb_int_t (*)[(expr) ? 1 : -1])0))

/**
   Check condition at compile time in the declaration block

   @param expr - expression to check.
 */
#define ZB_ASSERT_COMPILE_DECL(expr) typedef zb_int_t ZB_ASSERT_CAT(assert, __LINE__)[(expr) ? 1 : -1]


/**
   Ensures, that size of type `type' is not greater than `limit'.  If it is not,
   compilation is aborted.

   @param type - type name
   @param limit - size limit
 */
#define ZB_ASSERT_TYPE_SIZE_NOT_GREATER(type, limit) ZB_ASSERT_COMPILE_DECL(sizeof(type) <= (limit))

/**
   Ensures, that size of type `type' is not less than `limit'.  If it is not,
   compilation is aborted.

   @param type - type name
   @param limit - size limit
 */
#define ZB_ASSERT_TYPE_SIZE_NOT_LESS(type, limit) ZB_ASSERT_COMPILE_DECL(sizeof(type) >= (limit))

/**
   Ensures, that size of type `type' is equal to the `limit'.  If it is not,
   compilation is aborted.

   @param type - type name
   @param limit - size limit
 */
#define ZB_ASSERT_TYPE_SIZE_EQ(type, limit) ZB_ASSERT_COMPILE_DECL(sizeof(type) == (limit))


#ifdef ZB_ARRAYS_CHECK
/**
   Address all array elements. Use this macro to test that array really inited
   by valgring in Linux
 */
#define ZB_CHK_ARR(arr, len)                       \
  do                                               \
  {                                                \
  zb_ushort_t _ii;                                 \
  zb_ushort_t n = 0;                               \
  for (_ii = 0 ; _ii < (len) ; ++_ii)              \
  {                                                \
    if ((arr)[_ii] < 2)                            \
    {                                              \
      n++;                                         \
    }                                              \
    else                                           \
    {                                              \
      n--;                                         \
    }                                              \
  }                                                \
  }                                                \
while (0)

#else
#define ZB_CHK_ARR(arr, len)
#endif  /* ZB_ARRAYS_CHECK */


#if !defined ZB_NS_BUILD && defined ZB8051 && defined C8051F120
#define ZB_P3_ON() LED1 = 0
#define ZB_P3_OFF() LED1 = 1
#define ZB_P4_ON() LED2 = 0
#define ZB_P4_OFF() LED2 = 1
#else
#define ZB_P3_ON()
#define ZB_P3_OFF()
#define ZB_P4_ON() 
#define ZB_P4_OFF()
#endif

void dump_traf(zb_uint8_t *buf, zb_ushort_t len);


/*! @} */

#endif /* ZB_DEBUG_H */
