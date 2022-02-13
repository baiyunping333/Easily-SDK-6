CC = gcc
CFLAGS = -g -Wall -fPIC

VER = 6.0
SRV_PATH = /usr/local/xService
LNK_PATH = /usr/local/lib

INC_PATH = ../../include
SRC_PATH = ../../xdk
OUT_PATH = ~/Easily-app-6/linux/sbin/api

LIBS = -lm -ldl -lutil -lrt
DIRS = $(wildcard \
		$(SRC_PATH)/linux/*.c \
		$(SRC_PATH)/acp/*.c \
		$(SRC_PATH)/crypt/*.c \
		$(SRC_PATH)/encode/*.c \
		$(SRC_PATH)/geo/*.c \
		$(SRC_PATH)/img/*.c \
		$(SRC_PATH)/imp/*.c \
		$(SRC_PATH)/lua/*.c \
		$(SRC_PATH)/net/*.c \
		$(SRC_PATH)/nmath/*.c \
		$(SRC_PATH)/tdb/*.c \
		$(SRC_PATH)/zlib/*.c \
		$(SRC_PATH)/util/*.c \
		$(SRC_PATH)/*.c)
SRCS = $(notdir $(DIRS))
OBJS = $(patsubst %.c, %.o, $(SRCS))
MODULE = libxdk.so
TARGET = $(OUT_PATH)/$(MODULE).$(VER)

%.o : $(SRC_PATH)/linux/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/acp/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/crypt/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/encode/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/geo/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/img/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/imp/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/lua/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/net/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/nmath/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/tdb/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/zlib/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/util/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

%.o : $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_PATH)

all : $(OBJS)
	rm -f $@
	$(CC) -shared -fPIC -pthread -o $(TARGET) $(OBJS) $(LIBS)
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
