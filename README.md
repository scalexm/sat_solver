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

## Fichiers de tests
Des fichiers CNF de tests se trouvent dans le dossier `cnf_files`. Il y a
des traductions de problèmes de factorisation en problèmes SAT: les fichiers
`prime1.cnf` à `prime5.cnf` sont associés à des nombres premiers et sont non
satisfiables. Les fichiers `comp1.cnf` à `comp5.cnf` sont associés à des nombres
composés et sont satisfiables. Il y a également divers fichiers provenant du site
http://www.cs.ubc.ca/~hoos/SATLIB/benchm.html

## Structuration du code

* Le dossier `solver` constitue le coeur du projet.

    Le code du solveur lui-même est contenu dans `detail/solver.hpp`, `solver.hpp`, `solver.cpp`, `guess.cpp`, `deduce.cpp` et
    `backtrack.cpp`.

    Le sous-dossier `expr` contient le code relatif au traitement des formules logiques conviviales. La structure des expressions et les opérations sur celles-ci sont contenues dans les fichiers `expr/detail/logical_expr.hpp`, `expr/logical_expr.hpp`, `expr/logical_expr.cpp`, `expr/detail/tseitin.hpp`, `expr/tseitin.hpp` et `expr/tseitin.cpp`.

    Le code pour parser des expressions avec Flex et Bison se trouve dans les fichiers `expr/detail/logical_parser.ypp`, `expr/detail/logical_scanner.lpp`, `expr/detail/logical_driver.hpp` et `expr/detail/logical_driver.cpp`.

    Nous avons choisi une solution basée sur `boost::variant` pour représenter les expressions sous forme arborescente plutôt qu'une solution basée sur le polymorphisme virtuel qui est lent et source de nombreuses erreurs.

* Le dossier `tests` contient les tests unitaires du projet. Ce sont surtout des tests servant à vérifier le bon fonctionnement de plusieurs cas particuliers. Il permet aussi de générer automatiquement des problèmes associés à la factorisation de nombres entiers (sur 64 bits seulement pour l'instant, cf `factor.cpp` et `gen_tests.cpp`) pour tester le solveur plus en profondeur et pour pouvoir faire du profiling.

* Le dossier `sat` contient le code de l'interface en ligne de commande.

## Structures de données utilisées
Pour obtenir les meilleures performances possibles, nous avons concentré nos efforts dans la réduction des allocations mémoire
lors de la phase de propagation. Ainsi, toutes les structures utilisés au cours de cette phase sont des `std::vector` ayant
été pré-alloués lors de la construction de l'instance du solveur. Cela permet aussi d'avoir des structures qui sont contiguës
en mémoire, ceci est très important pour éviter au maximum les cache misses quand on parcout ces structures (et on les parcourt
très souvent).

Ainsi, lorsque l'algorithme de DPLL est en marche, il n'y a aucune allocation mémoire.

## Améliorations possibles
Nous devons encore chercher à améliorer les heuristiques MOMS et DLIS, car le temps passé à parier est beaucoup trop élevé par
rapport au temps passé à faire de la propagation ou du backtracking (60% du temps passé à parier pour MOMS et 35% pour DLIS).
Pour ces deux heuristiques, il faudrait réussir à tenir à jour une file de priorité pendant la phase de propagation, pour que
l'accès au littéral le plus intéressant lors des paris puisse se faire en temps constant. Mais il faudrait aussi faire attention
à ne pas ralentir la phase de propagation. Nous nous pencherons plus en détails sur ce problème un peu plus tard.

## Répartition du travail

* parsing des expressions logiques: Alexandre
* transformation de Tseitin: Nicolas
* tests unitaires: Alexandre
* DPLL: Alexandre et Nicolas
* prétraitement de l'entrée: Nicolas
* heuristiques MOMS, DLIS et RAND: Alexandre et Nicolas
* watched litterals: Alexandre
* script pour mesurer les performances: Nicolas
