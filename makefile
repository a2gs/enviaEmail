LIBS=-lcrypto
INC=-I./includes
SRC=./src
BIN=./bin

CC=gcc
CC_FLAGS = -g -Wall -std=c11 -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_POSIX_SOURCE=1 -D_DEFAULT_SOURCE=1 -D_GNU_SOURCE=1
#CC_FLAGS = -O2 -Wall -std=c11 -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -D_POSIX_SOURCE=1 -D_DEFAULT_SOURCE=1 -D_GNU_SOURCE=1

RM = rm -rf

all: clean compile

compile:
	$(CC) -o $(BIN)/envia_email $(SRC)/envia_email.c $(SRC)/ee_utils.c $(CC_FLAGS) $(LIBS) $(INC)

clean:
	-$(RM) $(BIN)/*
