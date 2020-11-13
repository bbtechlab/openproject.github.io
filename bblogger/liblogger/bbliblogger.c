/*
=============================================================================
Filename     : bbliblogger.c
Version      : 1.0
Created      : 15/04/2018 22:31:44
Revision     : none
Compiler     : gcc
Author       : Bamboo Do, dovanquyen.vn@gmail.com
Copyright (c) 2018,  All rights reserved.
Description  :
=============================================================================
*/
/***************************************************************************/
/**************************** Header Files *********************************/
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

#include "bbliberr.h"
#include "bblibtype.h"
#include "bbliblist.h"
#include "bbliblogger.h"
#include "bbliblogger_layout.h"
#include "bbliblogger_cmd.h"
/* End Including Headers */

BBLOG_DEBUG_LEVEL("BBLOG", BBLOG_LEVEL_WRN, BBLOG_OUTPUT_MAX)

/***************************************************************************/
/****************************** define *************************************/
/***************************************************************************/
/* Start #define */
#define PR_ERR(fmt, ...)    fprintf(stdout, BBLOG_DEFAULT_COLOR_ERR fmt  BBLOG_DEFAULT_COLOR_NONE "",  ## __VA_ARGS__)
#define PR_WRN(fmt, ...)    fprintf(stdout, BBLOG_DEFAULT_COLOR_WRN fmt  BBLOG_DEFAULT_COLOR_NONE "", ## __VA_ARGS__)
#define PR_DBG(fmt, ...)    fprintf(stdout, BBLOG_DEFAULT_COLOR_DBG fmt  BBLOG_DEFAULT_COLOR_NONE "",  ## __VA_ARGS__)
#define PR_MSG(fmt, ...)    fprintf(stdout, BBLOG_DEFAULT_COLOR_MSG fmt  BBLOG_DEFAULT_COLOR_NONE "",  ## __VA_ARGS__)
#define PR_TRACE(fmt, ...)  fprintf(stdout, BBLOG_DEFAULT_COLOR_TRACE fmt  BBLOG_DEFAULT_COLOR_NONE "",  ## __VA_ARGS__)
/* End #define */

/***************************************************************************/
/*********************** Type Defination ***********************************/
/***************************************************************************/
/* Start typedef */
typedef struct bblogger 
{

    /* The list of bblogger module */
    BBLIST_HEAD(list_logger_modules, bblog_module) list_logger_modules;

    /* The number of bblogger modules */
    int             num_loger_modules;

    /* The format specifier. */
    char            fmt[BBLOG_FORMAT_LENGTH];

    /* Date format */
    char            date_fmt[BBLOG_FORMAT_LENGTH];

    /* Time format */
    char            time_fmt[BBLOG_FORMAT_LENGTH];

    /* Tracks whether the fd needs to be closed eventually. */
    int             initialized;

} bblogger_t;
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
static bblogger_t       s_tBBLogger;
static LogWriter_t      *s_ptBBLogWriter    = NULL;
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
static bblog_module_t   *P_BBLOG_SearchByName(const char *name);
static char             *P_BBLOG_ConvertLevelToName(const BBLOG_LEVEL_e log_level);
static char             *P_BBLOG_ConvertOutputToName(const BBLOG_OUTPUT_e output);
/* End static functions */


/***************************************************************************/
/*********************** Function Description*******************************/
/***************************************************************************/
#define ___STATIC_FUNCTION___________________________________________________
static bblog_module_t *P_BBLOG_SearchByName(const char *name)
{
    bblog_module_t    *module = NULL;

    module = BBLIST_FIRST (&s_tBBLogger.list_logger_modules);
    while(module)
    {
        if (strcmp(module->name, name) == 0)
        {
            break;
        }

        module = BBLIST_NEXT(module, link);
    }

    return module;
}

static char *P_BBLOG_ConvertLevelToName(const BBLOG_LEVEL_e log_level)
{
    BBLOG_LEVEL_e level = log_level;

    switch (level)
    {
        case BBLOG_LEVEL_TRACE:
            return "TRACE";

        case BBLOG_LEVEL_DBG:
            return "DEBUG";

        case BBLOG_LEVEL_MSG:
            return "INFOR";

        case BBLOG_LEVEL_WRN:
            return "WARNN";

        case BBLOG_LEVEL_ERR:
            return "ERROR";
        default:
            return NULL;
    }

    return NULL;
}

static char *P_BBLOG_ConvertOutputToName(const BBLOG_OUTPUT_e output)
{
    BBLOG_OUTPUT_e out = output;

    switch (out)
    {
        case BBLOG_OUTPUT_CONSOLE:
            return "CONSOLE";

        case BBLOG_OUTPUT_FILE:
            return "FILE";

        case BBLOG_OUTPUT_MAX:
            return "ALL";

        default:
            return NULL;
    }

    return NULL;
}

static void P_BBLOG_Log(BBLOG_LEVEL_e level, bblog_module_t *module,
                    const char *fileName, const char *funcName,
                    const int lineNo, const char *fmt, va_list ap)
{
    BBERR_e bErr = BBERR_OK;

    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char    buf[4096];
    size_t  buf_size = 4096;
    char    *dynbuf = buf;
    char    *message;
    char    message_buf[4096];
    int     result;
    va_list ap_copy;

    /* Format the message text with the argument list. */
    va_copy(ap_copy, ap);
    result = vsnprintf(dynbuf, buf_size, fmt, ap);
    if ((size_t) result >= buf_size)
    {
        buf_size = result + 1;
        dynbuf = (char *) malloc(buf_size);
        result = vsnprintf(dynbuf, buf_size, fmt, ap_copy);
        if ((size_t) result >= buf_size)
        {
            /* Formatting failed -- too large */
            PR_ERR("[%s:%d]Formatting failed.\n", __FUNCTION__, __LINE__);
            va_end(ap_copy);
            free(dynbuf);
            return;
        }
    }
    va_end(ap_copy);

    /* Format according to log format and write to log */
    bErr = s_ptBBLogWriter->log(s_ptBBLogWriter,
                level, module->outLevel, message_buf, 4096,
                &s_tBBLogger.fmt[0], &s_tBBLogger.date_fmt[0], &s_tBBLogger.time_fmt[0],
                fileName, funcName, lineNo,
                dynbuf);

    if (dynbuf != buf)
    {
        free(dynbuf);
    }

    return;
}

static void P_BBLOG_LogPrint(BBLOG_LEVEL_e level, bblog_module_t *module,
                    const char *fmt, va_list ap)
{
    BBERR_e bErr = BBERR_OK;

    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char    buf[4096];
    size_t  buf_size = 4096;
    char    *dynbuf = buf;
    int     result;
    va_list ap_copy;

    /* Format the message text with the argument list. */
    va_copy(ap_copy, ap);
    result = vsnprintf(dynbuf, buf_size, fmt, ap);
    if ((size_t) result >= buf_size)
    {
        buf_size = result + 1;
        dynbuf = (char *) malloc(buf_size);
        result = vsnprintf(dynbuf, buf_size, fmt, ap_copy);
        if ((size_t) result >= buf_size)
        {
            /* Formatting failed -- too large */
            PR_ERR("[%s:%d]Formatting failed.\n", __FUNCTION__, __LINE__);
            va_end(ap_copy);
            free(dynbuf);
            return;
        }
    }
    va_end(ap_copy);

    /* Format according to log format and write to log */
    bErr = s_ptBBLogWriter->logPrint(s_ptBBLogWriter,
                level, module->outLevel, dynbuf);

    if (dynbuf != buf)
    {
        free(dynbuf);
    }

    return;
}

#define ___GLOBAL_FUNCTION___________________________________________________
/* @Description:
 *  Init loggers
 *
 * @param:
 *  - parms: The module parameters of logger.
 *
 * @return:
 *  <none>
 */
void BBLOG_Init(void)
{
    if (1 == s_tBBLogger.initialized)
    {
        BBLOG_WRN("BBLogger already initialized. \n");
        return;
    }

    BBLIST_INIT(&s_tBBLogger.list_logger_modules);
    s_tBBLogger.num_loger_modules = 0;

    strcpy(s_tBBLogger.fmt,      BBLOG_DEFAULT_FORMAT);
    strcpy(s_tBBLogger.date_fmt, BBLOG_DEFAULT_DATE_FORMAT);
    strcpy(s_tBBLogger.time_fmt, BBLOG_DEFAULT_TIME_FORMAT);

    (void)BBLOG_LAYOUT_Init(&s_ptBBLogWriter);

    (void)BBLOG_CMD_Init();

    s_tBBLogger.initialized = 1;
}

/* @Description:
 *  Destroy loggers
 *
 * @param:
 *
 * @return:
 *  <none>
 */
void BBLOG_DeInit(void)
{
    BBLOG_MSG("BBLogger Destroying ... \n");
    bblog_module_t *module      = BBLIST_FIRST(&s_tBBLogger.list_logger_modules);
    bblog_module_t *next_module = NULL;
    while(module != NULL)
    {
        next_module = BBLIST_NEXT(module, link);
        BBLIST_REMOVE(&s_tBBLogger.list_logger_modules, module, link);
        module      = next_module;
    }
    BBLIST_INIT(&s_tBBLogger.list_logger_modules);

    BBLOG_LAYOUT_DeInit(s_ptBBLogWriter);

    s_ptBBLogWriter = NULL;

    BBLOG_CMD_DeInit();

    s_tBBLogger.initialized         = 0;
    s_tBBLogger.num_loger_modules   = 0;

    BBLOG_MSG("BBLogger Destroy Finished. \n");
}

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
int BBLOG_SetDefaultLevel(bblog_module_t *ptBBlogModule)
{
    BBERR_e             bbErr   = BBERR_OK;
    bblog_module_t      *module = NULL;

    // Mutex_Lock, fix me

    module = BBLIST_FIRST(&s_tBBLogger.list_logger_modules);
    while(module)
    {
        if (0 == strcmp(module->name, ptBBlogModule->name))
        {
            break;
        }

        module = BBLIST_NEXT(module, link);
    }

    /* If it exists, set log_level and output for this module logger */
    if (module)
    {
        module->level       = ptBBlogModule->level;
        module->outLevel    = ptBBlogModule->outLevel;
    }
    else
    {
        /* If it's a new module logger, add it to the list */
        BBLIST_INSERT_HEAD (&s_tBBLogger.list_logger_modules, ptBBlogModule, link);
        s_tBBLogger.num_loger_modules++;
    }

    // Mutex_Unlock, fix me

    return bbErr;
}

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
int BBLOG_SetModuleLevel(const char *name, BBLOG_LEVEL_e level)
{
    BBERR_e             bbErr   = BBERR_OK;
    bblog_module_t      *module = NULL;

    // Mutex_Lock, fix me
    if (name == NULL)
    {
        BBLOG_WRN("Invalid params.\n");

        return BBERR_INVAL;
    }

    module = P_BBLOG_SearchByName(name);
    if (module != NULL)
    {
        module->level = level;
    }
    else
    {
        BBLOG_WRN("Can not find any module name:%s.\n", name);
        bbErr = BBERR_ERROR;
    }
    // Mutex_Unlock, fix me

    return bbErr;
}

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
int BBLOG_SetModuleOutput(const char *name, BBLOG_OUTPUT_e output)
{
    BBERR_e             bbErr   = BBERR_OK;
    bblog_module_t      *module = NULL;

    // Mutex_Lock, fix me
    if (name == NULL)
    {
        BBLOG_WRN("Invalid params.\n");

        return BBERR_INVAL;
    }

    module = P_BBLOG_SearchByName(name);
    if (module != NULL)
    {
        module->outLevel = output;
    }
    else
    {
        BBLOG_WRN("Can not find any module name:%s.\n", name);
        bbErr = BBERR_ERROR;
    }
    // Mutex_Unlock, fix me

    return bbErr;
}

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
int BBLOG_SetTimeFormat(const char *fmt)
{
    BBERR_e     bbErr = BBERR_OK;

    // Mutex_Lock, fix messages
    if (strlen(fmt) < BBLOG_FORMAT_LENGTH)
    {
        strcpy(s_tBBLogger.time_fmt, fmt);
    }
    else
    {
        BBLOG_ERR("Invalid params.\n");
        bbErr = BBERR_ERROR;
    }
    // Mutex_Unlock, fix me

    return bbErr;
}

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
int BBLOG_SetDateFormat(const char *fmt)
{
    BBERR_e     bbErr = BBERR_OK;

    // Mutex_Lock, fix messages
    if (strlen(fmt) < BBLOG_FORMAT_LENGTH)
    {
        strcpy(s_tBBLogger.date_fmt, fmt);
    }
    else
    {
        BBLOG_ERR("Invalid params.\n");
        bbErr = BBERR_ERROR;
    }
    // Mutex_Unlock, fix me

    return bbErr;
}

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
int BBLOG_SetLayoutFormat(const char *fmt)
{
    BBERR_e     bbErr = BBERR_OK;

    // Mutex_Lock, fix messages
    if (strlen(fmt) < BBLOG_FORMAT_LENGTH)
    {
        strcpy(s_tBBLogger.fmt, fmt);
    }
    else
    {
        BBLOG_ERR("Invalid params.\n");
        bbErr = BBERR_ERROR;
    }
    // Mutex_Unlock, fix me

    return bbErr;
}

/**
 * @Description:
 *  Print out the information of logger
 *
 * @return:
 *  <none>
 */
void BBLOG_ShowLogLevel(void)
{
    bblog_module_t *module        = NULL;
    bblog_module_t *next_module   = NULL;

    PR_MSG("\nLOGGING LEVEL STATUS:\n");
    PR_MSG("---------------------------------+-----------+-----------\n");
    PR_MSG("            MODULE NAME          | LOG LEVEL | OUT LEVEL \n");
    PR_MSG("---------------------------------+-----------+-----------\n");
    // Mutex_Lock, fix messages
    module = BBLIST_FIRST (&s_tBBLogger.list_logger_modules);
    while(module != NULL)
    {
        next_module = BBLIST_NEXT(module, link);
        PR_MSG("%32s | %9s | %9s\n", module->name,
            (char *)P_BBLOG_ConvertLevelToName(module->level),
            (char *)P_BBLOG_ConvertOutputToName(module->outLevel));

        module = next_module;
    }
    // Mutex_Unlock, fix me
}

void BBLOG_Log(BBLOG_LEVEL_e level, const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo, const char *fmt, ...)
{
    bblog_module_t *module = NULL;
    va_list ap;

    // Mutex_Lock, fix messages
    module = P_BBLOG_SearchByName(moduleName);
    if (module != NULL)
    {
        if (level < module->level)
        {
            /* All message is below log level will be dropped */
            return;
        }
        else
        {
            va_start(ap, fmt);
            P_BBLOG_Log(level, module, fileName, funcName, lineNo, fmt, ap);
            va_end(ap);
        }
    }
    // Mutex_Unlock, fix me

    return;
}

void BBLOG_LogEnter(const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo)
{
    BBERR_e bErr            = BBERR_OK;
    bblog_module_t *module  = NULL;
    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char    buf[1024];
    size_t  buf_size = 1024;    

    // Mutex_Lock, fix messages
    module = P_BBLOG_SearchByName(moduleName);
    if (module != NULL)
    {
        if (BBLOG_LEVEL_TRACE < module->level)
        {
            /* All message is below log level will be dropped */
            return;
        }
        else
        {
            bErr = s_ptBBLogWriter->logFuncEntry(s_ptBBLogWriter, module->outLevel,
                        buf, buf_size,
                        &s_tBBLogger.fmt[0], &s_tBBLogger.date_fmt[0], &s_tBBLogger.time_fmt[0],
                        fileName, funcName, lineNo);
        }
    }
    // Mutex_Unlock, fix me

    return;
}

void BBLOG_LogExit(const char *moduleName, const char *fileName,
                const char *funcName, const int lineNo)
{
    BBERR_e bErr            = BBERR_OK;
    bblog_module_t *module  = NULL;
    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char    buf[1024];
    size_t  buf_size = 1024;     

    // Mutex_Lock, fix messages
    module = P_BBLOG_SearchByName(moduleName);
    if (module != NULL)
    {
        if (BBLOG_LEVEL_TRACE < module->level)
        {
            /* All message is below log level will be dropped */
            return;
        }
        else
        {
            bErr = s_ptBBLogWriter->logFuncExit(s_ptBBLogWriter, module->outLevel,
                        buf, buf_size,
                        &s_tBBLogger.fmt[0], &s_tBBLogger.date_fmt[0], &s_tBBLogger.time_fmt[0],
                        fileName, funcName, lineNo);
        }
    }
    // Mutex_Unlock, fix me

    return;
}

void BBLOG_LogPrint(BBLOG_LEVEL_e level, const char *moduleName, const char *fmt, ...)
{
    bblog_module_t *module = NULL;
    va_list ap;

    // Mutex_Lock, fix messages
    module = P_BBLOG_SearchByName(moduleName);
    if (module != NULL)
    {
        if (level < module->level)
        {
            /* All message is below log level will be dropped */
            return;
        }
        else
        {
            va_start(ap, fmt);
            P_BBLOG_LogPrint(level, module, fmt, ap);
            va_end(ap);
        }
    }
    // Mutex_Unlock, fix me

    return;
}

/*********************** End of File ***************************************/
