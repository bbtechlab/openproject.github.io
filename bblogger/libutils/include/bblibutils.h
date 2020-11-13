/*
============================================================================
Filename     : bblibutils.h
Version      : 1.0
Created      : 15/04/2018 22:45:12
Revision     : none
Compiler     : gcc

Author       : Bamboo Do, dovanquyen.vn@gmail.com
Copyright (c) 2018,  All rights reserved.

Description  :
============================================================================
*/

#ifndef _BBLIBUTILS_H_
#define _BBLIBUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/***************************************************************************/
/**************************** Header Files**********************************/
/***************************************************************************/
/* Start Including Header Files */
#include "bbliblist.h"
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
typedef struct bbutils_stack {
	int 			*data;
	unsigned int	top;
	pthread_mutex_t	lock;
	unsigned int	maxNumber;
} bbutils_stack_t;

typedef struct bbutils_que {
	unsigned int	curRd;
	unsigned int	curWr;
	unsigned int	countNumber;
	unsigned int	maxNumber;
	int				*data;
	pthread_mutex_t	lock;
	pthread_cond_t	condRd;
	pthread_cond_t	condWr;
} bbutils_que_t;

/* End typedef */

/***************************************************************************/
/******************** global function prototype ****************************/
/***************************************************************************/
/* Start global function prototypes */
/***************************************************************************
  These are implemented at bblibutils_stack.c
 ***************************************************************************/
int  BBUTILS_stackCreate(bbutils_stack_t *pStack, unsigned int maxNumber);
void BBUTILS_stackDelete(bbutils_stack_t *pStack);
int  BBUTILS_stackPush(bbutils_stack_t *pStack, int data);
int  BBUTILS_stackPop(bbutils_stack_t *pStack, int *data);
bool BBUTILS_stackIsFull(bbutils_stack_t *pStack);
bool BBUTILS_stackIsEmpty(bbutils_stack_t *pStack);
void BBUTILS_stackDump(bbutils_stack_t *pStack);

/***************************************************************************
  These are implemented at bblibutils_que.c
 ***************************************************************************/
int  BBUTILS_queCreate(bbutils_que_t *pQueue, unsigned int maxNumber);
void BBUTILS_queDelete(bbutils_que_t *pQueue);
int  BBUTILS_quePut(bbutils_que_t *pQueue, int data, bool isWait);
int  BBUTILS_queGet(bbutils_que_t *pQueue, int *data, bool isWait);
bool BBUTILS_queIsFull(bbutils_que_t *pQueue);
bool BBUTILS_queIsEmpty(bbutils_que_t *pQueue);
int  BBUTILS_queGetCount(bbutils_que_t *pQueue);
/* End global function prototypes */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _BBLIBUTILS_H_ */

