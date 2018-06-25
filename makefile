CC=g++
INCLUDE_DIR=include
OBJ_DIR=build
SRC_DIR=src
FLAGS=-Wall -I$(INCLUDE_DIR) -std=c++11
objects=CPU.o PPU.o NES.o main.o cart.o
OBJS=$(foreach obj,$(objects),$(OBJ_DIR)/$(obj))

vpath %.cpp $(SRC_DIR)
vpath %.h $(INCLUDE_DIR)
vpath %.o $(OBJ_DIR)

NESemu: $(OBJS)
	$(CC) $(FLAGS) -o NESemu $(OBJS)
$(OBJ_DIR)/CPU.o : CPU.cpp CPU.h PPU.h cart.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/CPU.cpp -o $@
$(OBJ_DIR)/PPU.o : PPU.cpp PPU.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/PPU.cpp -o $@
$(OBJ_DIR)/NES.o : NES.cpp NES.h CPU.h PPU.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/NES.cpp -o $@
$(OBJ_DIR)/main.o : main.cpp NES.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/main.cpp -o $@
$(OBJ_DIR)/cart.o : cart.cpp cart.h
	$(CC) $(FLAGS) -c $(SRC_DIR)/cart.cpp -o $@
clean:
	rm build/*.o NESemu
