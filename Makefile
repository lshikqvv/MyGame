# compiler settings
CC = g++
CFLAGS = -Wall -g
LIBS = `pkg-config --cflags --libs gtkmm-3.0`

SERVER_PROG = Server
CLIENT_PROG = Client
ARRAY_PROG  = game/arrays
CARD_PROG   = game/Card
DECK_PROG   = game/Deck
HAND_PROG   = game/Hand
SOCKET_PROG = connection/Socket
# WINDOW_PROG = window_app/Window_app
SERVER_OBJECTS = server.o socket.o card.o deck.o hand.o array.o
CLIENT_OBJECTS = client.o socket.o
BIN = ./build/bin
OBJ = ./build/obj
SRC = ./src
BINPROGS = $(addprefix $(BIN)/, $(SERVER_PROG) $(CLIENT_PROG))

all: $(BINPROGS)

clean:
	rm -f $(BIN)/* $(OBJ)/*.o

# compile objects
$(OBJ)/server.o : $(SRC)/$(SERVER_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/client.o : $(SRC)/$(CLIENT_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/array.o : $(SRC)/$(ARRAY_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/card.o : $(SRC)/$(CARD_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/deck.o : $(SRC)/$(DECK_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/hand.o : $(SRC)/$(HAND_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/socket.o : $(SRC)/$(SOCKET_PROG).cc
	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJ)/window_app.o : $(SRC)/$(WINDOW_PROG).cc
# 	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

# link server program
$(BIN)/$(SERVER_PROG): $(addprefix $(OBJ)/, $(SERVER_OBJECTS))
	$(LINK.o) $^ $(LDLIBS) -o $@

# link client program
$(BIN)/$(CLIENT_PROG): $(addprefix $(OBJ)/, $(CLIENT_OBJECTS))
	$(LINK.o) $^ $(LDLIBS) -o $@
