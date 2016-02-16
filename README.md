#A C++ SAT solver

* pour compiler, exécuter `make`
* pour compiler avec les messages de debug, exécuter `make debug`
* pour exécuter les tests unitaires, exécuter `./tests/tests`
* pour utiliser le solveur en mode CNF, exécuter `./resol file.cnf`
* pour utiliser le solveur en mode convivial, exécuter `./resol -tseitin file.for`

## Fonctionnalités
* solveur utilisant l'algorithme DPLL pour satisfaire des formules sous forme CNF
* parsing de formules logiques sous forme générale
* transformation de Tseitin

## Fichiers de tests
Des fichiers CNF de tests se trouvent dans le dossier `cnf_files`. Ce sont
des traductions de problèmes de factorisation en problèmes SAT. Les fichiers
`prime1.cnf` à `prime5.cnf` sont associés à des nombres premiers et sont non
satisfiables. Les fichiers `comp1.cnf` à `comp5.cnf` sont associés à des nombres
composés et sont satisfiables.

## Structuration du code

* Le dossier `solver` constitue le coeur du projet.

    Le code du solveur lui-même est contenu dans `detail/clause.hpp`, `solver.hpp` et `solver.cpp`.

    Le sous-dossier `expr` contient le code relatif au traitement des formules logiques conviviales. La structure des expressions et les opérations sur celles-ci sont contenues dans les fichiers `expr/detail/logical_expr.hpp`, `expr/logical_expr.hpp`, `expr/logical_expr.cpp`, `expr/detail/tseitin.hpp`, `expr/tseitin.hpp` et `expr/tseitin.cpp`.

    Le code pour parser des expressions avec Flex et Bison se trouve dans les fichiers `expr/detail/logical_parser.ypp`, `expr/detail/logical_scaner.lpp`, `expr/detail/logical_driver.hpp` et `expr/detail/logical_driver.cpp`.

    Nous avons choisi une solution basée sur `boost::variant` pour représenter les expressions sous forme arborescente plutôt qu'une solution basée sur le polymorphisme virtuel qui est lent et susceptible d'engendrer de nombreuses erreurs.

* Le dossier `tests` contient les tests unitaires du projet. Ce sont surtout des tests servant à vérifier le bon fonctionnement de plusieurs cas particuliers. Il contient aussi une base de code qui permettra de générer automatiquement des problèmes associés à la factorisation de nombres entiers pour tester automatiquement le solveur plus en profondeur et pour pouvoir faire du profiling.

* Le dossier `sat` contient le code de l'interface en ligne de commande.
