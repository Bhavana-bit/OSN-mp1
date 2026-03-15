# Compiler
CC = gcc
CFLAGS = -Wall -g
LIBS = -lcrypto

# Files
CLIENT_SRC = client.c
SERVER_SRC = server.c
HEADER = sham.h

# Executables
CLIENT_EXE = client
SERVER_EXE = server

# Default target
all: $(CLIENT_EXE) $(SERVER_EXE)

# Build client
$(CLIENT_EXE): $(CLIENT_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(CLIENT_EXE) $(CLIENT_SRC)

# Build server
$(SERVER_EXE): $(SERVER_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(SERVER_EXE) $(SERVER_SRC) $(LIBS)

# Clean build files
clean:
	rm -f $(CLIENT_EXE) $(SERVER_EXE) *.o