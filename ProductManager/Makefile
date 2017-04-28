.PHONY : util
.SUFFIXES : .c .o

TARGET = goodManager.exe

OBJS = goodsList.o goodsManager.o util.o


CC = gcc
CFLAGS = -pthread
CFLAGS += -Wall
LD = gcc
LDFAGS = -pthread

CFLAGS += -g -O0

#기본 제공되는 라이브러리에 대한 설정
UTIL_DIR = ../util
CFLAGS += -I$(UTIL_DIR)/include
LDFLAGS += -L$(UTIL_DIR)/lib
LDFLAGS += -lgthread-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl

#기본 BUILD RULE 설정
all: $(TARGET)

$(TARGET): $(OBJS)
	  $(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)
clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
.c.o:
	$(CC) -c $< -o $@ $(CFLAGS)
