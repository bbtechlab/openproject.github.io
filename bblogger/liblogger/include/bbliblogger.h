/*
 ===========================================================================
 Filename     : bbliblogger.h
 Version      : 1.0
 Created      : 15/04/2018 22:29:57
 Revision     : none
 Compiler     : gcc

 Author       : Bamboo Do, dovanquyen.vn@gmail.com
 Copyright (c) 2018,  All rights reserved.

 Description  :
 ===========================================================================
 */

#ifndef _BBLIBLOGGER_H_
#define _BBLIBLOGGER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************/
/**************************** Header Files**********************************/
/***************************************************************************/
/* Start Including Header Files */
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>    
#include <unistd.h>

#include "bblibtype.h"
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
/* Format strings cannot be longer than this. */
#define BBLOG_FORMAT_LENGTH              256
#define BBLOG_DATETIME_LENGTH            256

/* Default format strings. */
#define BBLOG_DEFAULT_FORMAT             "[%d %t, %l]%s %f.%n: %m"
#define BBLOG_DEFAULT_DATE_FORMAT        "%Y-%m-%d"
#define BBLOG_DEFAULT_TIME_FORMAT        "%H:%M:%S"

/* Default file path */
#define BBLOG_DEFAULT_FILE_NAME          "/tmp/bbliblogger.log"

/* Default file path of hlogger daemon */
#define BBLOG_DEFAULT_CONFIG_FILE_NAME   "/tmp/.bbliblogger.cfg"

/* Default color logs */
#define BBLOG_DEFAULT_COLOR_NONE         "\033[0m"   /* NONE */
#define BBLOG_DEFAULT_COLOR_TRACE        "\033[0m"   /* NONE */
#define BBLOG_DEFAULT_COLOR_MSG          "\033[0m"   /* NONE */
#define BBLOG_DEFAULT_COLOR_DBG          "\033[35m"  /* MAGENTA */
#define BBLOG_DEFAULT_COLOR_WRN          "\033[34m"  /* BLUE */
#define BBLOG_DEFAULT_COLOR_ERR          "\033[31m"  /* RED */

/* End Macro definition */

/***************************************************************************/
/****************************** typedef ************************************/
/***************************************************************************/
/* Start typedef */
typedef enum
{
    BBLOG_OUTPUT_CONSOLE = 1,
    BBLOG_OUTPUT_FILE,
    BBLOG_OUTPUT_MAX
} BBLOG_OUTPUT_e;

typedef enum
{
    BBLOG_LEVEL_TRACE = 1,
    BBLOG_LEVEL_DBG,
    BBLOG_LEVEL_MSG,
    BBLOG_LEVEL_WRN,
    BBLOG_LEVEL_ERR
} BBLOG_LEVEL_e;

typedef struct bblog_module
{
    /* Name of module */
    char                        *name;

    /* The current log level of this module. All message is below it will be dropped */
    BBLOG_LEVEL_e               level;

    /* The current output of this module. */
    BBLOG_OUTPUT_e              outLevel;

    /* list module */
    BBLIST_ENTRY (bblog_module) link;

} bblog_module_t;

/*
 * Logging macros
 */
void BBLOG_Log(BBLOG_LEVEL_e level, const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo, const char *fmt, ...);

void BBLOG_LogEnter(const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo);

void BBLOG_LogExit(const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo);
				
void BBLOG_LogPrint(BBLOG_LEVEL_e level, const char *moduleName, const char *fmt, ...);				

#define BBLOG_DEBUG_LEVEL(module_name, log_level, output)    \
    static bblog_module_t s_tBBLogModule = \
    {module_name, (log_level), (output)};   \
    static bool s_bDefaultLevelSet= false;

#define BBLOG_SET_DEFAULT_LEVEL \
do {    \
    if (!s_bDefaultLevelSet) {  \
        (void) BBLOG_SetDefaultLevel(&s_tBBLogModule); \
        s_bDefaultLevelSet = TRUE;  \
    }   \
} while(0)

#define BBLOG_ENTER()   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_LogEnter(s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__);   \
} while(0)

#define BBLOG_EXIT()   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_LogExit(s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__);   \
} while(0)

#define BBLOG_DBG(fmt, ...)   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_Log(BBLOG_LEVEL_DBG, s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__, fmt , ## __VA_ARGS__);   \
} while(0)

#define BBLOG_MSG(fmt, ...)   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_Log(BBLOG_LEVEL_MSG, s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__, fmt , ## __VA_ARGS__);   \
} while(0)

#define BBLOG_WRN(fmt, ...)   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_Log(BBLOG_LEVEL_WRN, s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__, fmt , ## __VA_ARGS__);   \
} while(0)

#define BBLOG_ERR(fmt, ...)   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_Log(BBLOG_LEVEL_ERR, s_tBBLogModule.name, __FILE__, __FUNCTION__, __LINE__, fmt , ## __VA_ARGS__);   \
} while(0)
	
#define BBLOG_PRINT(fmt, ...)   \
do {    \
    BBLOG_SET_DEFAULT_LEVEL;    \
    BBLOG_LogPrint(BBLOG_LEVEL_MSG, s_tBBLogModule.name, fmt , ## __VA_ARGS__);   \
} while(0)

/* End typedef */

/***************************************************************************/
/******************** global function prototype ****************************/
/***************************************************************************/
/* Start global function prototypes */
/***************************************************************************
  These are implemented at bbliblogger.c
 ***************************************************************************/
/* @Description:
 *  Init loggers
 *
 * @param:
 *
 * @return:
 *  <none>
 */
void BBLOG_Init(void);

/* @Description:
 *  Destroy loggers
 *
 * @param:
 *
 * @return:
 *  <none>
 */
void BBLOG_DeInit(void);

/**
 * @Description:
 *  Set the minimum level of messages that should be written to the log.
 *  Messages below this level will not be written.
 *
 * @param:
 *  - ptBBlogModule: The pointer point to module logger.
 *  - level: The new minimum log level.
 *
 * @return:
 *  Zero on success, non-zero on failure.
 */
int BBLOG_SetDefaultLevel(bblog_module_t *ptBBlogModule);

/**
 * @Description:
 *  Set the minimum level of messages that should be written to the log.
 *  Messages below this level will not be written.
 *
 * @param:
 *  - name: The name of module logger.
 *  - level: The new minimum log level.
 *
 * @return:
 *  Zero on success, non-zero on failure.
 */
int BBLOG_SetModuleLevel(const char *name, BBLOG_LEVEL_e level);

/**
 * @Description:
 *  Set the output features of messages that should be written to the log.
 *
 * @param:
 *  - name: The name of module logger.
 *  - output: The new output of module logger.
 *
 * @return:
 *  Zero on success, non-zero on failure.
 */
int BBLOG_SetModuleOutput(const char *name, BBLOG_OUTPUT_e output);

/**
 * @Description:
 *  Print out the information of logger
 *
 * @return:
 *  <none>
 */
void BBLOG_ShowLogLevel(void);

/**
 * @Description:
 *  Set the format string used for times.  See strftime(3) for how this string
 *  should be defined.  The default format string is HLOG_DEFAULT_TIME_FORMAT.
 *
 * @param:
 *  - fmt: The new format string, which must be less than HLOG_FORMAT_LENGTH bytes.
 *
 * @return
 *  Zero on success, non-zero on failure.
 */
int BBLOG_SetTimeFormat(const char *fmt);

/**
 * Set the format string used for dates.  See strftime(3) for how this string
 * should be defined.  The default format string is HLOG_DEFAULT_DATE_FORMAT.
 * 
 * @param fmt
 * The new format string, which must be less than HLOG_FORMAT_LENGTH bytes.
 *
 * @return
 * Zero on success, non-zero on failure.
 */
int BBLOG_SetDateFormat(const char *fmt);

/**
 * @Description:
 *  Set the format string for log messages.  Here are the substitutions you may use:
 *
 *     %f: Source file name generating the log call.
 *     %n: Source line number where the log call was made.
 *     %m: The message text sent to the logger (after printf formatting).
 *     %d: The current date, formatted using the logger's date format.
 *     %t: The current time, formatted using the logger's time format.
 *     %l: The log level (one of "DEBUG", "INFO", "WARN", or "ERROR").
 *     %%: A literal percent sign.
 *
 * The default format string is BBLOG_DEFAULT_FORMAT.
 * 
 * @param:
 *  - fmt: The new format string, which must be less than HLOG_FORMAT_LENGTH bytes.
 *      You probably will want to end this with a newline (\n).
 *
 * @return:
 *  Zero on success, non-zero on failure.
 */
int BBLOG_SetLayoutFormat(const char *fmt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* End global function prototypes */
#endif /* _BBLIBLOGGER_H_ */

