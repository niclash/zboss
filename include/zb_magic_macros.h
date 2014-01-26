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
PURPOSE: Some macros for fast calculation

*/
#ifndef MAGIC_MACROSES_H
#define MAGIC_MACROSES_H 1

/*! \cond internals_doc */
/**
   @addtogroup ZB_BASE
   @{
*/

#define MAGIC_ROUND_A_TO_B(a, b) (((a) + ((b)-1)) & -((zb_int_t)(b)))

#define MAGIC_ROUND_TO_32(a) (((a) + ((32)-1)) & -(32))

#define MAGIC_ROUND_TO_4(a) (((a) + ((4)-1)) & -(4))

#define MAGIC_ROUND_TO_INT_SIZE(a) (((a) + (sizeof(zb_int_t)-1)) & -((zb_int_t)sizeof(zb_int_t)))

#define MAGIC_TRUNC_TO_4(a) ((a) & (~3))

#define MAGIC_LEAST_SIGNIFICANT_BIT_MASK(x) ((x) & (-((zb_int_t)(x))))

#define MAGIC_LS_ZERO_MASK(x) ((~(x)) & ((x) + 1))

#define MAGIC_TURN_OFF_LS_1(x) ((x) & ((x) - 1))

/**
 * Turn off (set to 0) rightmost chain of continuous 1s
 */
#define MAGIC_TURN_OFF_RIGHT_ONES(x)  ((((x) | ((x) - 1)) + 1) & (x))


#define MAGIC_ISOLATE_RIGHT_ONES(x)  ((x) ^ MAGIC_TURN_OFF_RIGHT_ONES(x))

/**
 * Branch-free calculation of x = (x == a ? b : a)
 */
#define MAGIC_ALTERNATE(x, a, b) ((a) ^ (b) ^ (x))


#define MAGIC_AVG(a, b) ((a) & (b) + ((a) ^ (b))/2)

/**
   return -1, 0, 1 if x < y, x == y, x > y
  */
#define MAGIC_CMP(x, y) (((x) > (y)) - ((x) < (y)))

/**
   Return signum of the result: -1 if < 0, 0 if == 0, else 1
 */
#define MAGIC_SIGN(x) (((x) > 0) - ((x) < 0))

/**
   Return 1 if the number is a power of 2, works only if x > 0
 */
#define MAGIC_IS_POWER_OF_TWO(x) ( ((x) & ((x) - 1)) == 0 )

/*! @} */
/*! \endcond */

#endif /* MAGIC_MACROSES_H */
