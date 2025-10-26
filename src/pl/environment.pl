:- ensure_loaded(syntax).
:- ensure_loaded(universes).
:- ensure_loaded(typecheck).


% can_declare/5
% schema: can_declare(Limit, Term, Type, Env)
% Limit: recursion limit.
% Term: term to check for declaration.
% Type: type to check for declaration.
% Env: environment.
% NOTE: all term,type pairs in env are expected to be ground.
% NOTE: all terms will be grounded before declaration.
% NOTE: all types will be grounded before declaration.
assert_declarable(Limit, Gamma, Rho, Term, Type) :-
    % step 1: make sure the term is an atom
    assertion(atom(Term)),
    % step 2: make sure the type is ground
    assertion(ground(Type)),
    % step 3: make sure the term is not already part of the environment.
    assertion(\+ member([Term|_], Gamma)),
    % step 4: make sure the type is valid (belongs to a universe).
    assertion(typecheck(Limit, Gamma, Rho, Type, set@Level)),
    assertion(level(Level)).


% define declarea/5
% NOTE: prepends a declaration to the environment.
declarea(_, _, _, [], _) :-
    !.
declarea(Limit, Gamma, Rho, [[Term|Type]|RestDecls], NewGamma) :-
    assert_declarable(Limit, Gamma, Rho, Term, Type),
    append([[Term|Type]], Gamma, TmpGamma),
    declarea(Limit, TmpGamma, Rho, RestDecls, NewGamma).


% define declarez/5
% NOTE: appends a declaration to the environment.
declarez(_, _, _, [], _) :-
    !.
declarez(Limit, Gamma, Rho, [[Term|Type]|RestDecls], NewGamma) :-
    assert_declarable(Limit, Gamma, Rho, Term, Type),
    append(Gamma, [[Term|Type]], TmpGamma),
    declarez(Limit, TmpGamma, Rho, RestDecls, NewGamma).


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

