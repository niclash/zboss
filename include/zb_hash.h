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
PURPOSE: Macros for hash calculation
*/

#ifndef ZB_HASH_H
#define ZB_HASH_H 1

/*! \cond internals_doc */
/*! \addtogroup ZB_BASE */
/*! @{ */

#define ZB_HASH_MAGIC_VAL 5381u

/* TODO: check is this hash optimal for ints */
#define ZB_HASH_FUNC_STEP(hash_var, v) ((((hash_var) << 5) + (hash_var)) + (v))
#define ZB_4INT_HASH_FUNC(v1, v2, v3, v4) (ZB_HASH_FUNC_STEP(ZB_HASH_FUNC_STEP( ZB_HASH_FUNC_STEP( ZB_HASH_FUNC_STEP(ZB_HASH_MAGIC_VAL, (v1)), (v2)), (v3)), (v4)) & ZB_INT_MASK)
#define ZB_2INT_HASH_FUNC(v1, v2)      (ZB_HASH_FUNC_STEP(ZB_HASH_FUNC_STEP(ZB_HASH_MAGIC_VAL, (v1)), (v2)) & ZB_INT_MASK)

#define ZB_1INT_HASH_FUNC(v1)          (ZB_HASH_FUNC_STEP(ZB_HASH_MAGIC_VAL, (v1)) & ZB_INT_MASK)


/*! @} */
/*! \endcond */

#endif /* ZB_HASH_H */
