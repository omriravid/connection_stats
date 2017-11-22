#
# Created on: 22 Nov 2017
# Author: Omri Ravid
# 
# This makefile is used to build the libconnstat library
# It creates a .dll file (Windows) named libconnstat.dll

TARGET_NAME = libconnstat
TARGET = $(TARGET_NAME).dll
CC = gcc
LINKER = CC

SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
BIN_DIR = bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
INC_FILES := $(wildcard $(INC_DIR)/*.h)
OBJ_FILES := $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BIN_FILES := $(wildcard $(BIN_DIR)/*)

# Define compilation & Linker flags (link also the curl lib)
LFLAGS   = -Wall -I. -lm -lcurl
CFLAGS   = -Wall -I.
# Creates shared object
LDFLAGS  = -shared

# Link (all object files with the CURL lib)
$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	$(info $(TARGET_NAME): Linker- Start..)
	@$(LINKER) $(OBJ_FILES) $(LFLAGS) $(LDFLAGS) -o $@
	$(info $(TARGET_NAME): Linker- Done!)
	$(info $(TARGET_NAME): $(TARGET) Succesfully created)

# Compile
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(info $(TARGET_NAME): Compiling $<)
	@$(CC) $(CFLAGS) -c $< -o $@


.PHONY: rebuild clean remove

rebuild: clean $(BIN_DIR)/$(TARGET)

clean:
	@rm -f $(OBJ_FILES)
	$(info $(TARGET_NAME): obj files removed) 	

remove: clean
	@rm -f $(BIN_FILES)
	$(info $(TARGET_NAME): bin files [.dll] removed) 	
