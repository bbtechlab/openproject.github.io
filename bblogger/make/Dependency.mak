BB_CFLAG += -MMD

BB_DEPENDENCY_FILE := $(wildcard $(sub-obj-y:.o=.d))

-include $(BB_DEPENDENCY_FILE)
