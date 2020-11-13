V =
ifeq ($(strip $(V)), )
	Q=@
else
	Q=
endif

ifneq ($(BB_CREATE_LIB),)
-include obj.lst
-include so.lst
endif

ifneq ($(BB_OBJ),)
obj-y := $(BB_OBJ)
so-y := $(BB_OBJ)
endif

ifneq ($(BB_BUILD_DIR),)
include $(BB_BUILD_DIR)Makefile
endif

include $(TOP_MAKE_DIR)/Toolset.mak

BB_MULTI_JOBS := -j8

BB_MAKEFLAGS="$(BB_MULTI_JOBS) $(MAKEFLAGS)"

sub-dir-y := $(addprefix $(BB_BUILD_DIR), $(filter %/, $(obj-y)))
sub-so-dir-y := $(addprefix $(BB_BUILD_DIR), $(filter %/, $(so-y)))
sub-obj-y := $(addprefix $(BB_BUILD_DIR), $(filter %.o, $(obj-y))) $(fullpath-obj-y)
sub-so-y := $(addprefix $(BB_BUILD_DIR), $(filter %.o, $(so-y)))
sub-target-y := $(filter-out %.o, $(filter-out %/, $(obj-y)))
sub-so-target-y := $(filter-out %.o, $(filter-out %/, $(so-y)))

sub-extra-lib-y := $(addprefix $(BB_BUILD_DIR), $(extra-lib-y))

obj-list += $(sub-obj-y)

ifneq ($(LIB_TYPE), SHARED)
build_sub_dir: $(sub-dir-y) $(sub-obj-y) $(sub-target-y) add_obj_list_file FORCE
	@echo $(sub-dir-y) $(sub-obj-y) $(sub-target-y)
else
build_sub_dir: $(sub-so-dir-y) $(sub-so-y) $(sub-so-target-y) add_so_list_file FORCE
	@echo $(sub-so-dir-y) $(sub-so-y) $(sub-so-target-y)
endif

static-library: make_obj_list_file build_sub_dir FORCE
	$(Q)$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak BB_OBJ= BB_BUILD_DIR= BB_CREATE_LIB=y lib$(LIBNAME).a

shared-library: make_so_list_file build_sub_dir FORCE
	$(Q)$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak BB_OBJ= BB_BUILD_DIR= BB_CREATE_LIB=y lib$(LIBNAME).so

build_dir: make_obj_list_file build_sub_dir FORCE

make_obj_list_file: FORCE
	$(Q)$(BB_ECHO) "BB_OBJLIST := \\" > obj.lst

make_so_list_file: FORCE
	$(Q)$(BB_ECHO) "BB_SOLIST := \\" > so.lst

add_obj_list_file:
	$(Q)$(BB_ECHO) $(sub-obj-y)\\ >> obj.lst
	$(Q)$(BB_ECHO) $(sub-extra-lib-y)\\ >> obj.lst

add_so_list_file:
	$(Q)$(BB_ECHO) $(sub-so-y)\\ >> so.lst
	$(Q)$(BB_ECHO) $(sub-extra-lib-y)\\ >> so.lst

clean_sub_dir: $(sub-dir-y) FORCE
	$(Q)$(BB_RM) $(wildcard $(BB_BUILD_DIR)*.o $(BB_BUILD_DIR)*.d)

clean_so_sub_dir: $(sub-so-dir-y) FORCE
	$(Q)$(BB_RM) $(wildcard $(BB_BUILD_DIR)*.o $(BB_BUILD_DIR)*.d)

clean: clean_sub_dir FORCE
	$(Q)$(BB_RM) obj.lst
	$(Q)$(BB_RM) $(notdir $(CURDIR)).a

so_clean: clean_so_sub_dir FORCE
	$(Q)$(BB_RM) so.lst
	$(Q)$(BB_RM) $(notdir $(CURDIR)).so

%/: FORCE
ifeq ($(findstring clean,$(MAKECMDGOALS)),)
	$(Q)$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak LIB_TYPE=$(LIB_TYPE) MAKEFLAGS=$(BB_MAKEFLAGS) BB_OBJ= BB_BUILD_DIR=$@ build_sub_dir
else
ifneq ($(LIB_TYPE), SHARED)
	$(Q)$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak LIB_TYPE=$(LIB_TYPE) BB_OBJ= BB_BUILD_DIR=$@ clean_sub_dir
else
	$(Q)$(BB_MAKE) -f $(TOP_MAKE_DIR)/Rule.mak LIB_TYPE=$(LIB_TYPE) BB_OBJ= BB_BUILD_DIR=$@ clean_so_sub_dir
endif
endif

include $(TOP_MAKE_DIR)/Dependency.mak

COLOR_COMPILE ?= \x1b[33;01m
COLOR_WARNING ?= \x1b[32;01m
COLOR_ERROR   ?= \x1b[31;01m
COLOR_NONE    ?= \x1b[0m

%.o: %.c
	$(BB_ECHO) [Compiling ---------- $@]
ifneq ($(LIB_TYPE), SHARED)
	$(Q)$(BB_CC) $(BB_CFLAG) $< -c -o $@ -MF $(@:%.o=%.d) -MT $@ 2>${filter %.o,${subst /, ,$@}}.log || touch ${filter %.o,${subst /, ,$@}}.err;
else
	$(Q)$(BB_CC) $(BB_CFLAG) -DCONFIG_SHARED_COMPILE $< -c -o $@ -MF $(@:%.o=%.d) -MT $@ 2>${filter %.o,${subst /, ,$@}}.log || touch ${filter %.o,${subst /, ,$@}}.err;
endif
	@$(BB_SED) -e 's/.*\//     /' -e 's/error/$(COLOR_ERROR)&$(COLOR_NONE)/g' -e 's/warning/$(COLOR_WARNING)&$(COLOR_NONE)/g' ${filter %.o,${subst /, ,$@}}.log
	@if test -e ${filter %.o,${subst /, ,$@}}.err; then $(BB_RM) -f ${filter %.o,${subst /, ,$@}}.log ${filter %.o,${subst /, ,$@}}.err; exit 1; fi;
	@$(BB_RM) -f ${filter %.o,${subst /, ,$@}}.log ${filter %.o,${subst /, ,$@}}.err
	
%.a: $(BB_OBJLIST) FORCE
	$(Q)$(BB_AR) $(BB_AR_FLAGS) $@ $(BB_OBJLIST)
	$(Q)$(BB_CP) $@ $(TOP_OUTPUT_DIR)/lib

%.so: $(BB_SOLIST) FORCE
	$(Q)$(BB_CC) $(BB_CFLAGS) -shared -o $@ $(BB_SOLIST)
	$(Q)$(BB_CP) $@ $(TOP_OUTPUT_DIR)/lib

FORCE:

