CC=g++
INCLUDE_DIR=include
OBJ_DIR=build
SRC_DIR=src
FLAGS=-g -Wall -I$(INCLUDE_DIR) -std=c++11 -I/usr/include/SDL2 -D_REENTRANT -L/usr/lib/x86_64-linux-gnu -lSDL2 -O0
objects=CPU.o PPU.o APU.o NES.o controller.o main.o cart.o allsuite_cart.o cart000.o cart001.o
OBJS=$(foreach obj,$(objects),$(OBJ_DIR)/$(obj))

vpath %.cpp $(SRC_DIR)
vpath %.h $(INCLUDE_DIR)
vpath %.o $(OBJ_DIR)

NESemu: $(OBJS)
	$(CC) -o NESemu $(OBJS) $(FLAGS)
$(OBJ_DIR)/CPU.o : make_dir CPU.cpp CPU.h PPU.h APU.h cart.h controller.h types.h constants.h
	$(CC) -c $(SRC_DIR)/CPU.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/PPU.o : make_dir PPU.cpp PPU.h types.h constants.h
	$(CC) -c $(SRC_DIR)/PPU.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/APU.o : make_dir APU.cpp APU.h types.h constants.h
	$(CC) -c $(SRC_DIR)/APU.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/NES.o : make_dir NES.cpp NES.h CPU.h PPU.h APU.h controller.h types.h constants.h
	$(CC) -c $(SRC_DIR)/NES.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/controller.o : make_dir controller.cpp controller.h types.h constants.h
	$(CC) -c $(SRC_DIR)/controller.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/main.o : make_dir main.cpp NES.h types.h constants.h
	$(CC) -c $(SRC_DIR)/main.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/cart.o : make_dir cart.cpp cart.h types.h constants.h
	$(CC) -c $(SRC_DIR)/cart.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/allsuite_cart.o : make_dir carts/allsuite_cart.cpp cart.h types.h constants.h
	$(CC) -c $(SRC_DIR)/carts/allsuite_cart.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/cart000.o : make_dir carts/cart000.cpp cart.h types.h
	$(CC) -c $(SRC_DIR)/carts/cart000.cpp -o $@ $(FLAGS)
$(OBJ_DIR)/cart001.o : make_dir carts/cart001.cpp cart.h types.h
	$(CC) -c $(SRC_DIR)/carts/cart001.cpp -o $@ $(FLAGS)
make_dir:
	mkdir -p $(OBJ_DIR)
clean:
	rm -f build/*.o NESemu
