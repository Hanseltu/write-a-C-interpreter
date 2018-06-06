CC=gcc
CFLAGS = -m32 -g
xc-tutor: xc-tutor.o
	$(CC) $(CFLAGS) -o $@ $<
xc-tutor.o: xc-tutor.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -rf *.o xc-tutor
