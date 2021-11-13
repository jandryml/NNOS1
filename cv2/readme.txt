Dealokace prostředků vlákna (body do zkoušky: 5, termín odevzdání: 23. 11. 2021 13.00)
Modifikujte příklad <pthread_cleanup_sem.c> tak, aby se automaticky dealokovaly prostředky v něm alokované i při
jeho explicitním ukončení. K tomuto účelu použijte pthread_cleanup_push(3). Dealokace nealokovaných prostředků
bude považována za chybu. Opravte také zjevnou chybu v programu. Program pojmenujte <pthread_cleanup.c>.