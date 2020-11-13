#
# Copyright (C) 2018 BBTech Lab
#

# define SHELL

LC_ALL		= C
LANG		= C
export SHELL	= /usr/bin/env bash

# Toolchain
export BB_TOOLCHAIN_PATH=

# define base dir path
export CONFIG_OS_LINUX=y
export TOP_MAKE_DIR=$(shell pwd)
export TOP_WORK_DIR=$(subst /make,,$(TOP_MAKE_DIR))
export TOP_OUTPUT_DIR=$(TOP_MAKE_DIR)/output
USER=$(shell echo $(shell whoami) | sed 's/[ ]//g')

.PHONY:

TARGETS :=

# include sub-makefiles for libraries
include $(TOP_MAKE_DIR)/Toolset.mak
include $(TOP_MAKE_DIR)/Makefile.lib
include $(TOP_MAKE_DIR)/Makefile.app

# include TARGETS and use clean distclean on activate TARGETS
reverse = $(if $1,$(call reverse,$(wordlist 2,$(words $1),$1)) $(firstword $1))
TARGETS_CLEAN = $(call reverse, $(patsubst %,%-clean,$(filter-out $(IGNORE),$(TARGETS))))
TARGETS_DISTCLEAN = $(call reverse, $(patsubst %,%-distclean,$(filter-out $(IGNORE),$(TARGETS))))

world: prepare-directories show-build
	$(BB_MAKE) $(TARGETS)
	
show-build:
	@echo 
	@echo ============================
	@echo 
	@echo Building Start !!!!
	@echo 
	@echo Building orders : prepare-directories $(TARGETS) 
	@echo 
	@echo ============================	
	
prepare-directories:
	@if [ ! -d $(TOP_MAKE_DIR)/output ]; then \
		mkdir -p $(TOP_MAKE_DIR)/output; \
	fi
	
	@if [ ! -d $(TOP_OUTPUT_DIR)/bin ]; then \
		mkdir -p $(TOP_OUTPUT_DIR)/bin; \
	fi
	
	@if [ ! -d $(TOP_OUTPUT_DIR)/lib ]; then \
		mkdir -p $(TOP_OUTPUT_DIR)/lib; \
	fi			
	
clean: $(TARGETS_CLEAN)
	@echo clean!!

distclean: $(TARGETS_DISTCLEAN)	
	@echo distclean!!