# executed on command-prompt <MinGW GCC 3.9.5>

CC = gcc
C++ = g++
CFLAGS = -Wall -O2
LDFLAGS  = -Wall -s
INCLUDES =
LIBS =
TARGET_EXE = cui_ptv.exe
TARGET = $(TARGET_EXE)
TARGET_RES =
OBJS_EXE = cui_main.o cui_viewer.o ../driver_loader.o ../kernel_interface.o ../hook_angler.o ../paging.o ../pae_paging.o ../simple_list.o ../hook_caller.o ../hook_share.o
OBJS = $(OBJS_EXE)

all : $(TARGET)

$(TARGET_EXE) : $(OBJS_EXE) $(LIB) $(TARGET_RES)
	$(CC) $(LDFLAGS) -o $@ $(OBJS_EXE) $(TARGET_RES) $(LIBS) $(LIB)

clean :
	del $(TARGET) $(OBJS)

.SUFFIXES : .exe .cpp .c .o .rc .res .def .h

.c.o :
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.cpp.o :
	$(C++) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.rc.res : 
	windres --language=0411 $< -O coff -o $@

main.o:  main.h
