CC=gcc
LD=ld
CFLAGS= -fPIC -c
LFLAGS= -x --shared
FILES=pam_riddle.o pam_riddle.so riddle

.PHONY=all install clean deinstall remove test

all: pam_riddle.so

test: riddle

riddle: pam_riddle.c
	$(CC) -DTEST -g -o $@ $^

pam_riddle.so: pam_riddle.o
	$(LD) $(LFLAGS) $^ -o $@
pam_riddle.o: pam_riddle.c
	$(CC) $(CFLAGS) $^ -o $@

install:
	install -o root -g root -t /usr/lib/x86_64-linux-gnu/security/ pam_riddle.so

clean:
	rm -f $(FILES)

deinstall:

remove:
