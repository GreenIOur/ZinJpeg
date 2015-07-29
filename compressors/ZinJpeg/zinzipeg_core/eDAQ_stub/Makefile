INC_DIR  	:= ../inc
SRC_DIR  	:= ../src
OBJ_DIR  	:= obj
DEST_LIB        := .

LIBRARY		:= libZinzi.a

CC 		:= gcc
CL		:= ar -cru
COPT		:= -O2 -Wall 
#-Werror
CFLAGS		:= -D__EDAQ__ -std=c99 -I $(INC_DIR)

VPATH		:= $(SRC_DIR)
SRC_FILES  	:= $(shell ls $(SRC_DIR)/*.c;)
OBJ_FILES  	:= ${SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o}

all:	
	make all_;

test:	
	EXT_DEFINES=-DDEBUG  make all_;


all_:	$(LIBRARY)

$(DEST_LIB)/$(LIBRARY): $(OBJ_FILES)
	@echo 
	@echo "---------------------------------------------"
	@echo "Creating uCV library: " $(LIBRARY)
	$(CL) $(DEST_LIB)/$(LIBRARY) $(OBJ_FILES)
	@echo Done!
	@echo "---------------------------------------------"
	@echo
	
install:
	@echo
	@echo "**************************************************"
	@echo -n "Installing library: " $(LIBRARY) " ..."
	@cp ./$(LIBRARY) ../../../lib/
	@echo " done"
	@echo "**************************************************"
	@echo 
	
clean:
	rm -fr $(OBJ_DIR)/*.o
	rm -fr $(DEST_LIB)/$(LIBRARY)
	
$(OBJ_DIR)/%.o : %.c $
	$(CC) $(COPT) -c $(EXT_DEFINES) $(CFLAGS) $< -o $@
