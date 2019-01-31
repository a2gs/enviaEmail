CC_FLAGS = -Wall -g
LIBS=-lcrypto
INC=-I./includes
SRC=./src
BIN=./bin
CC=gcc

all: clean compile

compile:
	$(CC) -o $(BIN)/envia_email $(SRC)/envia_email.c $(SRC)/ee_utils.c $(CC_FLAGS) $(LIBS) $(INC)

clean:
	rm -rf $(BIN)/*
