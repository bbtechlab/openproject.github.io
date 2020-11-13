#
# Copyright (C) 2018 BBTech Lab. http://bbtechlab.com/
#

# 

##################  BBLOGGER TESTAPP S/W  ####################

TARGETS += bblog_testapp

# bblogger test app dir path
BBLOG_TESTAPP_DIR=$(TOP_WORK_DIR)/apps/liblogger
BBLOG_TESTAPP_MAKE_DIR=$(TOP_WORK_DIR)/apps/liblogger

BBLOG_TESTAPP_COMPILE_FLAGS=CONFIG_TOOLCHAIN_PATH=$(CONFIG_TOOLCHAIN_PATH) \
	TOP_WORK_DIR=$(TOP_WORK_DIR) \
	TOP_MAKE_DIR=$(TOP_MAKE_DIR) \
	TOP_OUTPUT_DIR=$(TOP_OUTPUT_DIR)

define build_bblog_testapp
	$(BB_MAKE) -C $(BBLOG_TESTAPP_MAKE_DIR) $(BB_TESTAPP_COMPILE_FLAGS) $(1)
endef

bblog_testapp:
	$(call build_bblog_testapp, build)
	$(call build_bblog_testapp, link)

bblog_testapp-clean:
	$(call build_bblog_testapp, clean)

bblog_testapp-distclean: bblog_testapp-clean

