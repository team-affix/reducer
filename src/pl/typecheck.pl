:- ensure_loaded(syntax).
:- ensure_loaded(grounding).
:- ensure_loaded(application).
:- ensure_loaded(universes).


% Forward declaration - environment.pl will define these
:- discontiguous declarea/4.
:- discontiguous can_declare/4.


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

% base cases for typechecking
% - function definitions have function types
typecheck(Limit, Env, (X::A)~>B, (X::A)~>BType) :-
    % step 1: handle recursion limit
    Limit > 0,
    NewLimit is Limit - 1,
    % step 2: get the left universe level, and instantiate A.
    %     NOTE: A may be unground, but upon declaration, it will be grounded.
    typecheck(NewLimit, Env, A, set@ALevel),
    % step 3: prepend the term,type pair to the env.
    %     this grounds X,A.
    declarea(NewLimit, [[X|A]], Env, NewEnv),
    % step 4: get the body type.
    typecheck(NewLimit, NewEnv, B, BType),
    % step 5: the body must not be a type.
    BType \= set@_.


iterative_deepening_typecheck(Limit, Env, Term, Type) :-
    typecheck(Limit, Env, Term, Type);
    NewLimit is Limit + 1,
    iterative_deepening_typecheck(NewLimit, Env, Term, Type).

