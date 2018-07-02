CC=g++
INCLUDE_DIR=include
OBJ_DIR=build
SRC_DIR=src
FLAGS=-g -Wall -I$(INCLUDE_DIR) -std=c++11 -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib/x86_64-linux-gnu -lSDL2
objects=CPU.o PPU.o NES.o main.o cart.o
OBJS=$(foreach obj,$(objects),$(OBJ_DIR)/$(obj))

vpath %.cpp $(SRC_DIR)
vpath %.h $(INCLUDE_DIR)
vpath %.o $(OBJ_DIR)

NESemu: $(OBJS)
	$(CC) -o NESemu $(OBJS) $(FLAGS)
$(OBJ_DIR)/CPU.o : CPU.cpp CPU.h PPU.h cart.h
	$(CC) -c $(SRC_DIR)/CPU.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/PPU.o : PPU.cpp PPU.h
	$(CC) -c $(SRC_DIR)/PPU.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/NES.o : NES.cpp NES.h CPU.h PPU.h
	$(CC) -c $(SRC_DIR)/NES.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/main.o : main.cpp NES.h
	$(CC) -c $(SRC_DIR)/main.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/cart.o : cart.cpp cart.h
	$(CC) -c $(SRC_DIR)/cart.cpp -o $@ $(FLAGS)
clean:
	rm build/*.o NESemu
