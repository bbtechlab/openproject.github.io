/*
============================================================================
Filename     : bbliblist.h
Version      : 1.0
Created      : 15/04/2018 23:17:55
Revision     : none
Compiler     : gcc

Author       : Bamboo Do, dovanquyen.vn@gmail.com
Copyright (c) 2018,  All rights reserved.

Description  :
============================================================================
*/

#ifndef _BBLIBLIST_H_
#define _BBLIBLIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************/
/**************************** Header Files**********************************/
/***************************************************************************/
/* Start Including Header Files */
#include <stdio.h>
#include <assert.h>
/* End Including Headers */

/***************************************************************************/
/************************ Extern variables *********************************/
/***************************************************************************/
/* Start Extern variable */
/* End Extern variable */


/***************************************************************************/
/************************ Macro Definition *********************************/
/***************************************************************************/
/* Start Macro definition */

/* End Macro definition */

/***************************************************************************/
/****************************** typedef ************************************/
/***************************************************************************/
/* Start typedef */

/* End typedef */

/***************************************************************************/
/******************** global function prototype ****************************/
/***************************************************************************/
/* Start global function prototypes */
/***************************************************************************
  These are implemented at ...
 ***************************************************************************/

/* @Description:
 *  Declares ebtrys for the list inside the user structure, for the element
 *
 * @params:
 *  - type: the existing user data type for the element of the list
 *
 * @example:
 *	struct element {
 *		BBLIST_ENTRY(element) link;
 *		char buffer[256];
 * 	}
 */
#define BBLIST_ENTRY(type) struct {struct type *l_next, *l_prev; const void *l_head;}

/* @Description:
 *  Create a new data type for the list head, this type used to create
 *  variable for the list head. The user should create new the list head data
 *  type for every different element datatype.
 *
 * @params:
 *  - name: name for the new list data type, this can be any unique structure name
 *  - type: existing user data type used for the element of the list
 *
 * @example:
 *	struct element {
 *		BBLIST_ENTRY(element) link;
 *		char buffer[256];
 * 	};
 *
 *	BBLIST_HEAD(elementlist_type, element) listElement;
 */
#define BBLIST_HEAD(name, type) struct name { struct type *l_first;}

/* @Description:
 *  Initializes the head of the list. The head shall be initialized before
 *  list can be used. This macro used for dynamic initialization
 *
 * @params
 *  - head: pointer to the list head
 *
 * @example
 *  struct element {
 *      BBLIST_ENTRY(element) link;
 *      char buffer[256];
 *  };
 *
 *  BBLIST_HEAD(elementlist_type, element) listElement;
 *  BBLIST_INIT(&listElement);
 *
 */
 #define BBLIST_INIT(head) ((head)->l_first=NULL)

/* @Description:
 *  Tests if list is empty.
 *
 * @params:
 *  - head: pointer to the list head
 *
 * @return:
 *	- true: list is empty
 *	- false: list is not empty
 *
 * @example
 *  struct element {
 *      BBLIST_ENTRY(element) link;
 *      char buffer[256];
 *  };
 *
 *  BBLIST_HEAD(elementlist_type, element) listElement;
 *  BBLIST_INIT(&listElement);
 *	if BBLIST_EMPTY(&listElement) {return ; }
 *
 */
 #define BBLIST_EMPTY(head) ((head)->l_first==NULL)

/* @Description:
 *  Returns pointer to the first element of the list
 *
 * @params:
 *  - head: pointer to the list head
 *
 * @return:
 *	pointer to the first element of the list.
 *
 * @example
 *  struct element {
 *      BBLIST_ENTRY(element) link;
 *      char buffer[256];
 *  };
 *
 *  BBLIST_HEAD(elementlist_type, element) listElement;
 *  BBLIST_INIT(&listElement);
 *	struct element *first=BBLIST_FIRST(&listElement);
 *
 */
 #define BBLIST_FIRST(head) ((head)->l_first)

/* @Descripton:
 *  Returns pointer to the next element of the list
 *
 * @params:
 *  - head: pointer to the list head
 *  - field: name of the elements link field
 *
 * @return:
 *	pointer to the next element of the list.
 *
 * @example
 *	struct element {
 *		BBLIST_ENTRY(element) link;
 *		char buffer[256];
 * 	};
 *  BBLIST_HEAD(elementlist_type, element) listElement;
 *  struct element *first=BBLIST_FIRST(&listElement);
 *	struct element *next=BBLIST_NEXT(first, link);
 *
 */
 #define BBLIST_NEXT(elm, field) ((elm)->field.l_next)

/* @Description:
 *  Returns pointer to the previuos element of the list
 *
 * @params:
 *  - head: pointer to the list head
 *  - field: name of the elements link field
 *
 * @return:
 *  pointer to the next element of the list.
 *
 * @example
 *  struct element {
 *      BBLIST_ENTRY(element) link;
 *      char buffer[256];
 *  };
 *  BBLIST_HEAD(elementlist_type, element) listElement;
 *  struct element *first=BBLIST_FIRST(&listElement);
 *  struct element *prev=BBLIST_PREV(first, link);
 *
 */
#define BBLIST_PREV(elm, field) ((elm)->field.l_prev)

/* @Description:
 *  Inserts new element into the head of the list
 *
 * @params
 *  - head: pointer to the list head
 *  - new_elm: pointer to the the new element
 *  - field: name of the elements link field
 *
 * @return:
 *  <none>
 *
 * @example
 *  HLIST_INSERT_HEAD(&listElement, new_element, link);
 *
 */
#define BBLIST_INSERT_HEAD(head, new_elm, field) do { \
    (new_elm)->field.l_head = (const void *)head; \
    if ( ((new_elm)->field.l_next = (head)->l_first) != NULL ) (head)->l_first->field.l_prev = (new_elm); \
    (head)->l_first = (new_elm); (new_elm)->field.l_prev = NULL; \
    }  while(0)

/* @Description:
 *  Inserts new element after existing element.
 *
 * @params
 *  - head: pointer to the list head
 *  - elm: pointer to the element of the list head
 *  - new_elm: pointer to the the new element
 *  - field: name of the elements link field
 *
 * @return:
 *  <none>
 *
 * @example
 *  BBLIST_INSERT_AFTER(&listElement, element, new_element, link);
 *
 */
#define BBLIST_INSERT_AFTER(head, elm, new_elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (new_elm)->field.l_head = (const void *)head; \
    (new_elm)->field.l_prev = (elm); \
    if (((new_elm)->field.l_next = elm->field.l_next)!=NULL)  elm->field.l_next->field.l_prev = new_elm; \
    (elm)->field.l_next = (new_elm); \
    } while(0)

/* @Description:
 *  Inserts new element before existing element.
 *
 * @params
 *  - head: pointer to the list head
 *  - elm: pointer to the element of the list head
 *  - new_elm: pointer to the the new element
 *  - field: name of the elements link field
 *
 * @return:
 *  <none>
 *
 * @example
 *  BBLIST_INSERT_BEFORE(&listElement, element, new_element, link);
 *
 */
#define BBLIST_INSERT_BEFORE(head, elm, new_elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (new_elm)->field.l_head = (const void *)head; \
    (new_elm)->field.l_next = (elm); \
    if (((new_elm)->field.l_prev = (elm)->field.l_prev)!=NULL) elm->field.l_prev->field.l_next = new_elm; else (head)->l_first = (new_elm); \
    (elm)->field.l_prev = (new_elm); \
    } while(0)

/* @Description:
 *  Removes element from the list.
 *
 * @params
 *  - head: pointer to the list head
 *  - elm: pointer to the element of the list head
 *  - field: name of the elements link field
 *
 * @return:
 *  <none>
 *
 * @example
 *  BBLIST_REMOVE(&listElement, element, link);
 *
 */
#define BBLIST_REMOVE(head, elm, field) do { \
    assert((elm)->field.l_head == (const void *)head); \
    (elm)->field.l_head = NULL; \
    if ((elm)->field.l_next) (elm)->field.l_next->field.l_prev = (elm)->field.l_prev;  \
    if ((elm)->field.l_prev) (elm)->field.l_prev->field.l_next = (elm)->field.l_next; else (head)->l_first = (elm)->field.l_next; \
    } while(0)

/* @Description:
 *  Removes elements from the head of the list.
 *
 * @params
 *  - head: pointer to the list head
 *  - field: name of the elements link field
 *
 * @return:
 *  <none>
 *
 * @example
 *  BBLIST_REMOVE_HEAD(&listElement, link);
 *
 */
#define BBLIST_REMOVE_HEAD(head, field) do { \
    assert((head)->l_first); \
    assert((head)->l_first->field.l_head == (const void *)head); \
    (head)->l_first->field.l_head = NULL; \
    (head)->l_first = (head)->l_first->field.l_next; \
    if ((head)->l_first) { (head)->l_first->field.l_prev = NULL;} \
    } while(0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* End global function prototypes */
#endif /* _BBLIBLIST_H_ */

