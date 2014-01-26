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
 PURPOSE: Error codes
*/

#ifndef ZB_ERRORS_H
#define ZB_ERRORS_H

/*! \cond internals_doc */
/**
   \addtogroup ZB_BASE
   @{
*/


/* categories */

#define ERROR_CATEGORY_INTERVAL 10000

enum zb_error_category_e
{
  ERROR_CATEGORY_GENERIC,
  ERROR_CATEGORY_SYSTEM
};

/* Let's always return 0 for RET_OK - in any category. */
#define ERROR_CODE(category, code) ((code) ? -(zb_int_t)(((category) * ERROR_CATEGORY_INTERVAL) + (code)) : 0)
#define GENERIC_ERROR_CODE(code)   ERROR_CODE(ERROR_CATEGORY_GENERIC, code)
#define SYSTEM_ERROR_CODE(code)    ERROR_CODE(ERROR_CATEGORY_SYSTEM, code)

#define ERROR_GET_CATEGORY(err) (((-(zb_int_t)(err))) / ERROR_CATEGORY_INTERVAL)
#define ERROR_GET_CODE(err) ((-(zb_int_t)(err)) % ERROR_CATEGORY_INTERVAL)

/*
   functions return type.
   In general, function can return OK, BLOCKED or some error. Errors
   are negative.
   Error can be "generic" or some additional error code.
*/
enum zb_ret_e
{
  /* Most common return types: ok, generic error, BLOCKED, thread exit indication. */
  RET_OK              = 0,
  RET_ERROR           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 1), /* -1 indeed */
  RET_BLOCKED         = ERROR_CODE(ERROR_CATEGORY_GENERIC, 2),
  RET_EXIT            = ERROR_CODE(ERROR_CATEGORY_GENERIC, 3),
  RET_BUSY            = ERROR_CODE(ERROR_CATEGORY_GENERIC, 4),
  RET_EOF             = ERROR_CODE(ERROR_CATEGORY_GENERIC, 5),
  RET_OUT_OF_RANGE    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 6),
  RET_EMPTY           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 7),
  RET_CANCELLED       = ERROR_CODE(ERROR_CATEGORY_GENERIC, 8),

  RET_INVALID_PARAMETER_1                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 10),
  RET_INVALID_PARAMETER_2                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 11),
  RET_INVALID_PARAMETER_3                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 12),
  RET_INVALID_PARAMETER_4                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 13),
  RET_INVALID_PARAMETER_5                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 14),
  RET_INVALID_PARAMETER_6                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 15),
  RET_INVALID_PARAMETER_7                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 16),
  RET_INVALID_PARAMETER_8                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 17),
  RET_INVALID_PARAMETER_9                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 18),
  RET_INVALID_PARAMETER_10                = ERROR_CODE(ERROR_CATEGORY_GENERIC, 19),
  RET_INVALID_PARAMETER_11_OR_MORE        = ERROR_CODE(ERROR_CATEGORY_GENERIC, 20),
  RET_PENDING                             = ERROR_CODE(ERROR_CATEGORY_GENERIC, 21),
  RET_NO_MEMORY                           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 22),
  RET_INVALID_PARAMETER                   = ERROR_CODE(ERROR_CATEGORY_GENERIC, 23),
  RET_OPERATION_FAILED                    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 24),
  RET_BUFFER_TOO_SMALL                    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 25),
  RET_END_OF_LIST                         = ERROR_CODE(ERROR_CATEGORY_GENERIC, 26),
  RET_ALREADY_EXISTS                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 27),
  RET_NOT_FOUND                           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 28),
  RET_OVERFLOW                            = ERROR_CODE(ERROR_CATEGORY_GENERIC, 29),
  RET_TIMEOUT                             = ERROR_CODE(ERROR_CATEGORY_GENERIC, 30),
  RET_NOT_IMPLEMENTED                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 31),
  RET_NO_RESOURCES                        = ERROR_CODE(ERROR_CATEGORY_GENERIC, 32),
  RET_UNINITIALIZED                       = ERROR_CODE(ERROR_CATEGORY_GENERIC, 33),
  RET_NO_SERVER                           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 34),
  RET_INVALID_STATE                       = ERROR_CODE(ERROR_CATEGORY_GENERIC, 35),
  RET_DOES_NOT_EXIST                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 36),
  RET_CONNECTION_FAILED                   = ERROR_CODE(ERROR_CATEGORY_GENERIC, 37),
  RET_CONNECTION_LOST                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 38),
  RET_CANCELLED_BY_USER                   = ERROR_CODE(ERROR_CATEGORY_GENERIC, 39),
  RET_UNAUTHORIZED                        = ERROR_CODE(ERROR_CATEGORY_GENERIC, 40),
  RET_CONFLICT                            = ERROR_CODE(ERROR_CATEGORY_GENERIC, 41),
  RET_COULD_NOT_OPEN_FILE                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 41),
  RET_NO_MATCH                            = ERROR_CODE(ERROR_CATEGORY_GENERIC, 43),
  RET_PROTOCOL_ERROR                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 44),
  RET_VERSION                             = ERROR_CODE(ERROR_CATEGORY_GENERIC, 45),
  RET_MALFORMED_ADDRESS                   = ERROR_CODE(ERROR_CATEGORY_GENERIC, 46),
  RET_COULD_NOT_READ_FILE                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 47),
  RET_FILE_NOT_FOUND                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 48),
  RET_DIRECTORY_NOT_FOUND                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 49),
  RET_CONVERTION_ERROR                    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 50),
  RET_INCOMPATIBLE_TYPES                  = ERROR_CODE(ERROR_CATEGORY_GENERIC, 51),
  RET_INCOMPATIBLE_TYPES_IN_COMPARE       = ERROR_CODE(ERROR_CATEGORY_GENERIC, 52),
  RET_INCOMPATIBLE_TYPES_IN_ASSIGNMENT    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 53),
  RET_INCOMPATIBLE_TYPES_IN_EXPRESSION    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 54),
  RET_ILLEGAL_COMPARE_OPERATION           = ERROR_CODE(ERROR_CATEGORY_GENERIC, 55),
  RET_FILE_CORRUPTED                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 56),
  RET_PAGE_NOT_FOUND                      = ERROR_CODE(ERROR_CATEGORY_GENERIC, 57),
  RET_FILE_WRITE_ERROR                    = ERROR_CODE(ERROR_CATEGORY_GENERIC, 58),
  RET_FILE_READ_ERROR                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 59),
  RET_FILE_PARTIAL_WRITE                  = ERROR_CODE(ERROR_CATEGORY_GENERIC, 60),
  RET_TOO_MANY_OPEN_FILES                 = ERROR_CODE(ERROR_CATEGORY_GENERIC, 61),
  RET_ILLEGAL_REQUEST                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 62),
  RET_INVALID_BINDING                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 63),
  RET_INVALID_GROUP                       = ERROR_CODE(ERROR_CATEGORY_GENERIC, 64),
  RET_TABLE_FULL                          = ERROR_CODE(ERROR_CATEGORY_GENERIC, 65),
  RET_NO_ACK                              = ERROR_CODE(ERROR_CATEGORY_GENERIC, 66),
  RET_ACK_OK                              = ERROR_CODE(ERROR_CATEGORY_GENERIC, 67),
  RET_NO_BOUND_DEVICE                     = ERROR_CODE(ERROR_CATEGORY_GENERIC, 68)
};

typedef zb_int_t zb_ret_t;

#define ES_GOTO_ON_ERR(ret, error_label) do { if ((ret) != RET_OK) goto error_label; } while (0)
#define ES_GOTO_ON_BLK(ret, error_label) do { if ((ret) == RET_BLOCKED) goto error_label; } while (0)

/*! @} */
/*! \endcond */

#endif /* ZB_ERRORS_H */
