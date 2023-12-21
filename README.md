# Codename-Plage: TP SEA - USER MODE

Pour ce projet, nous avons terminé les challenges 1, 2, 3 et 4.

Le code source du processus tracé se trouve dans le fichier src/prog.c:
- La fonction à optimiser est exponentiation_long_long
- La fonction answer l'appelle un certains nombre de fois est elle est elle même lancé plusieurs fois et est chronométré à chaque fois
- La fonction appellé dans le challenge 2 est la fonction foo
- Le programme est compilé en statique

Le code source du processus traçant se trouve dans le fichier src/challenges.c.
- Il est découpé en trois fonctions: challenge1, pour le challenge 1, challenge2, et challenge3_4, pour le challenge 3 et 4.
- Il contient quelques fonction utilitaire pour récupérer l'addresse d'une fonction donnée, de même pour le pid du programme donné.

La fonction optimisé se trouve dans le fichier src/function_optimized.c.
- On en extrait le code assembleur par `objdump -d build/function_optimized.o | awk '/<fast_exponentiation_long_long>:/,/^$/' | grep -v '^$'`

## Compiler le projet

Pour compiler le projet, il suffit de lancer le script `compil.sh` à la racine (`./compil.sh`).

Cela va compiler les trois programmes: 2 éxecutables, 1 pour le tracé et 1 pour le traceur, et 1 fichier objet pour la fonction optimisée.

Note: l'executable de prog.c s'appelle `prog_to_run`

## Lancer le projet

Pour lancer le projet, il faut tout d'abord se placer dans la racine du projet.

Ensuite, on peut lancer dans un premier terminal le premier programme: `./build/prog_to_run`

Et dans un autre, on peut lancer le programme traçant par: `./build/challenges prog_to_run`
Par défaut, le processus trançant se lance sur le challenge 3 et 4, mais on peut ajouter un argument en plus pour choisir le challenge 1 ou 2 (ou 3 pour 3 et 4):
- `./build/challenges prog_to_run 1`: challenge 1
- `./build/challenges prog_to_run 2`: challenge 2
- `./build/challenges prog_to_run 3`: challenge 3 et 4

Une fois le programme traçant lancé, celui-ci par réalisé les opérations demandé dans le challenge donné:
1) Challenge 1: stop le tracé avec un trap
2) Challenge 2: attrape le programme par un trap, lui fait appeler une fonction avec un argument qui est un pointeur, l'attrape après et vérifie les résultats (y compris la modification du pointeur par la fonction), puis restaure l'execution correcte du programme avant d'être mis en pause
3) Challenge 3 et 4: 
    - attrape le programmme
    - alloue de l'espace sur la HEAP pour y écrire la version optimisé de la fonction avec `posix_memalign`
    - donne le droit d'éxecution sur cet espace mémoire alloué avec `mprotect`
    - écrit le code de la fonction optimisé à cet endroit
    - restauration des instructions éventuellement écrasée (dans notre cas, vu la taille de la fonction, ce n'est pas utile mais nous l'avons tout de même fait pour s'entrainer aux bonnes pratiques)
    - écriture d'un trampoline (jump) pour remplacer l'exécution de la fonction non-optimisé avec l'éxecution de la fonction optimisé que l'on vient d'écrire en mémoire
    - on fait reprendre le processus là où on l'avait mis en pause, à savoir l'appel de cette fonction

On pourra voir que une fois le programme lancé et terminé, le temps pris par programme tracé par appelle de answer va être accéléré (d'un facteur 10 sur nos machines).
