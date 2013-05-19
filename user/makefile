# executed on command-prompt <MinGW GCC 3.9.5>

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS  = -Wall -mwindows -s
#LDFLAGS  += -mwindows
INCLUDES =
LIBS = -lcomctl32
LIB =
DEF =

TARGET_EXE = ptviewer.exe
OBJS_EXE = main.o driver_loader.o gui_viewer.o paging.o hook_gui_viewer.o hook_angler.o hook_caller.o hook_share.o kernel_interface.o pae_paging.o simple_list.o register_viewer.o

TARGET_DLL = hook_ptviewer.dll
OBJS_DLL =  hook_main.o hook_share.o driver_loader.o kernel_interface.o

TARGET_RES = ptv.res
OBJS_RES =

TARGET = $(TARGET_EXE) $(TARGET_DLL)
OBJS = $(OBJS_EXE) $(OBJS_DLL)

all : $(TARGET)

$(TARGET_EXE) : $(OBJS_EXE) $(LIB) $(TARGET_RES)
	$(CC) $(LDFLAGS) -o $@ $(OBJS_EXE) $(TARGET_RES) $(LIBS) $(LIB)

$(TARGET_DLL) : $(OBJS_DLL) $(OBJS_RES)
	dllwrap -k --driver-name $(CC) $(LDFLAGS) -o $@ $(OBJS_DLL) $(OBJS_RES) $(LIBS)
# dllwrap -k -def $(DEF) --driver-name $(CC) $(LDFLAGS) -o $@ $(OBJS_DLL) $(OBJS_RES) $(LIBS)

$(LIB) : $(DEF) $(TARGET_DLL)
	dlltool -k -d $(DEF) -l $@

clean :
	del $(TARGET) $(OBJS)

.SUFFIXES : .exe .c .o .rc .res .def .h

.c.o :
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.rc.res : 
	windres --language=0411 $< -O coff -o $@

main.o:  main.h
driver_loader.o:  driver_loader.h
gui_viewer.o:  gui_viewer.h hook_gui_viewer.h paging.h pae_paging.h
paging.o:  paging.h kernel_interface.h simple_list.h
kernel_interface.o:  kernel_interface.h
pae_paging.o:  pae_paging.h kernel_interface.h simple_list.h
simple_list.o:  simple_list.h
hook_angler.o: hook_angler.h
hook_main.o: hook_main.h hook_share.h
hook_share.o: hook_share.h
hook_caller.o: hook_caller.h hook_angler.h hook_share.h
hook_gui_viewer.o: hook_gui_viewer.h hook_caller.h
