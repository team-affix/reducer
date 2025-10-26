:- ensure_loaded(syntax).
:- ensure_loaded(grounding).
:- ensure_loaded(universes).
:- ensure_loaded(typecheck).


% can_declare/4
% schema: can_declare(Limit, Term, Type, Env)
% Limit: recursion limit.
% Term: term to check for declaration.
% Type: type to check for declaration.
% Env: environment.
% NOTE: all term,type pairs in env are expected to be ground.
% NOTE: all terms will be grounded before declaration.
% NOTE: all types will be grounded before declaration.
assert_declarable(Limit, Term, Type, Env) :-
    % step 1: get len of env for renaming vars to atoms.
    length(Env, NameIndex),
    % step 2: make sure the term is an atom
    atom(Term),
    % step 3: make sure the type is ground
    ground(Type),
    % step 4: make sure the term is not already part of the environment.
    \+ member([Term|_], Env),
    % step 5: make sure the type is valid (belongs to a universe).
    typecheck(Limit, Env, Type, set@Level),
    level(Level).


% define declarea/4
% NOTE: prepends a declaration to the environment.
declarea(_    , []      , Env, Env   ).
declarea(Limit, [[Term|Type]|RestDecls], Env, NewEnv) :-
    assert_declarable(Limit, Term, Type, Env),
    append([[Term|Type]], Env, TmpEnv),
    declarea(Limit, RestDecls, TmpEnv, NewEnv).


% define declarez/4
% NOTE: appends a declaration to the environment.
declarez(_    , []      , Env, Env   ).
declarez(Limit, [[Term|Type]|RestDecls], Env, NewEnv) :-
    assert_declarable(Limit, Term, Type, Env),
    append(Env, [[Term|Type]], TmpEnv),
    declarez(Limit, RestDecls, TmpEnv, NewEnv).


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

