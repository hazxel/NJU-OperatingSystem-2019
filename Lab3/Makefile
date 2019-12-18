CC = cc

TARGET = main

OBJS = my_syscall.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

my_syscall.o:

.PHONY : clean cleanobj
clean:
	-rm syscall $(OBJS)
cleanobj:
	-rm $(OBJS)
