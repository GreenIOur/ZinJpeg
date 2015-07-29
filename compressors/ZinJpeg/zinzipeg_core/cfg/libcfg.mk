## Check the Console library selection
ifeq ($(findstring __LIB_JPEG__,$(LIB_OPT)) , __LIB_JPEG__)
INCLUDE_JPEG = YES
endif

## Check if it's selected all libs inclusion
ifeq ($(findstring __BUILD_ALL_LIBS__,$(EEOPT)) , __BUILD_ALL_LIBS__)
INCLUDE_JPEG = YES
endif

##
## If the library is required
##
ifeq ($(INCLUDE_JPEG), YES)

##
## Library code
##

## Add the inc path to the include pathlist
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

## Add each file individually
EE_SRCS_JPEG +=

## If the file list is not empty, create the jpeg lib
ifneq ($(EE_SRCS_JPEG),)
EE_OBJS_JPEG := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o,$(patsubst %.S,%.o, $(EE_SRCS_JPEG))))
LIBSRCS += $(EE_SRCS_JPEG)
libjpeg.a: $(EE_OBJS_JPEG)
	@echo $(EE_SRC_JPEG)
	@printf "AR  libjpeg.a\n" ;
	$(QUIET)$(EE_AR) rs libjpeg.a $(EE_OBJS_JPEG)
##
## Add the library to the linker list and list of lib files
##
OPT_LIBS += -ljpeg
ALL_LIBS += libjpeg.a
endif

endif
