/*
 ============================================================================
 Filename     : bbliblogger_cmd.c
 Version      : 1.0
 Created      : 15/04/2018 22:33:18
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "bbliberr.h"
#include "bbliblogger.h"

BBLOG_DEBUG_LEVEL("BBLOG", BBLOG_LEVEL_MSG, BBLOG_OUTPUT_MAX)

/* End Including Headers */

/***************************************************************************/
/****************************** define *************************************/
/***************************************************************************/
/* Start #define */
#define MAX_EVENTS  1024    /* Max. number of events to process at one go */
#define LEN_NAME    256     /* Assuming that the length of the filename won't exceed 256 bytes */
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /* size of one event */
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /* buffer to store the data of events */
/* End #define */

/***************************************************************************/
/*********************** Type Defination ***********************************/
/***************************************************************************/
/* Start typedef */
typedef struct bblog_cmd_file_monitor {
    /* This is the file descriptor for the inotify watch */
    int          inotify_fd;
    /* watch descriptor */
    int          inotify_wd;
    /* the number of watched items */
    unsigned int ulWatchedItems;
    /* thread id */
    pthread_t    thread_id;
    /* Status */
    unsigned int initialized;
} bblog_cmd_file_monitor_t;

typedef struct bblog_cmd_handler {
    char *tag_name;
    void (*func)(char **argv, ...);

} bblog_cmd_handler_t;
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
static bblog_cmd_file_monitor_t     s_tBBlogFileMonitor;
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
static int      P_BBLOG_InotifyAddWatch(int inotify_fd, const char *dirname, unsigned long mask);
static int      P_BBLOG_InotifyRemoveWd(int inotify_fd, int inotify_wd);
static int      P_BBLOG_CmdReceiving(const char *dirname, char buffer[BBLOG_FORMAT_LENGTH]);
static void     P_BBLOG_CmdAnalyzing(char *cmd);

static void*    P_BBLOG_ShowLogLevel(char *argv[]);
static void*    P_BBLOG_ChangeLevel(char *argv[]);
static void*    P_BBLOG_ChangeOutput(char *argv[]);
static void*    P_BBLOG_ChangeFormat(char *argv[], char *cmd);

static bblog_cmd_handler_t  s_tBBLogCmdHanlder[] = {
    {"show",    P_BBLOG_ShowLogLevel},
    {"level",   P_BBLOG_ChangeLevel},
    {"print",   P_BBLOG_ChangeOutput},
    {"fmt",     P_BBLOG_ChangeFormat},
    {NULL,      NULL}
};
/* End static functions */


/***************************************************************************/
/*********************** Function Description*******************************/
/***************************************************************************/
#define ___STATIC_FUNCTION___________________________________________________
static int P_BBLOG_CmdReceiving(const char *dirname, char buffer[BBLOG_FORMAT_LENGTH])
{
    FILE    *fp;

    if (dirname == NULL)
    {
        BBLOG_ERR("Invalid input params\n");
        return BBERR_INVAL;
    }

    fp = fopen(dirname, "r");
    if (fp == NULL)
    {
        BBLOG_ERR("Could not open file:%s\n", dirname);
        return BBERR_ERROR;
    }

    memset(buffer, 0, BBLOG_FORMAT_LENGTH);
    fgets(buffer, BBLOG_FORMAT_LENGTH - 1, fp);

    fclose(fp);

    return BBERR_OK;
}

static int P_BBLOG_InotifyAddWatch(int inotify_fd, const char *dirname, unsigned long mask)
{
    int wd;

    wd = inotify_add_watch (inotify_fd, dirname, mask);

    if (wd < 0)
    {
        BBLOG_ERR("Cannot add watch for \"%s\" with event mask %lX\n %s", dirname, mask, strerror(errno));
        fflush (stdout);
    }
    else
    {
        s_tBBlogFileMonitor.ulWatchedItems++;
        BBLOG_MSG("Watching %s WD=%d (items:%d)\n", dirname, wd, s_tBBlogFileMonitor.ulWatchedItems);
    }

    return wd;
}

static int P_BBLOG_InotifyRemoveWd(int inotify_fd, int inotify_wd)
{
    int ret;

    ret = inotify_rm_watch (inotify_fd, inotify_wd);

    if (ret < 0)
    {
        BBLOG_ERR("Can not inotify_rm_watch\n");
        return BBERR_ERROR;
    }
    else
    {
        s_tBBlogFileMonitor.ulWatchedItems--;
        return BBERR_OK;
    }

    return BBERR_OK;
}

static BBLOG_LEVEL_e P_BBLOG_ConvertLevel(char *level)

{
    if(!strcmp(level, "trace"))
        return  BBLOG_LEVEL_TRACE;
    else if(!strcmp(level, "dbg"))
        return BBLOG_LEVEL_DBG;
    else if(!strcmp(level, "msg"))
        return  BBLOG_LEVEL_MSG;
    else if(!strcmp(level, "wrn"))
        return BBLOG_LEVEL_WRN;
    else if(!strcmp(level, "err"))
        return  BBLOG_LEVEL_ERR;
    else
        BBLOG_WRN("Invalid log level");

    return 0;
}

static BBLOG_LEVEL_e P_BBLOG_ConvertOutput(char *level)

{
    if(!strcmp(level, "all"))
        return BBLOG_OUTPUT_MAX;
    else if(!strcmp(level, "console"))
        return BBLOG_OUTPUT_CONSOLE;
    else if(!strcmp(level, "file"))
        return BBLOG_OUTPUT_FILE;
    else
        BBLOG_WRN("Invalid log output\n");

    return 0;
}

static void* P_BBLOG_ShowLogLevel(char *argv[])
{
    HAPPY(argv);

    BBLOG_ShowLogLevel();
}

static void* P_BBLOG_ChangeLevel(char *argv[])
{
    if(argv[1] && argv[2])
    {
        BBLOG_LEVEL_e level = P_BBLOG_ConvertLevel(argv[1]);
        if(level)
        {
            BBLOG_MSG("Changing %s to level %s\n", argv[2], argv[1]);
            BBLOG_SetModuleLevel(argv[2], level);
        }
    }
}

static void* P_BBLOG_ChangeOutput(char *argv[])
{
    if(argv[1] && argv[2])
    {
        BBLOG_OUTPUT_e output = P_BBLOG_ConvertOutput(argv[1]);
        if(output)
        {
            BBLOG_MSG("Changing %s to output %s\n", argv[2], argv[1]);
            BBLOG_SetModuleOutput(argv[2], output);
        }
    }
}

static void* P_BBLOG_ChangeFormat(char *argv[], char *cmd)
{
    char *fmt = cmd + strlen("fmt:");
    if(fmt)
    {
        BBLOG_SetLayoutFormat(fmt);
    }
}

static void P_BBLOG_ExcuteCommand(char **argv, char *cmd)
{
    int i;
    for(i = 0; i < sizeof(s_tBBLogCmdHanlder)/sizeof(s_tBBLogCmdHanlder[0]); i++)
    {
        if(argv[0] && !strcmp(argv[0], s_tBBLogCmdHanlder[i].tag_name))
        {
            if(s_tBBLogCmdHanlder[i].func)
            {
                if(!strcmp(s_tBBLogCmdHanlder[i].tag_name, "fmt"))
                    s_tBBLogCmdHanlder[i].func(argv, cmd);
                else
                    s_tBBLogCmdHanlder[i].func(argv);
            }
            return;
        }
    }
    BBLOG_WRN("No command matching %s, len = %d", argv[0], strlen(argv[0]));
}

static void P_BBLOG_CmdAnalyzing(char *cmd)
{
    char s[strlen(cmd) +1];
    strcpy(s, cmd);
    const char delim[] = ":\n";
    char **argv = NULL;
    argv = (char**)malloc(BBLOG_FORMAT_LENGTH);
    char *token = strtok(cmd, delim);
    int i = 0;

    while(token != NULL)
    {

        argv[i++] = token;
        token = strtok(NULL, delim);
    }

    P_BBLOG_ExcuteCommand(argv, s);

    memset(argv, 0, BBLOG_FORMAT_LENGTH);
    free(argv);
}

static void *P_BBLOG_CMD_Task(void *params)
{
    char                    buffer[BUF_LEN];
    char                    *p = NULL;
    int                     byteRead = 0;
    struct inotify_event    *event;
    int                     cmd = 0;

    HAPPY(params);

    while(1)
    {
        memset(buffer, 0, BUF_LEN);
        byteRead = 0;
        cmd      = 0;
        byteRead = read(s_tBBlogFileMonitor.inotify_fd, buffer, BUF_LEN);
        if (byteRead > 0)
        {
            for (p = buffer; p < buffer + byteRead;)
            {
                event = (struct inotify_event *)p;
                if (event->mask & IN_CREATE)
                {
                    BBLOG_MSG( "CREATE: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_MODIFY)
                {
                    BBLOG_MSG( "MODIFY: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_DELETE)
                {
                    BBLOG_MSG( "DELETE: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_ATTRIB)
                {
                    BBLOG_MSG( "IN_ATTRIB: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_CLOSE_WRITE)
                {
                    BBLOG_MSG( "IN_CLOSE_WRITE: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_CLOSE_NOWRITE)
                {
                    BBLOG_MSG( "IN_CLOSE_NOWRITE: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (event->mask & IN_OPEN)
                {
                    BBLOG_MSG( "IN_OPEN: %s \"%s\" on WD #%i\n", (event->mask & IN_ISDIR)? "Dir":"File", event->name, event->wd );
                    cmd = 1;
                }
                if (1 == cmd)
                {
                    /* Remove watching file */
                    (void)P_BBLOG_InotifyRemoveWd(s_tBBlogFileMonitor.inotify_fd, s_tBBlogFileMonitor.inotify_wd);
                    if (s_tBBlogFileMonitor.inotify_fd >= 0)
                    {
                        close(s_tBBlogFileMonitor.inotify_fd);
                    }
                    break;
                }
                p += sizeof(struct inotify_event) + event->len;
            }

            /* Read configuration files, analyzing command, the perform task accordingly */
            if (1 == cmd)
            {
                char cmBuf[BBLOG_FORMAT_LENGTH];
                P_BBLOG_CmdReceiving(BBLOG_DEFAULT_CONFIG_FILE_NAME, cmBuf);
                BBLOG_MSG("Received command: %s\n", cmBuf);

                P_BBLOG_CmdAnalyzing(cmBuf);

                /* Continue watching file */
                s_tBBlogFileMonitor.inotify_fd = -1;
                s_tBBlogFileMonitor.inotify_fd = inotify_init();
                if (s_tBBlogFileMonitor.inotify_fd < 0)
                {
                    BBLOG_WRN("Couldn't initialize inotify\n");
                }
                else
                {
                    s_tBBlogFileMonitor.inotify_wd = P_BBLOG_InotifyAddWatch(s_tBBlogFileMonitor.inotify_fd,
                                                                        BBLOG_DEFAULT_CONFIG_FILE_NAME,
                                                                        IN_ALL_EVENTS);
                }
            }
        }
        else
        {
            sleep(1);
        }
    }
}

#define ___GLOBAL_FUNCTION___________________________________________________
int BBLOG_CMD_Init(void)
{
    BBERR_e bbErr = BBERR_OK;

    if (s_tBBlogFileMonitor.initialized == 1)
    {
        return BBERR_OK;
    }

    s_tBBlogFileMonitor.inotify_fd = inotify_init();
    if (s_tBBlogFileMonitor.inotify_fd < 0)
    {
        BBLOG_ERR("Couldn't initialize inotify\n");
        return BBERR_ERROR;
    }

    s_tBBlogFileMonitor.ulWatchedItems  = 0;
    s_tBBlogFileMonitor.inotify_wd = P_BBLOG_InotifyAddWatch(s_tBBlogFileMonitor.inotify_fd,
                                                        BBLOG_DEFAULT_CONFIG_FILE_NAME,
                                                        IN_ALL_EVENTS);
    if (pthread_create(&s_tBBlogFileMonitor.thread_id, NULL, P_BBLOG_CMD_Task, NULL))
    {
        BBLOG_ERR("Couldn't create BBLOG file monitoring task.\n");
        return BBERR_ERROR;
    }

    s_tBBlogFileMonitor.initialized = 1;

    BBLOG_MSG("Initilized BBLOG file monitoring.!\n");

    return bbErr;
}

void BBLOG_CMD_DeInit(void)
{
    if (s_tBBlogFileMonitor.initialized == 1)
    {
        P_BBLOG_InotifyRemoveWd(s_tBBlogFileMonitor.inotify_fd, s_tBBlogFileMonitor.inotify_wd);
        close(s_tBBlogFileMonitor.inotify_fd);
        s_tBBlogFileMonitor.initialized = 0;
    }
}
/*********************** End of File ***************************************/


