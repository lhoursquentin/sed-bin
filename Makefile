objs = address.o operations.o read.o sed-bin.o
bin = sed-bin

$(bin): $(objs)

sed-bin.o: generated.c generated-init.c

.PHONY: clean

clean:
	rm -f *.o $(bin)
