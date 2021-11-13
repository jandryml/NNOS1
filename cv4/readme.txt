Implementace semaforu s čítačem – mutex a podmínková proměnná (body do zkoušky: 6, termín odevzdání: 30. 11. 2021 13.00)
Modifikujte hlavičkový soubor <pthread_sem-template.h> tak, že implementujete semafor se záporným čítačem
pomocí mutexu a podmínkové proměnné posixových vláken.
    - Nad datovou strukturou semaforu pt_sem_t (s čítačem, mutexem a podmínkovou proměnnou) budou definovány
      operace init, destroy, wait a post (vše s prefixem pt_sem_).
    - Návratové hodnoty funkcí: nula = bez chyb, 1 = chyba mutexu, 2 = chyba podmínkové proměnné, 3 =
      přetečení/podtečení čítače.
    - Při opouštění funkcí nastavte errno na návratovou hodnotu příslušného (neúspěšného) volání pro mutex či
      podmínkovou proměnnou, při přetečení/podtečení ji nastavte na hodnotu EOVERFLOW.
Hlavičkový soubor pojmenujte <pthread_sem.h>. Odevzdejte jej společně s testovacím programem <test_pt_sem.c>.