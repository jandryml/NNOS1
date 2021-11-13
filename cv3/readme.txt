Kritická sekce – SW + test-and-set a synchronizace – bariéra (body do zkoušky: 10, termín odevzdání: 23. 11. 2021 13.00)
Modifikujte příklad <bank_withdraw.c> tak, aby se kritická sekce ošetřila softwarovým algoritmem používajícím
jednu sdílenou proměnnou locked (nastavenou na false pro volnou KS a na true při obsazené KS).
    - Toto SW řešení samozřejmě nebude korektní a vlákna mohou skončit současně v KS. Vytvořte proto ještě
      modifikaci, která využije instrukci typu test-and-set, konkrétně implementovanou pomocí instrukce xchg;
      použijte pro tento účel připravený hlavičkový soubor test_and_set_bool.h.
    - Pro obě varianty vytvořte ještě modifikaci aktivního čekání: místo prázdného příkazu volejte v cyklu sched_yield(2).
    - Dále nastavte synchronní start všech vláken ve funkci sync_threads().
    - Pozn.: Sdílené proměnné deklarujte jako volatile.
    - Volání sched_yield(2) je dostupné, pouze pokud je v hlavičkovém souboru unistd.h definovaná konstanta
      (makro) _POSIX_PRIORITY_SCHEDULING. Aby byl program korektní a přenositelný, měli byste kontrolovat, zda je
      konstanta definovaná, nicméně do hodnocení to zahrnovat nebudu, takže ji nekontrolujte. Návratová hodnota
      sched_yield(2) na Linuxu je vždy nulová (úspěch), takže tu také nekontrolujte. Příklad použití najdete
      v Peterson_sched.c.
Programy pojmenujte <bank_withdraw_SW1.c>, <bank_withdraw_SW1_sched.c>, <bank_withdraw_xchg.c> a
<bank_withdraw_xchg_sched.c>. Odevzdejte všechny čtyři zdrojové kódy včetně hlavičkového souboru a souboru <Makefile>.