/******************************************************************************
 *
 * Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file  fmsh_common_list.h
 *
 * This file contains
 *
 * @note		None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------
 *
 *</pre>
 ******************************************************************************/
#ifndef _FMSH_COMMON_LIST_H_
#define _FMSH_COMMON_LIST_H_

#ifdef __cplusplus
extern "C"
{  // allow C++ to use these headers
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

// INLINE definition for the ARM C compiler
#define INLINE inline  //__inline

/*****************************************************************************
 * DESCRIPTION
 *  This is the structure used for managing linked lists.
 *
 *****************************************************************************/
typedef struct fmsh_list_head {
    struct fmsh_list_head *next, *prev;
} FMSH_listHead;

#define FMSH_LIST_HEAD_INIT(name) {&(name), &(name)}

#define FMSH_LIST_HEAD(name)      FMSH_listHead name = FMSH_LIST_HEAD_INIT(name)

#define FMSH_INIT_LIST_HEAD(ptr) \
    do                           \
    {                            \
        (ptr)->next = (ptr);     \
        (ptr)->prev = (ptr);     \
    } while (0)

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************
 * DESCRIPTION
 *  Insert a new entry between two known consecutive entries.
 *  This is only for internal list manipulation where we know
 *  the prev/next entries already!
 * ARGUMENTS
 *  new         element to insert
 *  prev        previous entry
 *  next        next entry
 *
 *****************************************************************************/
static INLINE void __FMSH_listAdd (FMSH_listHead *new, FMSH_listHead *prev,
                                   FMSH_listHead *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/*****************************************************************************
 * DESCRIPTION
 *  Insert a new entry after the specified head.
 *  This is good for implementing stacks.
 * ARGUMENTS
 *  new         new entry to be added
 *  head        list head to add it after
 *
 *****************************************************************************/
static INLINE void FMSH_listAdd (FMSH_listHead *new, FMSH_listHead *head)
{
    __FMSH_listAdd(new, head, head->next);
}

/*****************************************************************************
 * DESCRIPTION
 *  Insert a new entry before the specified head.
 *  This is useful for implementing queues.
 * ARGUMENTS
 *  new         new entry to be added
 *  head        list head to add it before
 *
 *****************************************************************************/
static INLINE void FMSH_listAddTail (FMSH_listHead *new, FMSH_listHead *head)
{
    __FMSH_listAdd(new, head->prev, head);
}

/*****************************************************************************
 * DESCRIPTION
 *  Delete a list entry by making the prev/next entries point to each
 *  other.  This is only for internal list manipulation where we know
 *  the prev/next entries already!
 * ARGUMENTS
 *  prev        previous entry
 *  next        next entry
 *
 *****************************************************************************/
static INLINE void __FMSH_listDel (FMSH_listHead *prev, FMSH_listHead *next)
{
    next->prev = prev;
    prev->next = next;
}

/*****************************************************************************
 * DESCRIPTION
 *  Deletes entry from list.
 * ARGUMENTS
 *  entry       the element to delete from the list
 * NOTES
 *  list_empty on entry does not return TRUE after this, the entry
 *  is in an undefined state.
 *
 *****************************************************************************/
static INLINE void FMSH_listDel (FMSH_listHead *entry)
{
    __FMSH_listDel(entry->prev, entry->next);
}

/*****************************************************************************
 * DESCRIPTION
 *  Deletes entry from list and reinitializes it.
 * ARGUMENTS
 *  entry       the element to delete from the list
 *
 *****************************************************************************/
static INLINE void FMSH_listDelInit (FMSH_listHead *entry)
{
    __FMSH_listDel(entry->prev, entry->next);
    FMSH_INIT_LIST_HEAD(entry);
}

/*****************************************************************************
 * DESCRIPTION
 *  Tests whether a list is empty.
 * ARGUMENTS
 *  head        the list to test
 *
 *****************************************************************************/
static INLINE int FMSH_listEmpty (FMSH_listHead *head)
{
    return head->next == head;
}

/*****************************************************************************
 * DESCRIPTION
 *  Join two lists.
 * ARGUMENTS
 *  list        the new list to add
 *  head        the place to add it in the first list
 *
 *****************************************************************************/
static INLINE void FMSH_listSplice (FMSH_listHead *list, FMSH_listHead *head)
{
    FMSH_listHead *first = list->next;

    if (first != list)
    {
        FMSH_listHead *last = list->prev;
        FMSH_listHead *at = head->next;

        first->prev = head;
        head->next = first;

        last->next = at;
        at->prev = last;
    }
}

/*****************************************************************************
 * DESCRIPTION
 *  Get the struct for this entry.
 * ARGUMENTS
 *  ptr         the &FMSH_listHead pointer
 *  type        the type of the struct this is embedded in
 *  member      the name of the list_struct within the struct
 *
 *****************************************************************************/
#define FMSH_LIST_ENTRY(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/*****************************************************************************
 * DESCRIPTION
 *  Iterate over a list.
 * ARGUMENTS
 *  pos         the &FMSH_listHead to use as a loop counter
 *  head        the head for your list
 *
 *****************************************************************************/
#define FMSH_LIST_FOR_EACH(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/*****************************************************************************
 * DESCRIPTION
 *  list_for_each_safe(pos, head)
 * DESCRIPTION
 *  Iterate over a list safe against removal of list entry.
 * ARGUMENTS
 *  pos         the &FMSH_listHead to use as a loop counter
 *  n           another &FMSH_listHead to use as temporary storage
 *  head        the head for your list
 *
 *****************************************************************************/
#define FMSH_LIST_FOR_EACH_SAFE(pos, n, head)              \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

/*****************************************************************************
 * DESCRIPTION
 *  Iterate over a list in reverse order.
 * ARGUMENTS
 *  pos         the &FMSH_listHead to use as a loop counter
 *  head        the head for your list
 *
 *****************************************************************************/
#define FMSH_LIST_FOR_EACH_PREV(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FMSH_COMMON_LIST_H_ */
