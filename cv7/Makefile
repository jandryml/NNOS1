# compiler / kompilátor
CC = gcc

# compiler switches / přepínače pro kompilátor
CFLAGS = -Wall
# -Wall		warnings: all / vypisovat všechna varování
# -g		include debugging symbols / zahrnout symboly pro debugger
# -Dsymbol	define symbol like #define / definuje symbol jako #define
# -D_REENTRANT

# linker switches / přepínače pro linker
LDFLAGS =
# link libraries / knihovny pro linker
LDLIBS = 
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAMS = shmSV
INDIVIDUALLY =

all: $(PROGRAMS)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

testSV: shmSV
	@bash -c './shmSV $$RANDOM > key.txt & P1=$$! ; \
	sleep 1 ; \
	KEY="`cat key.txt`" ; \
	if [ -s key.txt ]; then \
		if expr "$$KEY" : "0x[0-9A-Fa-f]*$$" > /dev/null ; then \
			echo "Key to shared memory is: `cat key.txt`" ; \
		else \
			echo "There must be only a key on standard output of command shmSV," >&2 ; \
			echo "in the format: 0x########" >&2 ; \
			echo "If not, the test will fail." >&2 ; \
			echo "All other output messages should go to standard ERROR output." >&2 ; \
			kill $$P1 ; \
			exit 2 ; \
		fi ; \
	else \
		echo "The program did NOT print the key on standard output." >&2 ; \
		echo "Use “fflush(stdout);” after printing." >&2 ; \
		kill $$P1 ; \
		exit 1 ; \
	fi ; \
	echo ; LANG=C ipcs -m | egrep "^[^0]|^0x0*`cut -c3- key.txt`" ; echo ; \
	./shmSV $$RANDOM > /dev/null & P2=$$! ; \
	sleep 1 ; \
	echo ; LANG=C ipcs -m | egrep "^[^0]|^0x0*`cut -c3- key.txt`" ; echo ; \
	wait $$P1 ; \
	wait $$P2 ; \
	echo ; LANG=C ipcs -m | egrep "^[^0]|^0x0*`cut -c3- key.txt`" ; echo ; \
	rm -f key.txt'

show:
	LANG=C ipcs -m

clean_shm:
	@echo Removing shared memory segments from system.
	LANG=C ipcs -m | grep ^0 | tr -s ' ' '\t' | cut -f2 | xargs -rl ipcrm -m || true

rmm: clean_shm

clean: clean_shm
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS) $(INDIVIDUALLY)

