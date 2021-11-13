# compiler / kompilátor
CC = gcc

# compiler switches / přepínače pro kompilátor
CFLAGS = -Wall -D_REENTRANT
# -Wall		warnings: all / vypisovat všechna varování
# -g		include debugging symbols / zahrnout symboly pro debugger
# -Dsymbol	define symbol like #define / definuje symbol jako #define
# -D_REENTRANT

# linker switches / přepínače pro linker
LDFLAGS =
# link libraries / knihovny pro linker
LDLIBS = -lpthread
# -llibrary / -lknihovna
#  libNAME.so.version	filename of the library / jméno souboru knihovny
# -lNAME
# e.g. / např.: -lpthread	link with libpthread.so / připojit knihovnu libpthread.so

# for semaphores and POSIX message queues / pro semafory a posixové fronty zpráv
%_sem %_semN %_msgPOSIX %_semPOSIX %_mqPOSIX cpu_% %_CPUtime: LDLIBS += -lrt


RM = /bin/rm -f

OBJECTS = *.o
BACKUPS = *~ *.bak
PROGRAMS = bank_withdraw_SW1 bank_withdraw_SW1_sched bank_withdraw_xchg bank_withdraw_xchg_sched original working
# thread_add_Peterson_xchg bank_deposit
# bank_transactions_Peterson2
INDIVIDUALLY = bank_withdraw_SW1 bank_withdraw_SW1_sched bank_withdraw_xchg  bank_withdraw_xchg_sched original
TEMPLATES = cpu_time_measuring cpu_time_measuring2 cpu_time_measuring2_arg bank_deposit_CPUtime

all: $(PROGRAMS)

solo: $(INDIVIDUALLY)

#target/cíl: dependencies (sources) / závislosti (zdrojové kódy)
#	commands to create target (program) / příkazy pro vytvoření cíle (programu)

clean:
	@echo Deleting objects, backups and programs / Mažu objekty, zálohy a programy
	$(RM) $(OBJECTS) $(BACKUPS) $(PROGRAMS) $(INDIVIDUALLY) $(TEMPLATES)

