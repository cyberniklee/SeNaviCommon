################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Source/Timer/subdir.mk
-include Source/Time/subdir.mk
-include Source/Serialization/subdir.mk
-include Source/Parameter/subdir.mk
-include Source/Exception/subdir.mk
-include Source/Console/subdir.mk
-include Source/ConfigFile/subdir.mk
-include Source/Callbacks/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libSeNaviCommon.so

# Tool invocations
libSeNaviCommon.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	arm-openwrt-linux-muslgnueabi-g++ -shared -o "libSeNaviCommon.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(CC_DEPS)$(C++_DEPS)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS) libSeNaviCommon.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
