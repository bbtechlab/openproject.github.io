/*
============================================================================
Filename     : bblibutils_que.c
Version      : 1.0
Created      : 27/04/2018 00:24:37
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

BBLOG_DEBUG_LEVEL("BBUTILS_QUEUE", BBLOG_LEVEL_WRN, BBLOG_OUTPUT_MAX)

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
int  BBUTILS_queCreate(bbutils_que_t *pQueue, unsigned int maxNumber)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t  cond_attr;
    BBERR_e             status = BBERR_OK;

    BBLOG_ENTER();

    pQueue->data        = malloc(maxNumber*sizeof(int));
    if (pQueue->data == NULL)
    {
        BBLOG_ERR("Failed to malloc Queue\n");
        BBLOG_EXIT();
        return BBERR_ERROR;
    }

    pQueue->curRd       = pQueue->curWr = 0;
    pQueue->maxNumber   = maxNumber;
    pQueue->countNumber = 0;

    status |= pthread_mutexattr_init(&mutex_attr);
    status |= pthread_condattr_init(&cond_attr);
    status |= pthread_mutex_init(&pQueue->lock, &mutex_attr);
    status |= pthread_cond_init(&pQueue->condRd, &cond_attr);
    status |= pthread_cond_init(&pQueue->condWr, &cond_attr);

    if (status != BBERR_OK)
    {
        BBLOG_ERR("Queue create failed!\n");
        goto done;
    }

    BBLOG_MSG("Queue Initialized\n");

done:
    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    BBLOG_EXIT();

    return status;
}

void  BBUTILS_queDelete(bbutils_que_t *pQueue)
{
    BBLOG_ENTER();

    if(pQueue->data != NULL)
    {
        free(pQueue->data);
    }

    pthread_cond_destroy(&pQueue->condRd);
    pthread_cond_destroy(&pQueue->condWr);
    pthread_mutex_destroy(&pQueue->lock);

    BBLOG_EXIT();
}

int  BBUTILS_quePut(bbutils_que_t *pQueue, int data, bool isWait)
{
    int status = BBERR_OK;

    BBLOG_ENTER();

    pthred_mutex_lock(&pQueue->lock);

    while(1)
    {
        if (pQueue->countNumber < pQueue->maxNumber)
        {
            pQueue->data[pQueue->curWr] = data;
            pQueue->curWr               = (pQueue->curWr+1)%pQueue->maxNumber;
            pQueue->countNumber++;
            pthread_cond_signal(&pQueue->condRd);
            break;
        }
        else
        {
            if (isWait == FALSE)
            {
                BBLOG_ERR("Queue is Full\n");
                status = BBERR_ERROR;
                break;
            }

            status = pthread_cond_wait(&pQueue->condWr,  &pQueue->lock);
        }
    }

    pthred_mutex_unlock(&pQueue->lock);

    BBLOG_EXIT();

    return status;
}

int  BBUTILS_queGet(bbutils_que_t *pQueue, int *data, bool isWait)
{
    int status = BBERR_OK;

    BBLOG_ENTER();

    if (data == NULL)
    {
        BBLOG_ERR("Invalid input params\n");
        status = BBERR_INVAL;
        goto done;
    }

    pthred_mutex_lock(&pQueue->lock);

    while(1)
    {
        if (pQueue->countNumber > 0)
        {
            *data           = pQueue->data[pQueue->curRd];
            pQueue->curRd   = (pQueue->curRd+1)%pQueue->maxNumber;
            pQueue->countNumber--;
            pthread_cond_signal(&pQueue->condWr);
            break;
        }
        else
        {
            if (isWait == FALSE)
            {
                BBLOG_ERR("Queue is Empty\n");
                status = BBERR_ERROR;
                break;
            }

            status = pthread_cond_wait(&pQueue->condRd,  &pQueue->lock);
        }
    }

    pthred_mutex_unlock(&pQueue->lock);

done:

    BBLOG_EXIT();

    return status;
}

bool BBUTILS_queIsFull(bbutils_que_t *pQueue)
{
    bool isFull;

    BBLOG_ENTER();

    pthred_mutex_lock(&pQueue->lock);
    if (pQueue->countNumber == pQueue->maxNumber)
    {
        isFull = TRUE;
    }
    else
    {
        isFull = FALSE;
    }
    pthred_mutex_unlock(&pQueue->lock);

    BBLOG_EXIT();

    return isFull;
}

bool BBUTILS_queIsEmpty(bbutils_que_t *pQueue)
{
    bool isEmpty;

    BBLOG_ENTER();

    pthred_mutex_lock(&pQueue->lock);

    if (pQueue->countNumber == 0)
    {
        isEmpty = TRUE;
    }
    else
    {
        isEmpty = FALSE;
    }

    pthred_mutex_unlock(&pQueue->lock);

    BBLOG_EXIT();

    return isEmpty;
}

int BBUTILS_queGetCount(bbutils_que_t *pQueue)
{
    int count = 0;

    BBLOG_ENTER();

    pthread_mutex_lock(&pQueue->lock);

    count = pQueue->countNumber;

    pthread_mutex_unlock(&pQueue->lock);

    BBLOG_EXIT();

    return count;
}
/*********************** End of File ***************************************/


