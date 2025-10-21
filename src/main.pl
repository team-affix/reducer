% helper operator for representing function application.
:- op(500, yfx, @).

% helper operator for defining function types.
:- op(550, xfy, ~>).

% helper operator for representing named types.
:- op(600, xfy, ::).


% define ground_term/2
ground_term(_, Term) :-
    atom(Term).

ground_term(Index, Term) :-
    var(Term),
    atom_concat(tm, Index, Term).


% ground_type/2
% schema: ground_type(Env, Type)
% Env: environment
% Type: input type to ground.
ground_type(_, Type) :-
    atom(Type).

ground_type(Index, A@B) :-
    nonvar(A), % even though they may be nonground,
    nonvar(B), % we need them to be nonvar for the grounding to work.
    ground_type(Index, A),
    NextIndex is Index + 1,
    ground_type(NextIndex, B).

ground_type(Index, (A::B)~>C) :-
    % B should be nonvar at this point, as:
    %     if B was var at any point, and the whole thing is a valid type,
    %     then B should have been bound to an earlier binder name, which
    %     should have been made an atom by this point.
    %     and if B is itself a function type, then it may contain
    %     more binders, which can be variables, but it is still nonvar
    %     as function types are compound terms.
    nonvar(B),
    % step 1: make sure A is an atom
    ground_term(Index, A),
    % step 2: ground B.
    ground_type(Index, B),
    % step 3: compute the next index for naming.
    NextIndex is Index + 1,
    % step 4: ground C.
    ground_type(NextIndex, C).


% determine if two types are equivalent up to renamings.
% schema: equivalent(Mappings, A, B)
% Mappings: association list of binder name mappings.
% A: type to check for equivalence.
% B: type to check for equivalence.
equivalent(Mappings, A, A) :-
    atom(A),
    \+ member([A|_], Mappings).

equivalent(Mappings, A1, A2) :-
    atom(A1),
    member([A1|A2], Mappings).

equivalent(Mappings, A1@B1, A2@B2) :-
    equivalent(Mappings, A1, A2),
    equivalent(Mappings, B1, B2).
    
equivalent(Mappings, (N1::T1)~>R1, (N2::T2)~>R2) :-
    atom(N1),
    \+ member([N1|_], Mappings),
    equivalent(Mappings, T1, T2),
    equivalent([[N1|N2]|Mappings], R1, R2).


% operator for determining if some number of
%     partial applications of a type produces a result type.
% schema: apply(Type, Args, Params, Result)
apply(A, Mappings, [], [], B) :-
    equivalent(Mappings, A, B).

apply((X::A)~>B, Mappings, [XY|RestArgs], [AY|RestParams], R) :-
    atom(X),
    equivalent(Mappings, A, AY),
    apply(B, [[X|XY]|Mappings], RestArgs, RestParams, R).


% build application list from list of terms.
express_application(F, [], F).
express_application(F, [X|XT], R) :-
    express_application(F@X, XT, R).


% define level/1
level(lzero).
level(lsuc@X) :-
    level(X).


% define maxlevel/3
maxlevel(lzero, lzero, lzero).
maxlevel(lzero, lsuc@X, lsuc@X).
maxlevel(lsuc@X, lzero, lsuc@X).
maxlevel(lsuc@X, lsuc@Y, lsuc@Z) :-
    maxlevel(X, Y, Z).


% can_declare/4
% schema: can_declare(Limit, Term, Type, Env)
% Limit: recursion limit.
% Term: term to check for declaration.
% Type: type to check for declaration.
% Env: environment.
% NOTE: all term,type pairs in env are expected to be ground.
% NOTE: all terms will be grounded before declaration.
% NOTE: all types will be grounded before declaration.
can_declare(Limit, Term, Type, Env) :-
    % step 1: get len of env for renaming vars to atoms.
    length(Env, NameIndex),
    % step 2: make sure the term is an atom
    ground_term(NameIndex, Term),
    % step 3: make sure the type is ground
    ground_type(NameIndex, Type),
    % step 4: make sure the term is not already part of the environment.
    \+ member([Term|_], Env),
    % step 5: make sure the type is valid (belongs to a universe).
    typecheck(Limit, Env, Type, set@Level),
    level(Level).


% define declarea/4
% NOTE: prepends a declaration to the environment.
declarea(_    , []      , Env, Env   ).
declarea(Limit, [[Term|Type]|RestDecls], Env, NewEnv) :-
    can_declare(Limit, Term, Type, Env),
    append([[Term|Type]], Env, TmpEnv),
    declarea(Limit, RestDecls, TmpEnv, NewEnv).


% define declarez/4
% NOTE: appends a declaration to the environment.
declarez(_    , []      , Env, Env   ).
declarez(Limit, [[Term|Type]|RestDecls], Env, NewEnv) :-
    can_declare(Limit, Term, Type, Env),
    append(Env, [[Term|Type]], TmpEnv),
    declarez(Limit, RestDecls, TmpEnv, NewEnv).


% typecheck a list of terms and types.
typecheck_all(_, _, [], []).
typecheck_all(Limit, Env, [X|XT], [Y|YT]) :-
    typecheck(Limit, Env, X, Y),
    typecheck_all(Limit, Env, XT, YT).


% typecheck/4
% schema: typecheck(Limit, Env, Term, Type)
% Limit: recursion limit.
% Env: environment (association list of term,type pairs, including those from global signature and local context).
% Term: term to typecheck.
% Type: type to check against.
% NOTE: typechecking must always yield a ground term,type pair.
typecheck(Limit, Env, Term, Type) :-
    % step 1: get term,type pair from environment.
    member([PTerm|PType], Env),
    % step 2: determine if some application of the
    %     retrieved type produces the desired type.
    apply(PType, [], Args, Params, Type),
    % step 3: express the application, determining
    %     if the retrieved term applied to args
    %     matches the desired term.
    express_application(PTerm, Args, Term),
    % step 4: check if the arguments fulfill
    %     the parameter types.
    Limit > 0,
    NewLimit is Limit - 1,
    typecheck_all(NewLimit, Env, Args, Params).

% base cases for typechecking
% - function types are types (so long as their components are types)
typecheck(Limit, Env, (X::A)~>B, set@MaxLevel) :-
    % step 1: handle recursion limit
    Limit > 0,
    NewLimit is Limit - 1,
    % step 2: get the left universe level, and instantiate A.
    %     NOTE: A may be unground, but upon declaration, it will be grounded.
    typecheck(NewLimit, Env, A, set@ALevel),
    % step 3: prepend the term,type pair to the env.
    %     this grounds X,A.
    declarea(NewLimit, [[X|A]], Env, NewEnv),
    % step 4: get the right universe level.
    typecheck(NewLimit, NewEnv, B, set@BLevel),
    % step 5: the pi-type has the max level of its components.
    maxlevel(ALevel, BLevel, MaxLevel).


% define default_environment/1
% NOTE: this constructs an environment with all the necessary
%     declarations for the set family of universes.
default_environment(Env) :-
    Env = [
        [level|set@lzero],
        [lzero|level],
        [lsuc|(l::level) ~> level],
        [set|(l::level) ~> set@(lsuc@l)]
    ].


my_env(Env) :-
    default_environment(DefaultEnv),
    declarez(10, [
        [bool|set@lzero],
        [int|set@lzero],
        [double|set@lzero],
        [string|set@lzero],
        [vector|(t :: set@lzero) ~> set@lzero],
        [sum_type|(t :: set@lzero) ~> (x :: t) ~> set@lzero],
        [map|(k :: set@lzero) ~> (v :: set@lzero) ~> set@lzero],
        [false|bool],
        [true|bool],
        [lessthan|(lhs :: int) ~> (rhs :: int) ~> bool],
        ['0'|int],
        [default_int|int],
        [square|(x :: int) ~> int],
        [suc|(x :: int) ~> int],
        [size|(t :: set@lzero) ~> (v :: vector@t) ~> int],
        [default_double|double],
        [default_string|string],
        [default_vector|(t :: set@lzero) ~> vector@t],
        [cons|(t :: set@lzero) ~> (v :: vector@t) ~> (x :: t) ~> vector@t],
        [default_sum_type|(t :: set@lzero) ~> (x :: t) ~> sum_type@t@x]
    ], DefaultEnv, Env).


iterative_deepening_typecheck(Limit, Env, Term, Type) :-
    typecheck(Limit, Env, Term, Type);
    NewLimit is Limit + 1,
    iterative_deepening_typecheck(NewLimit, Env, Term, Type).


:- dynamic persist/1.
