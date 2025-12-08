# rough draft

In this document, I will attempt to specify a potential route in which AML can be implemented so as to balance efficency of search, and completeness of existential solutions.

## Dependent Types

In this implementation, we will have the candidate search for hole-filling in direct function building be constrained by type compatability. (Other heuristics can be added later.)

In order to make sufficiently complete our ability to specify functionality and relationships between functions / types, we will employ dependent types.

### Inhabitation Problem

The constraint solving needed by direct function building, when involving types, can be thought of as inhabitant search. When it comes to dependent types however, it is extremely challenging to implement an inhabitant search algorithm, and it is challenging to get it to execute quickly in an on-demand way (subordinate to DFB).

For this reason, I propose a **forward polling** system which takes in arbitrary dependent types (pi-types acceptable) which are specified by the user, and allows the search system (MCTS) to instantiate the dependent type information in ways that might be needed later on in construction of the model. The 'instantiations' of these templates are referred to as primitives. Primitives are simply typed functions or types, which can be accessed directly during the DFB process.

The forward polling system ensures completeness of reachability of necessary types from templates, but doesn't waste an inordinate amount of time doing something like dependent type inhabitant search. Instead, given a Pi-type e.g. `(T : Set) -> T -> T`, all dependencies (in our case, only one dependency `(T : Set)`) are instantiated in order to leave us with a simply typed structure. The instantiation process for the arguments supplied to the template are either done by means of *simple type inhabitant search* where previously declared concrete types are looked up in a table, or if param types are also allowed to be dependent types (pi-types), then a slightly more elaborate system will need to be thought of. In either case, the problem seems to be easier than dependent type inhab search (DTIS) due to the fact that construction of an inhabitant of a pi-type is not necessarily as challenging as needing to supply arguments to a pi-type in order to get some specific type output out of it.

Another thing to mention is that we could technically allow for forward polling to be a process by which an arbitrary nonzero number of the dependencies can be argued, making it so we can generate curried templates from templates. E.g. `f : (T : Set) -> (J : Set) -> T -> J` could be partially instantiated to: `(J : Set) -> int -> J` if `f` is partially applied to `int`, leaving us with a new template. (still cannot be directly used in DBF). This would give MCTS an easier time if multiple complete instantiations of a template could be useful which all have the same first n arguments.

## Helper Functions

In order to conduct AML, there needs to be a capacity to reuse subroutines (hellpers) across many different parts of the model. The ability to do functional distribution is one of the sources of strength for AML over other machine learning techniques, due to the fact that reuse of helpers allows us to approach occams razor which gives us hope for generality, but also that in reusing a definition in multiple places, the correctness of the definition is thereby informed by the influence that each occurrance has on the reward (non-local generalization).

However, if the goal is to get as close to occams razor representation as possible, we need to open up some generic toolsets to the machine learning system for it to use to its benefit. One toolset is **abstraction**, or the ability to construct subroutines. Previously mentioned (nonlocal generalization) is yielded through **application** of a subroutine in multiple places, but in order to make the subroutines themselves optimal, it requires that they be derived as well.

In this system, there are several types of functions, and types.

- Templatted { functions / types }
- Primitive  { functions / types }
- Derived    { functions / types }

