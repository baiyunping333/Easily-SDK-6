CC = g++
CFLAGS = -g -Wall

LIB_PATH = /usr/local/lib
API_PATH = ~/Easily-sdk-6/linux/bin
INC_PATH = ~/Easily-sdk-6/include
SRC_PATH = ~/Easily-sdk-6/test/xdc_frame_test
OBJ_PATH = ~/Easily-sdk-6/test/xdc_frame_test
OUT_PATH = ~/Easily-app-6/linux/bin

SRCS = $(SRC_PATH)/main.cc
OBJS = $(patsubst %.cc, %.o, $(SRCS))
TARGET = $(OUT_PATH)/xdc_frame_test

$(OBJ_PATH)/%.o : $(SRC_PATH)/%.cc
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

all : $(OBJS)
	rm -f $@
	$(CC) -o $(TARGET) $(OBJS) -L $(LIB_PATH) -lxdk -lxdu -lxdl -lxdc
	rm -f $(OBJS)

test:
	@echo $(SRCS)
	@echo $(OBJS)

.PHONY : clean
clean:
	-rm -f $(OBJS)
