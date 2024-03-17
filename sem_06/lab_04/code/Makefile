all:
	make s
	make c

s: server.c mymsg.h
	gcc server.c -o s.out

c: client.c mymsg.h
	gcc client.c -o c.out

clean:
	rm *.out