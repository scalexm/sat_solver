#A C++ SAT solver

* pour compiler, exécuter `make`
* pour compiler avec les messages de debug, exécuter `make debug`
* pour exécuter les tests unitaires par défaut, exécuter `./bin/tests`
* pour exécuter les tests unitaires relatifs à la factorisation d'entiers, exécuter `./bin/tests [factor]`
* pour générer un problème SAT sous forme CNF encodant la factorisation d'un entier, exécuter `./bin/tests --gen nombre_sur_64_bits <output_name>` (le paramètre `output_name` est facultatif)
* pour utiliser le solveur en mode CNF, exécuter `./bin/resol file.cnf`
* pour utiliser le solveur en mode convivial, exécuter `./bin/resol -tseitin file.for`

## Fonctionnalités
* solveur utilisant l'algorithme DPLL pour satisfaire des formules sous forme CNF
* heuristiques: LINEAR (les paris sont faits dans l'ordre d'apparition des variables), RAND, MOMS, DLIS,
VSIDS
* watched litterals
* apprentissage de clauses (avec mode interactif)
* oubli de clauses
* parsing de formules logiques sous forme générale
* transformation de Tseitin
* traduction de problèmes de factorisation d'entiers en problèmes SAT

## Remarques générales
Nous avons décidé de supprimer définitivement les deux pratiques suivantes:
* suppression des tautologies en pré-traitement
* polarité unique

En effet, aucune de ces deux pratiques n'apportaient un gain significatif en termes
d'efficacité (en fait c'était même pire dans tous les cas, sans réelle possibilité d'optimiser).
Dans un souci de simplicité, nous avons donc décidé de les supprimer.

## Options de la ligne de commande
Toutes les options qui suivent peuvent être activées sur les exécutables `resol` et `tests`.
À noter que pour l'instant, pour utiliser les options sur l'exécutable `tests`, il faut les écrire
en majuscule.

## Heuristiques
Les différentes heuristiques peuvent être activées via les options suivantes:
* par défaut: LINEAR
* `-rand`: RAND
* `-moms`: MOMS (incompatible avec watched litterals)
* `-dlis`: DLIS (incompatible avec watched litterals)
* `-vsids`: VSIDS (uniquement avec clause learning)

Seule la dernière option d'heuristique de la ligne d'arguments sera prise en compte.

## Watched litterals
Les watched litterals peuvent être activés via l'option `-wl`.
Avec les watched litterals, il est impossible d'utiliser les heuristiques MOMS et DLIS, car celles-ci ont
besoin de savoir exactement quelles sont les clauses déjà satisfaites, et le caractère paresseux des
watched litterals empêche d'obtenir cette information.

## Clause learning
L'apprentissage de clauses peut être activé via l'option `-cl` ou bien en mode interactif `-cl-interac`.
Il fonctionne aussi bien avec les watched litterals que sans. Nous avons eu quelques difficultés à
obtenir une version optimisée du clause learning et avons dû modifier quelques structures de données,
en particulier nous utilisons désormais une `std::list` pour stocker les clauses, car le fait d'ajouter
des clauses en cours de route entraîne des allocations mémoire et le fait d'utiliser un `std::vector`
pouvait invalider les pointeurs vers les clauses déjà existantes.

Au début la construction se faisait avec des mariages successifs de la clause apprise courante avec
la dernière clause "responsable" trouvée. Cette méthode entraînait beaucoup d'allocations et était
peu performante, nous l'avons donc en quelques sortes "compactifiée": on part d'une clause vide
qu'on fait grandir au fur et à mesure (en utilisant les mêmes règles que pour le mariage), et on
s'arrête comme pour les mariages quand il n'y a plus qu'un littéral du niveau courant (l'UIP) dans
la dernière clause responsable.

Nous visons le premier UIP comme demandé dans le bonus, en cherchant le littéral du niveau courant
affecté le plus récemment lors de la résolution. Notons que ceci nous a finalement permis de simplifier
encore la construction précédente.

## VSIDS
Pour l'heuristique VSIDS, à chaque fois qu'un littéral apparaît dans une clause apprise, on ajoute
1 à son score. On divise les scores par 2 tous les 100 conflits. Le littéral choisi est celui qui
a le score le plus grand (on utilise une file de priorité qu'on tient à jour lors du learning).

Notons que VSIDS est la seule heuristique qui nous permette d'utiliser une file de priorité de façon
efficace, car la mise à jour de la file de priorité ne se fait que pendant l'analyse de conflit (qui
ne représente pas beaucoup de temps d'exécution comparé au temps total). Les autres heuristiques
obligeraient à maintenir une file de priorité pendant la phase de propagation et cela serait beaucoup
trop coûteux.

## Oubli de clauses
L'oubli de clauses peut être activé via l'option `-forget`.
À chaque fois qu'une clause apprise intervient dans l'analyse de conflit, on incrémente son score
de 1. Dès que le nombre de clauses apprises dépasse une certaine borne (pour l'instant cette borne
vaut 1/3 du total des clauses initiales, ceci est totalement arbitraire et pourra être configuré à
l'avenir), on supprime la moitié des clauses apprises (arbitraire aussi) en choisissant les clauses
de plus petit score (donc les clauses les moins actives).

## Mode interactif (clause learning)
Le mode interactif du clause learning peut être activé via l'option `-cl-interac`. Lors d'un conflit,
les commandes disponibles sont:
* g pour obtenir le graphe de conflit: celui-ci sera écrit dans un fichier appelé `conflict.dot`
* c pour passer au conflit suivant
* t pour sortir du mode interactif

## Fichiers de tests
Des fichiers CNF de tests se trouvent dans le dossier `cnf_files`. Tous ces fichiers sont associés
à des problèmes concrets. Il y a des traductions de problèmes de factorisation en problèmes SAT: les fichiers
`prime1.cnf` à `prime5.cnf` sont associés à des nombres premiers et sont non
satisfiables tandis que les fichiers `comp1.cnf` à `comp5.cnf` sont associés à des nombres
composés et sont satisfiables. Il y a également divers fichiers provenant du site
http://www.cs.ubc.ca/~hoos/SATLIB/benchm.html.

## Structuration du code
* Le dossier `solver` constitue le coeur du projet.

    Le code du solveur lui-même est contenu dans `detail/clause.hpp`, `detail/clause.cpp`,
    `detail/solver.hpp`, `solver.hpp`, `solver.cpp`, `guess.cpp`, `deduce.cpp`,
    `backtrack.cpp`, `learn.cpp` et `interac.cpp`.

    Le sous-dossier `expr` contient le code relatif au traitement des formules logiques conviviales. La structure des expressions et les opérations sur celles-ci sont contenues dans les fichiers `expr/detail/logical_expr.hpp`, `expr/logical_expr.hpp`, `expr/logical_expr.cpp`, `expr/detail/tseitin.hpp`, `expr/tseitin.hpp` et `expr/tseitin.cpp`.

    Le code pour parser des expressions avec Flex et Bison se trouve dans les fichiers `expr/detail/logical_parser.ypp`, `expr/detail/logical_scanner.lpp`, `expr/detail/logical_driver.hpp` et `expr/detail/logical_driver.cpp`.

    Nous avons choisi une solution basée sur `boost::variant` pour représenter les expressions sous forme arborescente plutôt qu'une solution basée sur le polymorphisme virtuel qui est lent et source de nombreuses erreurs.

* Le dossier `tests` contient les tests unitaires du projet. Ce sont surtout des tests servant à vérifier le bon fonctionnement de plusieurs cas particuliers. Il permet aussi de générer automatiquement des problèmes associés à la factorisation de nombres entiers (sur 64 bits seulement pour l'instant, cf `factor.cpp` et `gen_tests.cpp`) pour tester le solveur plus en profondeur et pour pouvoir faire du profiling.

* Le dossier `sat` contient le code de l'interface en ligne de commande.

## Structures de données utilisées
Pour obtenir les meilleures performances possibles, nous avons concentré nos efforts dans la réduction des allocations mémoire
lors de la phase de propagation. Ainsi, presque toutes les structures utilisées sont des `std::vector` ayant
été si possible pré-alloués lors de la construction de l'instance du solveur. Cela permet aussi d'avoir des structures qui sont contiguës en mémoire, ceci est très important pour éviter au maximum les cache misses quand on parcout ces structures (et on les parcourt très souvent). Ceci nous oblige par contre à renuméroter les variables pour qu'elles soient représentées par des entiers
consécutifs, nous avons donc une table de conversion "anciennes variables" <-> "nouvelles variables" utilisées pour le debug et
pour le résultat donné en sortie. Notons l'utilisation d'une `std::list` pour les clauses comme expliqué plus haut
(section Clause learning).

## Observations sur les performances
Nous avons inclus dans le dossier `cnf_files` un rapport de performances sur les différents fichiers. Ces fichiers sont associés à des problèmes
concrets. On note que les combinaisons d'heuristiques qui s'en sortent le mieux en moyenne sur l'ensemble des fichiers sont:
* -cl -wl -rand
* -cl -wl -rand -forget
* -cl -wl -vsids
* -cl -wl -vsids -forget

De même dans le dossier `scripts`, il y a des courbes de performance au format PNG sur un grand nombre d'instances
3-SAT générées aléatoirement. Les courbes couvrent l'ensemble des combinaisons d'heuristiques possibles.
Ici, ce sont les combinaisons contenant les heuristiques `-moms` ou `-dlis` qui sont les plus efficaces.

Les tests ont été effectués sur un MacBook Pro avec un processeur Intel Core i7 à 3.1 GHz.

Notons que sur la plupart des problèmes concrets et à heuristiques égales (-cl -wl -vsids -forget),
notre solveur tient tête à MiniSat. Cependant sur les problèmes 3-SAT générés aléatoirement, MiniSat
est meilleur que toutes nos combinaisons d'heuristiques.

## Répartition du travail
* parsing des expressions logiques: Alexandre
* transformation de Tseitin: Nicolas
* tests unitaires: Alexandre
* DPLL: Alexandre et Nicolas
* prétraitement de l'entrée: Nicolas
* heuristiques MOMS, DLIS et RAND: Alexandre et Nicolas
* watched litterals: Alexandre
* scripts pour mesurer les performances: Nicolas
* clause learning: Alexandre
* mode interactif du clause learning: Nicolas
* heuristiques VSIDS et Oubli: Alexandre
