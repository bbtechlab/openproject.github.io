#
# Objects
#
obj-y :=
obj-y += expliblogger_main.o
obj-y += expliblogger_warn.o
obj-y += expliblogger_trace.o
obj-y += expliblogger_message.o
obj-y += expliblogger_error.o
obj-y += expliblogger_debug.o

# get the path info
include $(TOP_MAKE_DIR)/Toolset.mak

#
# Include 
#
inccflag-y := -I$(TOP_WORK_DIR)/liblogger/include/
inccflag-y += -I$(TOP_WORK_DIR)/libutils/include/
inccflag-y += -I$(TOP_WORK_DIR)/libos/include/

#
# Linklist
#
LIBLIST := $(TOP_OUTPUT_DIR)/lib/libbblogger.so
LIBLIST += -lpthread -lrt

#
# Extra flags
#
BB_EXTRA_CFLAG := $(inccflag-y)
BB_EXTRA_CFLAG += -D_GNU_SOURCE=1 -DCONFIG_BBLOG_DEBUG=1 -DCONFIG_BBLOG_DEBUG_FILENAME=1
BB_EXTRA_LDFLAG +=
export BB_EXTRA_CFLAG
export BB_EXTRA_LDFLAG

-include obj.lst

build:
	$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak BB_OBJ='$(obj-y)' build_dir
	
link:
	$(BB_CC) -o $(TOP_OUTPUT_DIR)/bin/bblog_testapp $(BB_OBJLIST) $(LIBLIST) $(BB_EXTRA_CFLAG) $(BB_EXTRA_LDFLAG)
	
clean:
	$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak BB_OBJ='$(obj-y)' clean	
