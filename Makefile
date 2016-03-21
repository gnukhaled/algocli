# Makefile to build the cli interface for 
# the AlgoBackup project

# the compiler: gcc for C program
CC = gcc

# Compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#  -fstack-protector-all turns on gcc stack protection 
#  -lreadline invokes libreadline
CFLAGS  = -g -O2 -Wall -fstack-protector-all -lreadline -lpam -lpam_misc -lsqlite3 -lsmartcols
CFLAGS_OBJS = -g -c
# The build target executable:

TARGET = algocli
OBJECTS = main.o algocli.o algo-log.o registry.o\
	  cmd-system.o cmd-fs.o cmd-bp.o \
	  cmd-accessctrl.o cmd-log.o cmd-nfs.o\
	  cmd-net.o cmd-user.o cmd-snapshot.o\
	  cmd-repl.o cmd-config.o cmd-cifs.o\
	  cmd-disk.o cmd-syshealth.o 

all: $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c
	$(CC) $(CFLAGS_OBJS) main.c

algocli.o: algocli.c
	$(CC) $(CFLAGS_OBJS) algocli.c

algo-log.o: algo-log.c
	$(CC) $(CFLAGS_OBJS) algo-log.c

registry.o: registry.c
	$(CC) $(CFLAGS_OBJS) registry.c

cmd-fs.o: cmd-fs.c
	$(CC) $(CFLAGS_OBJS) cmd-fs.c

cmd-log.o: cmd-log.c
	$(CC) $(CFLAGS_OBJS) cmd-log.c

cmd-bp.o: cmd-bp.c
	$(CC) $(CFLAGS_OBJS) cmd-bp.c

cmd-nfs.o: cmd-nfs.c
	$(CC) $(CFLAGS_OBJS) cmd-nfs.c

cmd-system.o: cmd-system.c
	$(CC) $(CFLAGS_OBJS) cmd-system.c

cmd-accessctrl.o: cmd-accessctrl.c
	$(CC) $(CFLAGS_OBJS) cmd-accessctrl.c

cmd-cifs.o: cmd-cifs.c
	$(CC) $(CFLAGS_OBJS) cmd-cifs.c

cmd-disk.o: cmd-disk.c
	$(CC) $(CFLAGS_OBJS) cmd-disk.c
		
cmd-snapshot.o: cmd-snapshot.c
	$(CC) $(CFLAGS_OBJS) cmd-snapshot.c
		
cmd-config.o: cmd-config.c
	$(CC) $(CFLAGS_OBJS) cmd-config.c
		
cmd-net.o: cmd-net.c
	$(CC) $(CFLAGS_OBJS) cmd-net.c
		
cmd-repl.o: cmd-repl.c
	$(CC) $(CFLAGS_OBJS) cmd-repl.c
		
cmd-syshealth.o: cmd-syshealth.c
	$(CC) $(CFLAGS_OBJS) cmd-syshealth.c

cmd-user.o: cmd-user.c
	$(CC) $(CFLAGS_OBJS) cmd-user.c

clean:
	@echo "Cleaning"
	$(RM) $(TARGET) *.o
