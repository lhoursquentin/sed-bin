objs = address.o operations.o read.o sed-bin.o
bin = sed-bin

$(bin): $(objs)
	# the line below is implicit with GNU make, add it for BSD compatibility
	$(CC) $(CFLAGS) $(objs) -o $@

sed-bin.o: generated.c generated-init.c

.PHONY: clean

clean:
	rm -f *.o $(bin)
