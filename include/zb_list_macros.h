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
PURPOSE: Very simple lists operations (macroses).
*/
#ifndef LIST_H
#define LIST_H 1

/*! \cond internals_doc */
/**
   @addtogroup ZB_BASE
   @{
*/


/**
  \par Double linked list

  Usage sample:
  - declare some structure, include list field into it:
   struct xxx {
    ...
    ZB_LIST_FIELD(struct xxx *, next1);
   };
  - declare list head in some other place:
  struct xxx * list_head;

  - insert into the list
   ZB_LIST_INSERT_HEAD(list_head, next1, ent)
   ZB_LIST_INSERT_TAIL(list_head, next1, ent)
   ZB_LIST_INSERT_AFTER(list_head, next1, old_ent, new_ent)

  - iterate (for loop):
   ZB_LIST_ITERATE(list_head, next1, ent)
   {
     do something with ent
   }

  - iterate back (for loop)
   ZB_LIST_ITERATE_BACK(list_head, next1, ent)
   {
     do something with ent
   }

  - iterate (manual)
  struct xxx *ent = ZB_LIST_GET_HEAD(list_head, next1)
  while (ent)
  {
    ...
    ent = ZB_LIST_NEXT(ent, next1);
  }

  - iterate back (manual)
  struct xxx *ent = ZB_LIST_GET_TAIL(list_head, next1)
  while (ent)
  {
    ...
    ent = ZB_LIST_PREV_FOR_ITER(ent, next1);
  }

  - remove
  ZB_LIST_REMOVE_HEAD(list_head, next1)
  ZB_LIST_REMOVE_TAIL(list_head, next1)
  ZB_LIST_REMOVE(list_head, next1, ent)

  - check that item is already in list
  ZB_LIST_ITEM_IN_LIST(ent, next1)
 */


/**
 * List control fields declaratuion. To be put into the structure
 * definition.
 *
 * _prev field has special meaning for the list head: this is pointer
 * to the list tail.
 */
#define ZB_LIST_FIELD(type, name) type name ## _next; type name ## _prev

#if defined DEBUG && defined __GNUC__
#define ZB_LIST_CHECK_ENT(list, name, ent) do { \
 {                                              \
   typeof(ent) tmp; /* FIXME: */                \
   ZB_LIST_ITERATE(list, name, tmp)             \
   {                                            \
     ZB_ASSERT(tmp != ent);                     \
   }                                            \
 }                                              \
} while(0)
#else
#define ZB_LIST_CHECK_ENT(list, name, ent) do{}while(0)
#endif

#define ZB_LIST_SIZE(type)  (2 * sizeof(type))

#define ZB_LIST_DEFINE(type, list) type list
#define ZB_LIST_ARRAY_DEFINE(type, list, n) type list[n]
#define ZB_LIST_INIT(list) (list) = NULL


#define ZB_LIST_IS_EMPTY(list) ((list) == NULL)
#define ZB_LIST_IS_NOT_EMPTY(list) ((list) != NULL)
#define ZB_LIST_NEXT(ent, name) (ent)->name ## _next
#define ZB_LIST_PREV(ent, name) (ent)->name ## _prev
#define ZB_LIST_ZERO(ent, name) (ent)->name ## _prev = (ent)->name ## _next = 0

/**
 * Like ZB_LIST_PREV(), but check for going over list head
 */
#define ZB_LIST_PREV_FOR_ITER(ent, name) ((((ent)->name ## _prev)->name ## _next) ? (ent)->name ## _prev : 0)
#define ZB_LIST_GET_HEAD(list, name) (list)
#define ZB_LIST_GET_TAIL(list, name) ((list) ? (list)->name ## _prev : 0)
#define ZB_LIST_ITEM_IN_LIST(t, name) (((t)->name ## _prev) != NULL)


/*
 * Remove from the list head, and store it in ent
 */
#define ZB_LIST_CUT_HEAD(list, name, ent) do                          \
{                                                                     \
  ent = list;                                                         \
  if ((list))                                                         \
  {                                                                   \
    if (((list)->name ## _next))                                      \
    {                                                                 \
      ((list)->name ## _next)->name ## _prev = (list)->name ## _prev; \
    }                                                                 \
    (list)->name ## _prev = 0;                                        \
    (list) = (list)->name ## _next;                                   \
  }                                                                   \
} while (0)



/*
 * Insert to the list head
 */
#define ZB_LIST_INSERT_HEAD(list, name, ent) do        \
{                                                      \
  ZB_LIST_CHECK_ENT(list, name, ent);                  \
  (ent)->name ## _next = (list);                       \
  if ((list))                                          \
  {                                                    \
    (ent)->name ## _prev = (list)->name ## _prev;      \
    (list)->name ## _prev = (ent);                     \
  }                                                    \
  else                                                 \
  {                                                    \
    (ent)->name ## _prev = (ent);                      \
  }                                                    \
  (list) = (ent);                                      \
} while(0)


/*
 * Insert to the list tail
 */
#define ZB_LIST_INSERT_TAIL(list, name, ent) do     \
{                                                   \
  ZB_LIST_CHECK_ENT(list, name, ent);               \
  (ent)->name ## _next = 0;                         \
  if ((list))                                       \
  {                                                 \
    (ent)->name ## _prev = (list)->name ## _prev;   \
    ((list)->name ## _prev)->name ## _next = (ent); \
    (list)->name ## _prev = (ent);                  \
  }                                                 \
  else                                              \
  {                                                 \
    (ent)->name ## _prev = (ent);                   \
    (list) = (ent);                                 \
  }                                                 \
} while (0)


/*
 * Insert after existend entry
 */
#define ZB_LIST_INSERT_AFTER(list, name, ent, new_ent) do \
{                                                         \
  ZB_LIST_CHECK_ENT(list, name, new_ent);                 \
  if ((ent))                                              \
  {                                                       \
    if ((ent) == ZB_LIST_GET_TAIL(list, name))            \
    {                                                     \
      ZB_LIST_INSERT_TAIL(list, name, new_ent);           \
    }                                                     \
    else                                                  \
    {                                                     \
      (new_ent)->name ## _next = (ent)->name ## _next;    \
      (new_ent)->name ## _prev = (ent);                   \
      ((ent)->name ## _next)->name ## _prev = (new_ent);  \
      (ent)->name ## _next = (new_ent);                   \
    }                                                     \
  }                                                       \
} while(0)


/*
 * Remove from the list head
 */
#define ZB_LIST_REMOVE_HEAD(list, name) do                            \
{                                                                     \
  if ((list))                                                         \
  {                                                                   \
    if (((list)->name ## _next))                                      \
    {                                                                 \
      ((list)->name ## _next)->name ## _prev = (list)->name ## _prev; \
    }                                                                 \
    (list)->name ## _prev = 0;                                        \
    (list) = (list)->name ## _next;                                   \
  }                                                                   \
} while (0)

/*
 * Remove from the list tail
 */
#define ZB_LIST_REMOVE_TAIL(list, name) do                              \
{                                                                       \
  if ((list))                                                           \
  {                                                                     \
    if ((list)->name ## _prev == (list)) /* made an empty list */       \
    {                                                                   \
      (list)->name ## _prev = 0;                                        \
      (list) = 0;                                                       \
    }                                                                   \
    else                                                                \
    {                                                                   \
      (((list)->name ## _prev)->name ## _prev)->name ## _next = 0;      \
      /* This is a bit tricky: we need to assign list->prev and set     \
       * tail->prev to 0, but we can't use temporary variable because   \
       * we have no type here. Use _next as temporary variable. */      \
      ((list)->name ## _prev)->name ## _next = ((list)->name ## _prev)->name ## _prev; \
      ((list)->name ## _prev)->name ## _prev = 0;                       \
      (list)->name ## _prev = ((list)->name ## _prev)->name ## _next;   \
    }                                                                   \
  }                                                                     \
} while (0)

#define ZB_LIST_ITERATE(list, name, ent) \
for ((ent) = (list) ; (ent) != 0 ; (ent) = (ent)->name ## _next) //FIXME "!=0"

#define ZB_LIST_ITERATE_BACK(list, name, ent) \
for ((ent) = (list) ? (list)->name ## _prev : 0 ; (ent) ; (ent) = (((ent)->name ## _prev)->name ## _next) ? (ent)->name ## _prev : 0)

#define ZB_LIST_NULL_INIT 0,0

#define ZB_LIST_REMOVE(list, name, ent)                             \
do                                                                  \
{                                                                   \
  if ((ent)->name ## _prev != 0)                                    \
  {                                                                 \
    if ((ent)->name ## _next)    /* this is not a tail */           \
    {                                                               \
      ((ent)->name ## _next)->name ## _prev = (ent)->name ## _prev; \
    }                                                               \
    else                          /* this is a tail */              \
    {                                                               \
      (list)->name ## _prev = (ent)->name ## _prev;                 \
    }                                                               \
    if ((ent) == (list))          /* this is a head */              \
    {                                                               \
      (list) = (ent)->name ## _next;                                \
    }                                                               \
    else                          /* this is not a head */          \
    {                                                               \
      ((ent)->name ## _prev)->name ## _next = (ent)->name ## _next; \
    }                                                               \
    (ent)->name ## _prev = 0;                                       \
  }                                                                 \
}                                                                   \
while (0)

/**
   \par Single-linked list (stack and FIFO).

  Use same name ## _next field of the double linked list, so can coexist with it.
 */

/**
   Define single-linked list head/tail pointer

   @param type - type of the pointer to the list element
   @param list - list head/tail name prefix

   @b Example:
@code
   ZB_SL_LIST_DEFINE (struct work_file_page0_s *, f);
@endcode
 */
#define ZB_SL_LIST_DEFINE(type, list) type list ## _head; type list ## _tail

#define ZB_SL_LIST_INIT(list) list ## _head = list ## _tail = 0


/**
   Define field in the single-linked list element

   @param type - type of the pointer to the list element
   @param name - list name (to be able to put element to more then one list)

   @b Example:
@code
   ZB_SL_LIST_FIELD (struct work_file_page0_s *, f);
@endcode
 */
#define ZB_SL_LIST_FIELD(type, name) type name ## _next

/**
   Insert into single-linked list head

   @param list - list header pointer (sample: some_ptr->some_list)
   @param name - list name
 */
#define ZB_SL_LIST_INSERT_HEAD(list, name, ent) \
do                                              \
{                                               \
  (ent)->name ## _next = (list ## _head);       \
  if (!(list ## _head))                         \
  {                                             \
    (list ## _tail) = (ent);                    \
  }                                             \
  (list ## _head) = (ent);                      \
}                                               \
while (0)


#define ZB_SL_LIST_INSERT_TAIL(list, name, ent) \
do                                              \
{                                               \
  (ent)->name ## _next = 0;                     \
  if (list ## _tail)                            \
  {                                             \
    (list ## _tail)->name ## _next = (ent);     \
    list ## _tail = (ent);                      \
  }                                             \
  else                                          \
  {                                             \
    (list ## _head) = (ent);                    \
    (list ## _tail) = (ent);                    \
  }                                             \
}                                               \
while (0)



#define ZB_SL_LIST_REMOVE_HEAD(list, name)            \
do                                                    \
{                                                     \
  if ((list ## _head))                                \
  {                                                   \
    (list ## _head) = (list ## _head)->name ## _next; \
    if (!(list ## _head))                             \
    {                                                 \
      (list ## _tail) = 0;                            \
    }                                                 \
  }                                                   \
}                                                     \
while (0)

#define ZB_SL_LIST_CUT_HEAD(list, name, ent)          \
do                                                    \
{                                                     \
  if ((list ## _head))                                \
  {                                                   \
    ent = (list ## _head);                            \
    (list ## _head) = (list ## _head)->name ## _next; \
    if (!(list ## _head))                             \
    {                                                 \
      (list ## _tail) = 0;                            \
    }                                                 \
  }                                                   \
  else                                                \
  {                                                   \
    (ent) = 0;                                        \
  }                                                   \
}                                                     \
while (0)

#define ZB_SL_LIST_REMOVE(type, list, name, ent)                  \
do                                                                \
{                                                                 \
  type p;                                                         \
  type prev = 0;                                                  \
  for (p = (list ## _head) ; p ; prev = p, p = p->name ## _next)  \
  {                                                               \
    if (p == (ent))                                               \
    {                                                             \
      if (prev)                                                   \
      {                                                           \
        prev->name ## _next = p->name ## _next;                   \
        if ((list ## _tail) == ent)                               \
        {                                                         \
          (list ## _tail) = prev;                                 \
        }                                                         \
      }                                                           \
      else                                                        \
      {                                                           \
        (list ## _head) = p->name ## _next;                       \
      }                                                           \
      if (!(list ## _head))                                       \
      {                                                           \
        (list ## _tail) = 0;                                      \
      }                                                           \
      break;                                                      \
    }                                                             \
  }                                                               \
}                                                                 \
while (0)


#define ZB_SL_LIST_ITERATE(list, name, ent) for ((ent) = (list ## _head) ; (ent) ; (ent) = (ent)->name ## _next)

#define ZB_SL_LIST_NEXT(ent, name) ((ent)->name ## _next)

#define ZB_SL_LIST_GET_HEAD(list) (list ## _head)


/* FIFO compatible with double-linked list macros */

#define ZB_FIFO_DEFINE            ZB_SL_LIST_DEFINE
#define ZB_FIFO_FIELD             ZB_SL_LIST_FIELD
#define ZB_FIFO_INIT              ZB_SL_LIST_INIT
#define ZB_FIFO_PUT               ZB_SL_LIST_INSERT_TAIL
#define ZB_FIFO_PUT_TAIL          ZB_SL_LIST_INSERT_HEAD
#define ZB_FIFO_GET               ZB_SL_LIST_CUT_HEAD
#define ZB_FIFO_PEEK              ZB_SL_LIST_GET_HEAD
#define ZB_FIFO_REMOVE            ZB_SL_LIST_REMOVE_HEAD
#define ZB_FIFO_ITERATE_FROM_TAIL ZB_SL_LIST_ITERATE


/* Stack compatible with double-linked list macros */

#define ZB_STK_DEFINE(type, list)       type list ## _head

#define ZB_STK_FIELD              ZB_SL_LIST_FIELD

#define ZB_STK_PUSH(list, name, ent)            \
do                                              \
{                                               \
  (ent)->name ## _next = (list ## _head);       \
  (list ## _head) = (ent);                      \
}                                               \
while (0)

#define ZB_STK_POP(list, name, ent)                   \
do                                                    \
{                                                     \
  if ((list ## _head))                                \
  {                                                   \
    (ent) = (list ## _head);                          \
    (list ## _head) = (list ## _head)->name ## _next; \
  }                                                   \
  else                                                \
  {                                                   \
    (ent) = 0;                                        \
  }                                                   \
}                                                     \
while (0)

#define ZB_STK_PEEK               ZB_SL_LIST_GET_HEAD
#define ZB_STK_ITERATE            ZB_SL_LIST_ITERATE
#define ZB_STK_NEXT               ZB_SL_LIST_NEXT

/*! @} */
/*! \endcond */

#endif /* LIST_H */
