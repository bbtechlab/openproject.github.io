BB_VERBOSE :=
BB_CROSS :=

BB_CFLAG += $(BB_EXTRA_CFLAG)
BB_CFLAG += $(BB_DEFAULT_CFLAG)

BB_ECHO := @echo
BB_MAKE := $(BB_VERBOSE)make
BB_CC := $(BB_VERBOSE)$(BB_CROSS)gcc
BB_AR := $(BB_VERBOSE)$(BB_CROSS)ar
BB_LD := $(BB_VERBOSE)$(BB_CROSS)ld
BB_STRIP := $(BB_VERBOSE)$(BB_CROSS)strip
BB_SED := $(BB_VERBOSE)sed
BB_CP := $(BB_VERBOSE)cp
BB_RM := $(BB_VERBOSE)rm -rf
BB_MV := $(BB_VERBOSE)mv
BB_MKDIR := $(BB_VERBOSE)mkdir -p
BB_LN := $(BB_VERBOSE)ln
BB_OBJCP := $(BB_VERBOSE)$(BB_CROSS)objcopy