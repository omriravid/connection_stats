#
# Created on: 22 Nov 2017
# Author: Omri Ravid
# 
# This makefile is used to run connstat_runner executable (after linking it with libconnstat library)
# After running 'make' you can run the executable with:
#      ./bin/connstat_runner.exe
# for example: 
#      ./bin/connstat_runner.exe -n 4 -H "Keep-Alive: 300" -H "Connection: keep-alive"


LIB_CONNSTAT_NAME = libconnstat
LIB_CONNSTAT_DIR = ./../$(LIB_CONNSTAT_NAME)

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BIN_FILES := $(wildcard $(BIN_DIR)/*)

# Executable target
TARGET_NAME = connstat_runner
TARGET = $(TARGET_NAME)
CC = gcc
LINKER = CC
CFLAGS   = -Wall -I.
LFLAGS   = -Wall -I. -I$(LIB_CONNSTAT_DIR)/inc -I./libs -lm -lconnstat

# Link all obj files together with the libconnstat library
$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(info $(TARGET_NAME): Linker- Start..)
	@$(LINKER) $(OBJ_FILES) $(LFLAGS) -o $@
	$(info $(TARGET_NAME): Linker- Done!)
	$(info $(TARGET_NAME): $(TARGET) executable succesfully created)

# Compile all C files, both for the runner and the libconnstat library
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@cd $(LIB_CONNSTAT_DIR) && $(MAKE)
	@cp ../$(LIB_CONNSTAT_NAME)/bin/$(LIB_CONNSTAT_NAME).dll ./$(BIN_DIR)
	$(info $(TARGET_NAME): Compiling $<)
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

# Clean all obj files and binaries
clean:
	@cd $(LIB_CONNSTAT_DIR) && $(MAKE) remove
	@rm -f $(OBJ_FILES)
	$(info $(TARGET_NAME): obj files removed) 	
	@rm -f $(BIN_FILES)
	$(info $(TARGET_NAME): bin files [executable] removed) 	
