LIBS=-lcrypto
INC=-I./includes
SRC=./src
BIN=./bin

CC=gcc
CC_FLAGS = -Wall -g -std=c11 -D_XOPEN_SOURCE=700

RM = rm -rf

all: clean compile

compile:
	$(CC) -o $(BIN)/envia_email $(SRC)/envia_email.c $(SRC)/ee_utils.c $(CC_FLAGS) $(LIBS) $(INC)

clean:
	-$(RM) $(BIN)/*
