/*
============================================================================
Filename     : bblibutils_stack.c
Version      : 1.0
Created      : 27/04/2018 00:24:24
Revision     : none
Compiler     : gcc
Author       : Bamboo Do, dovanquyen.vn@gmail.com
Copyright (c) 2018,  All rights reserved.
Description  : 
============================================================================
*/
/***************************************************************************/
/**************************** Header Files *********************************/
/***************************************************************************/
/* Start Including Header Files */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "bbliblogger.h"
#include "bblibutils.h"
#include "bbliberr.h"
/* End Including Headers */

BBLOG_DEBUG_LEVEL("BBUTILS_STACK", BBLOG_LEVEL_WRN, BBLOG_OUTPUT_MAX)

/***************************************************************************/
/****************************** define *************************************/
/***************************************************************************/
/* Start #define */
/* End #define */

/***************************************************************************/
/*********************** Type Defination ***********************************/
/***************************************************************************/
/* Start typedef */
/* End typedef */


/***************************************************************************/
/*********************** Global Variables **********************************/
/***************************************************************************/
/* Start global variable */
/* End global variable */


/***************************************************************************/
/*********************** Static Variables **********************************/
/***************************************************************************/
/* Start static variable */
/* End static variable */


/***************************************************************************/
/******************** Global Functions *************************************/
/***************************************************************************/
/* Start global functions */
/* End global functions */


/***************************************************************************/
/*********************** Static Functions **********************************/
/***************************************************************************/
/* Start static functions */
/* End static functions */


/***************************************************************************/
/*********************** Function Description*******************************/
/***************************************************************************/
#define ___STATIC_FUNCTION___________________________________________________

#define ___GLOBAL_FUNCTION___________________________________________________
int  BBUTILS_stackCreate(bbutils_stack_t *pStack, unsigned int maxNumber)
{
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t 	cond_attr;	
	BBERR_e				status = BBERR_OK;
	
	BBLOG_ENTER();
	
	pStack->top = 0;
	
	pStack->data = (int *)malloc(maxNumber);
	
	if (pStack->data == NULL)
	{
		BBLOG_ERR("Failed to malloc Stack\n");
		BBLOG_EXIT();
		return BBERR_ERROR;
	}
	
	status |= pthread_mutexattr_init(&mutex_attr);
	status |= pthread_mutex_init(&pStack->lock, &mutex_attr); 
	
	pStack->maxNumber = maxNumber;
	
	if (status != BBERR_OK)
	{
		BBLOG_ERR("Stack Create Failed\n");
		goto done;
	}
	
	BBLOG_MSG("Stack Initialized\n");
	
done:	
	pthread_mutexattr_destroy(&mutex_attr);	
	
	BBLOG_EXIT();	
	
	return status;
}

void BBUTILS_stackDelete(bbutils_stack_t *pStack)
{
	BBLOG_ENTER();
	
	if (pStack->data != NULL)
	{
		free(pStack->data);
	}
	
	pStack->top 		= 0;
	pStack->maxNumber 	= 0;
	pthread_mutex_destroy(&pStack->lock); 
	
	BBLOG_MSG("Stack DeInitialized\n");
	
	BBLOG_EXIT();	
}

int BBUTILS_stackPush(bbutils_stack_t *pStack, int data)
{
	BBLOG_ENTER();
	
	pthread_mutex_lock(&pStack->lock);
	
	if (BBUTILS_stackIsFull)
	{
		BBLOG_ERR("Stack is full\n");
		BBLOG_EXIT();
		return BBERR_ERROR;		
	}
	
	pStack->data[pStack->top] = data;
	(pStack->top)++;
	
	pthread_mutex_unlock(&pStack->lock);
	
	BBLOG_EXIT();	
	
	return BBERR_OK;
}

int BBUTILS_stackPop(bbutils_stack_t *pStack, int *data)
{
	BBLOG_ENTER();
	
	pthread_mutex_lock(&pStack->lock);	
	
	if (BBUTILS_stackIsEmpty)
	{
		BBLOG_ERR("Stack is empty\n");
		BBLOG_EXIT();
		return BBERR_ERROR;
	}
	(pStack->top)--;
	*(data) = pStack->data[pStack->top];
	
	pthread_mutex_unlock(&pStack->lock);
	
	BBLOG_EXIT();

	return BBERR_OK;
}

bool BBUTILS_stackIsFull(bbutils_stack_t *pStack)
{
	bool isFull;
	
	BBLOG_ENTER();
	
	pthread_mutex_lock(&pStack->lock);
	
	if (pStack->top >= pStack->maxNumber)
	{
		isFull = TRUE;
	}
	else
	{
		isFull = FALSE;
	}
	
	pthread_mutex_unlock(&pStack->lock);
	
	BBLOG_EXIT();
	
	return isFull;
}

bool BBUTILS_stackIsEmpty(bbutils_stack_t *pStack)
{
	bool isEmpty; 
	
	BBLOG_ENTER();
	
	pthread_mutex_lock(&pStack->lock);
	
	if (pStack->top <= 0)
	{
		isEmpty = TRUE;
	}
	else
	{
		isEmpty = FALSE;
	}
	
	pthread_mutex_unlock(&pStack->lock);
	
	BBLOG_EXIT();
	
	return isEmpty;
}

void BBUTILS_stackDump(bbutils_stack_t *pStack)
{
	unsigned int i;
	
	if (pStack->top == 0)
	{
		BBLOG_WRN("Stack is empty\n");
	}
	else
	{
		BBLOG_MSG("Stack Contents: \n");
		for (i=0; i  < pStack->top; i++)
		{
			BBLOG_PRINT("0x%X\t", pStack->data[i]);
		}
		BBLOG_PRINT("\n");
	}
}

/*********************** End of File ***************************************/


