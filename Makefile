OBJS=tmp/bridge.o
#SRCS=$(OBJS:%.o=%.c)

LIB_OBJS=tmp/log.o tmp/daemon.o tmp/socket.o tmp/util.o tmp/packet.o
#LIB_SRCS=$(LIB_OBJS:tmp/%.o=../netlib/%.c)

CFLAGS=-g -Wall -I./netlib/ -I./src/
TARGET=bridge

$(TARGET): $(LIB_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(LIB_OBJS) $(OBJS)

tmp/bridge.o: ./src/bridge.c
	$(CC) $(CFLAGS) -o tmp/bridge.o -c ./src/bridge.c

tmp/log.o: ./netlib/log.c
	mkdir -p tmp/
	$(CC) $(CFLAGS) -o tmp/log.o -c ./netlib/log.c

tmp/daemon.o: ./netlib/daemon.c
	$(CC) $(CFLAGS) -o tmp/daemon.o -c ./netlib/daemon.c

tmp/socket.o: ./netlib/socket.c
	$(CC) $(CFLAGS) -o tmp/socket.o -c ./netlib/socket.c

tmp/util.o: ./netlib/util.c
	$(CC) $(CFLAGS) -o tmp/util.o -c ./netlib/util.c

tmp/packet.o: ./netlib/packet.c
	$(CC) $(CFLAGS) -o tmp/packet.o -c ./netlib/packet.c


clean:
	-rm -f bridge
	-rm -rf ./tmp/
	-rm -f bridge.log