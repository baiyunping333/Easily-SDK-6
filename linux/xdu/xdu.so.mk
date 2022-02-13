CC = gcc
CFLAGS = -g -Wall -fPIC

SRV_PATH = /usr/local/xService
LNK_PATH = /usr/local/lib
SYS_PATH = /usr/include

INC_PATH = ../../include
SRC_PATH = ../../xdu
SUB_PATH = ../../xdu/linux
OUT_PATH = ~/Easily-app-6/linux/sbin/api

VER = 6.0
LIBS = -lm -ldl -lutil -lrt -lX11 -lcairo -L $(LNK_PATH) -lxdk
DIRS = $(wildcard $(SRC_PATH)/*.c $(SUB_PATH)/*.c)
SRCS = $(notdir $(DIRS))
OBJS = $(patsubst %.c, %.o, $(SRCS))
MODULE = libxdu.so
TARGET = $(OUT_PATH)/$(MODULE).$(VER)

%.o : $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(SYS_PATH) -I $(INC_PATH) -I $(SRC_PATH)

%.o : $(SUB_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(SYS_PATH) -I $(INC_PATH) -I $(SRC_PATH)

all : $(OBJS)
	rm -f $@
	$(CC) -shared -fPIC -o $(TARGET) $(OBJS) $(LIBS)
	rm -f $(OBJS)

test:
	@echo $(DIRS)
	@echo $(SRCS)
	@echo $(OBJS)

install:
	if ! test -d $(SRV_PATH); then \
	sudo mkdir $(SRV_PATH); \
	fi
	if ! test -d $(SRV_PATH)/api; then \
	sudo mkdir $(SRV_PATH)/api; \
	fi

	sudo cp -f $(TARGET) $(SRV_PATH)/api;
	sudo chmod +x $(SRV_PATH)/api/$(MODULE).$(VER);
	sudo rm -f $(LNK_PATH)/$(MODULE)*;
	sudo ln -bs $(SRV_PATH)/api/$(MODULE).$(VER) $(LNK_PATH)/$(MODULE);

uninstall:
	sudo rm -r $(LNK_PATH)/$(MODULE)*;
	sudo rm -f $(SRV_PATH)/api/$(MODULE).$(VER)

.PHONY : clean
clean:
	-rm -f $(OBJS)
