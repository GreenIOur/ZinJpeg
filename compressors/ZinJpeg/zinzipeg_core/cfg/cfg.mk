
## Check the jpeg library selection
ifeq ($(findstring __LIB_JPEG__,$(LIB_OPT)),__LIB_JPEG__)

##
## Application files
##
EE_SRCS_JPEG_COMMON := $(addprefix contrib/jpeg/src/, $(notdir $(shell jpeg -1 $(EEBASE)/contrib/jpeg/src/*.c)))
EE_SRCS += $(EE_SRCS_JPEG_COMMON)

ifeq ($(findstring __COSMIC__,$(EEOPT)), __COSMIC__)
ALLINCPATH += -i"$(shell cygpath -w $(EEBASE)/contrib/jpeg/inc)"
else
ifeq ($(findstring __RTD_CYGWIN__,$(EEOPT)), __RTD_CYGWIN__) 
ALLINCPATH += -I"$(shell cygpath -w $(EEBASE)/contrib/jpeg/inc)"
else
ALLINCPATH += -I$(EEBASE)/contrib/jpeg/inc
endif
endif

INCLUDE_PATH += $(EEBASE)/contrib/jpeg/inc
endif # __LIB_JPEG__
