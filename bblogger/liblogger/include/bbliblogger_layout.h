/*
============================================================================
Filename     : bbliblogger_layout.h
Version      : 1.0
Created      : 17/04/2018 10:32:15
Revision     : none
Compiler     : gcc

Author       : Bamboo Do, dovanquyen.vn@gmail.com
Copyright (c) 2018,  All rights reserved.

Description  :
============================================================================
*/

#ifndef _BBLIBLOGGER_LAYOUT_H_
#define _BBLIBLOGGER_LAYOUT_H_

/***************************************************************************/
/**************************** Header Files**********************************/
/***************************************************************************/
/* Start Including Header Files */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bbliblogger.h"
#include "bbliblogger_layout.h"
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
struct  LogWriter;
typedef int (*Log) (struct LogWriter *pLogWriter,
                    const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel,
                    char buf[], size_t bufSize,
                    const char *logFmt, const char *dateFmt, const char *timeFmt,
                    const char *fileName, const char *funcName, const int lineNo,
                    const char *message);
typedef int (*LogFuncEntry) (struct LogWriter *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                    char buf[], size_t bufSize,
                    const char *logFmt, const char *dateFmt, const char *timeFmt,
                    const char *fileName, const char *funcName, const int lineNo);
typedef int (*LogFuncExit) (struct LogWriter *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                    char buf[], size_t bufSize,
                    const char *logFmt, const char *dateFmt, const char *timeFmt,
                    const char *fileName, const char *funcName, const int lineNo);
					
typedef int (*LogPrint) (struct LogWriter *pLogWriter,
                    const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel,
                    const char *message);
					
typedef struct LogWriter
{
    /* Member function to log */
    Log             log;

    /* Member function to log the function entry*/
    LogFuncEntry    logFuncEntry;

    /* Member function to log the function exit*/
    LogFuncExit     logFuncExit;

	/* Member function to log the function print*/
	LogPrint		logPrint;
} LogWriter_t;

typedef struct bblog_layout
{
    LogWriter_t logWriter;
    FILE        *fp;
} bblog_layout_t;
/* End typedef */

/***************************************************************************/
/******************** global function prototype ****************************/
/***************************************************************************/
/* Start global function prototypes */
/***************************************************************************
  These are implemented at bbliblogger_layout.c
 ***************************************************************************/
int     BBLOG_LAYOUT_Init(LogWriter_t **logWriter);
void    BBLOG_LAYOUT_DeInit(LogWriter_t *logWriter);

/* End global function prototypes */
#endif /* _BBLIBLOGGER_LAYOUT_H_ */

