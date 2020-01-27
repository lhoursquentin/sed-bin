objs = operations.o sed-bin.o
bin = sed-bin

$(bin): $(objs)

.PHONY: clean

clean:
	rm -f *.o $(bin)
