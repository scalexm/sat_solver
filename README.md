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
* heuristiques: LINEAR (les paris sont faits dans l'ordre d'apparition des variables), RAND, MOMS, DLIS, WL (watched litterals)
* apprentissage de clauses
* parsing de formules logiques sous forme générale
* transformation de Tseitin
* traduction de problèmes de factorisation d'entiers en problèmes SAT

## Heuristiques
Les différentes heuristiques peuvent être activées sur les exécutables `resol` et `tests` via les options
suivantes:
* par défaut: LINEAR
* `-rand`: RAND
* `-moms`: MOMS
* `-dlis`: DLIS
* `-wl`: WL

Seule la dernière option d'heuristique de la ligne d'arguments sera prise en compte.
À noter que pour l'instant, pour utiliser les options sur l'exécutable `tests`, il faut écrire les options
en majuscule.

## Watched litterals
Avec les watched litterals, il est impossible d'utiliser les heuristiques MOMS et DLIS, car celles-ci ont
besoin de savoir exactement quelles sont les clauses déjà satisfaites, et le caractère paresseux des
watched litterals empêche d'obtenir cette information.
Par conséquent, l'heuristique utilisée quand les watched litterals sont activés est l'heuristique LINEAR.

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

## Fichiers de tests
Des fichiers CNF de tests se trouvent dans le dossier `cnf_files`. Il y a
des traductions de problèmes de factorisation en problèmes SAT: les fichiers
`prime1.cnf` à `prime5.cnf` sont associés à des nombres premiers et sont non
satisfiables tandis que les fichiers `comp1.cnf` à `comp5.cnf` sont associés à des nombres
composés et sont satisfiables. Il y a également divers fichiers provenant du site
http://www.cs.ubc.ca/~hoos/SATLIB/benchm.html.

## Structuration du code
* Le dossier `solver` constitue le coeur du projet.

    Le code du solveur lui-même est contenu dans `detail/clause.hpp`, `detail/clause.cpp`,
    `detail/solver.hpp`, `solver.hpp`, `solver.cpp`, `guess.cpp`, `deduce.cpp`,
    `backtrack.cpp` et `learn.cpp`.

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
Sur les fichiers présents dans `cnf_files`, les watched litterals obtiennent globalement les meilleures performances sauf pour les fichiers `comp2.cnf`, `dubois20.cnf`, `dubois21.cnf`, `dubois22.cnf`, `aim-100-1_6-no-1.cnf`.

Pour les dubois, l'heuristique LINEAR reste la meilleure.

`comp2.cnf` est un peu particulier car LINEAR n'arrive pas à le résoudre en un temps raisonnable et WL le résout en 14s,
mais MOMS et DLIS le résolvent tous les deux en moins de deux secondes (sur un MacBook Pro avec un processeur Intel
core i7 à 3.1GHz).

`aim-100-1_6-no-1.cnf` est résolu le plus rapidement par MOMS (1.2s en MOMS, 2.6s en WL, 5.8s en LINEAR).

Par contre sur des instances 3-SAT générées aléatoirement, MOMS et DLIS obtiennent en moyenne des performances
bien meilleures que WL, elle-même obtenant des performances meilleures que LINEAR et RAND.

Nous avons inclus dans le dossier `cnf_files` un rapport de performances sur les différents fichiers. De même dans le dossier
`scripts`, il y a plusieurs courbes de performance au format PNG sur un grand nombre d'instances 3-SAT générées aléatoirement.

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
