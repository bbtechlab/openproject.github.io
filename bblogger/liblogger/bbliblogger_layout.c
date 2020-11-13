/*
 ============================================================================
 Filename     : bbliblogger_layout.c
 Version      : 1.0
 Created      : 15/04/2018 22:33:36
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
#include "bbliblogger.h"
#include "bbliblogger_layout.h"
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
/* Helper function to write the logs to file */
static int P_BBLOG_LAYOUT_Log(LogWriter_t *pLogWriter,
                            const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo,
                            const char *message);

/* File Logger object function to log function entry */
static int P_BBLOG_LAYOUT_LogFuncEntry(LogWriter_t *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo);

/* File Logger object function to log function exit */
static int P_BBLOG_LAYOUT_LogExit(LogWriter_t *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo);

static int P_BBLOG_LAYOUT_LogPrint(LogWriter_t *pLogWriter,
                            const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel,
                            const char *message);
							
static bblog_layout_t s_tBBLogLayout = {
    {
        .log            = P_BBLOG_LAYOUT_Log,
        .logFuncEntry   = P_BBLOG_LAYOUT_LogFuncEntry,
        .logFuncExit    = P_BBLOG_LAYOUT_LogExit,
		.logPrint       = P_BBLOG_LAYOUT_LogPrint,
    },
    .fp = 0
};
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
static const char *P_BBLOG_LAYOUT_BaseName(const char *path);
static size_t P_BBLOG_LAYOUT_AppendStr(char **dst, char *orig_buf, const char *src, size_t cur_size);
static size_t P_BBLOG_LAYOUT_AppendTime(char **dst, char *orig_buf, struct tm *lt,
                  const char *fmt, size_t cur_size);
static size_t P_BBLOG_LAYOUT_AppendInt(char **dst, char *orig_buf, long int d, size_t cur_size);
static char *P_BBLOG_LAYOUT_GetLogLevelPrefix(const BBLOG_LEVEL_e logLevel);
static void P_BBLOG_LAYOUT_Print(const BBLOG_LEVEL_e log_level,  const char *msg);
/* End static functions */


/***************************************************************************/
/*********************** Function Description*******************************/
/***************************************************************************/
#define ___STATIC_FUNCTION___________________________________________________
static const char *P_BBLOG_LAYOUT_BaseName(const char *path)
{
    const char *slash = strrchr(path, '/');
    if (slash) {
        path = slash + 1;
    }
#ifdef _WIN32
    slash = strrchr(path, '\\');
    if (slash) {
        path = slash + 1;
    }
#endif

    return path;
}

static size_t P_BBLOG_LAYOUT_AppendStr(char **dst, char *orig_buf, const char *src, size_t cur_size)
{
    size_t new_size = cur_size;

    while (strlen(*dst) + strlen(src) >= new_size) {
        new_size *= 2;
    }
    if (new_size != cur_size) {
        if (*dst == orig_buf) {
            *dst = (char *) malloc(new_size);
            strcpy(*dst, orig_buf);
        } else {
            *dst = (char *) realloc(*dst, new_size);
        }
    }

    strcat(*dst, src);
    return new_size;
}

static size_t P_BBLOG_LAYOUT_AppendTime(char **dst, char *orig_buf, struct tm *lt,
                  const char *fmt, size_t cur_size)
{
    char buf[BBLOG_DATETIME_LENGTH];
    size_t result = strftime(buf, BBLOG_DATETIME_LENGTH, fmt, lt);

    if (result > 0) {
        return P_BBLOG_LAYOUT_AppendStr(dst, orig_buf, buf, cur_size);
    }

    return cur_size;
}

static size_t P_BBLOG_LAYOUT_AppendInt(char **dst, char *orig_buf, long int d, size_t cur_size)
{
    char buf[40]; /* Enough for 128-bit decimal */
    if (snprintf(buf, 40, "%ld", d) >= 40) {
        return cur_size;
    }
    return P_BBLOG_LAYOUT_AppendStr(dst, orig_buf, buf, cur_size);
}

static char *P_BBLOG_LAYOUT_GetLogLevelPrefix(const BBLOG_LEVEL_e logLevel)
{
    switch (logLevel)
    {
        case BBLOG_LEVEL_TRACE: return "TRACE";
        case BBLOG_LEVEL_DBG:   return "DEBUG";
        case BBLOG_LEVEL_MSG:   return "INFOR";
        case BBLOG_LEVEL_WRN:   return "WARNN";
        case BBLOG_LEVEL_ERR:   return "ERROR";
        default:                return "";
    }
}

static void P_BBLOG_LAYOUT_Print(const BBLOG_LEVEL_e log_level,  const char *msg)
{
    switch (log_level)
    {
        case BBLOG_LEVEL_TRACE:
            PR_TRACE("%s", msg);
            break;
        case BBLOG_LEVEL_DBG:
            PR_DBG("%s", msg);
            break;
        case BBLOG_LEVEL_MSG:
            PR_MSG("%s", msg);
            break;
        case BBLOG_LEVEL_WRN:
            PR_WRN("%s", msg);
            break;
        case BBLOG_LEVEL_ERR:
            PR_ERR("%s", msg);
            break;
    }
}

static int P_BBLOG_LAYOUT_Log(struct LogWriter *pLogWriter,
                            const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo,
                            const char *message)
{
    enum
    {
        NORMAL,
        SUBST
    } state             = NORMAL;
    time_t t            = time(NULL);
    struct tm *lt       = localtime(&t);
    size_t logFmtLen    = strlen(logFmt);
    size_t i;
    char *result        = buf;
    size_t cur_size     = bufSize;

    char *level         = P_BBLOG_LAYOUT_GetLogLevelPrefix(logLevel);
    result[0]           = 0;

#if defined (CONFIG_BBLOG_DEBUG)
    funcName = P_BBLOG_LAYOUT_BaseName(funcName);
#if !defined (CONFIG_BBLOG_DEBUG_FILENAME)
    HAPPY(fileName);
#else	
    fileName = P_BBLOG_LAYOUT_BaseName(fileName);	
#endif	
#else
    HAPPY(funcName);
    HAPPY(fileName);
    HAPPY(lineNo);
#endif

    bblog_layout_t *logWriter = (bblog_layout_t *)pLogWriter;

    if (!logWriter || !logWriter->fp)
    {
        PR_ERR("[%s:%d]Invalid params.\n", __FUNCTION__, __LINE__);
        return BBERR_ERROR;
    }

    for (i = 0; i < logFmtLen; ++i)
    {
        if (state == NORMAL)
        {
            if (logFmt[i] == '%')
            {
                state = SUBST;
            }
            else
            {
                char str[2] = {0,};
                str[0] = logFmt[i];
                cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, str, cur_size);
            }
        }
        else
        {
            switch (logFmt[i])
            {
                case '%':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, "%", cur_size);
                    break;
                case 't':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, timeFmt, cur_size);
                    break;
                case 'd':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, dateFmt, cur_size);
                    break;
                case 'l':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, level, cur_size);
                    break;
#if defined (CONFIG_BBLOG_DEBUG)
                case 'n':
                    cur_size = P_BBLOG_LAYOUT_AppendInt(&result, buf, lineNo, cur_size);
                    break;
#if defined (CONFIG_BBLOG_DEBUG_FILENAME)
                case 's':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, fileName, cur_size);
                    break;
#endif
                case 'f':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, funcName, cur_size);
                    break;
#endif
                case 'm':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, message, cur_size);
                    break;
            }

            state = NORMAL;

        }
    }

    switch (outLevel)
    {
        case BBLOG_OUTPUT_CONSOLE:
            P_BBLOG_LAYOUT_Print(logLevel, result);
            fflush(stdout);
            break;
        case BBLOG_OUTPUT_FILE:
            fprintf(logWriter->fp, "%s", result);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
        case BBLOG_OUTPUT_MAX:
            P_BBLOG_LAYOUT_Print(logLevel, result);
            fprintf(logWriter->fp, "%s", result);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
    }

    if (result != buf)
    {
        free(result);
    }

    return BBERR_OK;

}

static int P_BBLOG_LAYOUT_LogFuncEntry(struct LogWriter *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo)
{
    enum
    {
        NORMAL,
        SUBST
    } state             = NORMAL;
    time_t t            = time(NULL);
    struct tm *lt       = localtime(&t);
    size_t logFmtLen    = strlen(logFmt);
    size_t i;
    size_t cur_size     = bufSize;    
    char *result        = buf;
    

    char *level         = "Enter";
    result[0]           = 0;
    
    fileName = P_BBLOG_LAYOUT_BaseName(fileName);
    funcName = P_BBLOG_LAYOUT_BaseName(funcName);

    bblog_layout_t *logWriter = (bblog_layout_t *)pLogWriter;

    if (!logWriter || !logWriter->fp)
    {
        PR_ERR("[%s:%d]Invalid params.\n", __FUNCTION__, __LINE__);
        return BBERR_ERROR;
    }

    for (i = 0; i < logFmtLen; ++i)
    {
        if (state == NORMAL)
        {
            if (logFmt[i] == '%')
            {
                state = SUBST;
            }
            else
            {
                char str[2] = {0,};
                str[0] = logFmt[i];
                cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, str, cur_size);
            }
        }
        else
        {
            switch (logFmt[i])
            {
                case '%':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, "%", cur_size);
                    break;
                case 't':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, timeFmt, cur_size);
                    break;
                case 'd':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, dateFmt, cur_size);
                    break;
                case 'l':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, level, cur_size);
                    break;
                case 'n':
                    cur_size = P_BBLOG_LAYOUT_AppendInt(&result, buf, lineNo, cur_size);
                    break;
#if defined (CONFIG_BBLOG_DEBUG_FILENAME)                    
                case 's':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, fileName, cur_size);
                    break;
#endif                    
                case 'f':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, funcName, cur_size);
                    break;    
                case 'm':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, "\n", cur_size);
                    break;                    
            }

            state = NORMAL;
        }
    }

    switch (outLevel)
    {
        case BBLOG_OUTPUT_CONSOLE:
            P_BBLOG_LAYOUT_Print(BBLOG_LEVEL_TRACE, result);
            fflush(stdout);
            break;
        case BBLOG_OUTPUT_FILE:
            fprintf(logWriter->fp, "%s", result);
            fflush(logWriter->fp);
            break;
        case BBLOG_OUTPUT_MAX:
            P_BBLOG_LAYOUT_Print(BBLOG_LEVEL_TRACE, result);
            fprintf(logWriter->fp, "%s", result);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
    }

    if (result != buf)
    {
        free(result);
    }

    return BBERR_OK;

}

static int P_BBLOG_LAYOUT_LogExit(struct LogWriter *pLogWriter, const BBLOG_OUTPUT_e outLevel,
                            char buf[], size_t bufSize,
                            const char *logFmt, const char *dateFmt, const char *timeFmt,
                            const char *fileName, const char *funcName, const int lineNo)
{
    enum
    {
        NORMAL,
        SUBST
    } state             = NORMAL;
    time_t t            = time(NULL);
    struct tm *lt       = localtime(&t);
    size_t logFmtLen    = strlen(logFmt);
    size_t i;
    size_t cur_size     = bufSize;    
    char *result        = buf;

    char *level         = "Leave";
    result[0]           = 0;    

    fileName = P_BBLOG_LAYOUT_BaseName(fileName);
    funcName = P_BBLOG_LAYOUT_BaseName(funcName);

    bblog_layout_t *logWriter = (bblog_layout_t *)pLogWriter;

    if (!logWriter || !logWriter->fp)
    {
        PR_ERR("[%s:%d]Invalid params.\n", __FUNCTION__, __LINE__);
        return BBERR_ERROR;
    }

    for (i = 0; i < logFmtLen; ++i)
    {
        if (state == NORMAL)
        {
            if (logFmt[i] == '%')
            {
                state = SUBST;
            }
            else
            {
                char str[2] = {0,};
                str[0] = logFmt[i];
                cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, str, cur_size);
            }
        }
        else
        {
            switch (logFmt[i])
            {
                case '%':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, "%", cur_size);
                    break;
                case 't':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, timeFmt, cur_size);
                    break;
                case 'd':
                    cur_size = P_BBLOG_LAYOUT_AppendTime(&result, buf, lt, dateFmt, cur_size);
                    break;
                case 'l':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, level, cur_size);
                    break;
                case 'n':
                    cur_size = P_BBLOG_LAYOUT_AppendInt(&result, buf, lineNo, cur_size);
                    break;
#if defined (CONFIG_BBLOG_DEBUG_FILENAME)                    
                case 's':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, fileName, cur_size);
                    break;
#endif                    
                case 'f':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, funcName, cur_size);
                    break;     
                case 'm':
                    cur_size = P_BBLOG_LAYOUT_AppendStr(&result, buf, "\n", cur_size);
                    break;                        
            }

            state = NORMAL;

        }
    }

    switch (outLevel)
    {
        case BBLOG_OUTPUT_CONSOLE:
            P_BBLOG_LAYOUT_Print(BBLOG_LEVEL_TRACE, result);
            fflush(stdout);
            break;
        case BBLOG_OUTPUT_FILE:
            fprintf(logWriter->fp, "%s", result);
            fflush(logWriter->fp);
            break;
        case BBLOG_OUTPUT_MAX:
            P_BBLOG_LAYOUT_Print(BBLOG_LEVEL_TRACE, result);
            fprintf(logWriter->fp, "%s", result);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
    }

    if (result != buf)
    {
        free(result);
    }

    return BBERR_OK;
}

static int P_BBLOG_LAYOUT_LogPrint(struct LogWriter *pLogWriter,
                            const BBLOG_LEVEL_e logLevel, const BBLOG_OUTPUT_e outLevel, const char *message)
{
    bblog_layout_t *logWriter = (bblog_layout_t *)pLogWriter;

    if (!logWriter || !logWriter->fp)
    {
        PR_ERR("[%s:%d]Invalid params.\n", __FUNCTION__, __LINE__);
        return BBERR_ERROR;
    }

    switch (outLevel)
    {
        case BBLOG_OUTPUT_CONSOLE:
            P_BBLOG_LAYOUT_Print(logLevel, message);
            fflush(stdout);
            break;
        case BBLOG_OUTPUT_FILE:
            fprintf(logWriter->fp, "%s", message);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
        case BBLOG_OUTPUT_MAX:
            P_BBLOG_LAYOUT_Print(logLevel, message);
            fprintf(logWriter->fp, "%s", message);
            fflush(stdout);
            fflush(logWriter->fp);
            break;
    }

    return BBERR_OK;

}

#define ___GLOBAL_FUNCTION___________________________________________________
int     BBLOG_LAYOUT_Init(LogWriter_t **logWriter)
{
    BBERR_e             bbErr   = BBERR_OK;

    if (!logWriter)
    {
        PR_ERR("[%s:%d]Invalid params.\n", __FUNCTION__, __LINE__);

        return BBERR_ERROR;
    }

    /* deinitialize the file logger if already initialized. */
    if (s_tBBLogLayout.fp)
    {
        LogWriter_t *_logWriter = (LogWriter_t *)&s_tBBLogLayout;
        BBLOG_LAYOUT_DeInit(_logWriter);
    }

    s_tBBLogLayout.fp = fopen(BBLOG_DEFAULT_FILE_NAME, "a");
    if (!s_tBBLogLayout.fp)
    {
        PR_ERR("[%s:%d]Could not open log file:%s\n", __FUNCTION__, __LINE__, BBLOG_DEFAULT_FILE_NAME);

        return BBERR_ERROR;
    }

    fprintf(s_tBBLogLayout.fp,"\n----- Logging Started -----\n");

    *logWriter = (LogWriter_t *)&s_tBBLogLayout;

    return bbErr;
}

void    BBLOG_LAYOUT_DeInit(LogWriter_t *logWriter)
{
    bblog_layout_t *layout = (bblog_layout_t *)logWriter;

    if (layout && layout->fp)
    {
        fclose(layout->fp);
    }
}
/*********************** End of File ***************************************/


