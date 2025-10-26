:- ensure_loaded(syntax).
:- ensure_loaded(application).
:- ensure_loaded(universes).
:- ensure_loaded(environment).
:- ensure_loaded(variable).
:- use_module(library(error)).

% typecheck a list of terms and types.
typecheck_all(_, _, _, [], []) :-
    !.
typecheck_all(Limit, Gamma, Rho, [X|XT], [Y|YT]) :-
    typecheck(Limit, Gamma, Rho, X, Y),
    typecheck_all(Limit, Gamma, Rho, XT, YT).


% typecheck/5
% schema: typecheck(Limit, Gamma, Rho, Term, Type)
% Limit: recursion limit.
% Gamma: type environment
%     (association list of term,type pairs,
%     including those from global signature and local context).
% Rho: definition environment
%     (association list of term,definition pairs).
% Term: term to typecheck.
% Type: type to check against.
% NOTE: typechecking must always yield a ground term,type pair.

% early checks
typecheck(Limit, Gamma, Rho, _, _) :-
    assertion(number(Limit)),
    assertion(is_list(Gamma)),
    assertion(is_list(Rho)),
    fail.

% standard case, lookup term,type pair from environment and
%     determine if some application of the retrieved type
%     produces the desired type.
typecheck(Limit, Gamma, Rho, Term, Type) :-
    % step 1: get term,type pair from environment.
    member([PTerm|PType], Gamma),
    % step 2: determine if some application of the
    %     retrieved type produces the desired type.
    apply(PType, Rho, Args, Params, Type),
    % step 3: express the application, determining
    %     if the retrieved term applied to args
    %     matches the desired term.
    express_application(PTerm, Args, Term),
    % step 4: check if the arguments fulfill
    %     the parameter types.
    Limit > 0,
    NewLimit is Limit - 1,
    typecheck_all(NewLimit, Gamma, Rho, Args, Params).

% base cases for typechecking
% - function types are types (so long as their components are types)
typecheck(Limit, Gamma, Rho, (X::A)~>B, set@MaxLevel) :-
    % step 1: handle recursion limit
    Limit > 0,
    NewLimit is Limit - 1,
    % step 2: get the left universe level, and instantiate A.
    %     NOTE: A may be unground, but upon declaration, it will be grounded.
    typecheck(NewLimit, Gamma, Rho, A, set@ALevel),
    % step 3: prepend the term,type pair to the env.
    %     this grounds X,A.
    declarea(NewLimit, [[X|A]], Gamma, NewGamma),
    % step 4: get the right universe level.
    typecheck(NewLimit, NewGamma, Rho, B, set@BLevel),
    % step 5: the pi-type has the max level of its components.
    maxlevel(ALevel, BLevel, MaxLevel).

% base cases for typechecking
% - function definitions have function types
typecheck(Limit, Gamma, Rho, A~>B, (X::AType)~>BType) :-
    % step 1: handle recursion limit
    Limit > 0,
    NewLimit is Limit - 1,
    % step 2: get the left universe level, and instantiate A.
    %     NOTE: A may be unground, but upon declaration, it will be grounded.
    typecheck(NewLimit, Gamma, Rho, AType, set@_),
    % step 3: make sure A is grounded.
    (atom(A), !; next_variable(A)),
    % step 4: make sure X is grounded.
    (atom(X), !; next_variable(X)),
    % step 5: prepend the term,type pair to the env.
    %     this grounds X,A.
    declarea(NewLimit, [[A|AType]], Gamma, NewGamma),
    % step 6: get the body type.
    typecheck(NewLimit, NewGamma, Rho, B, BType),
    % step 7: the body must not be a type.
    BType \= set@_.


iterative_deepening_typecheck(Limit, Gamma, Rho, Term, Type) :-
    typecheck(Limit, Gamma, Rho, Term, Type);
    NewLimit is Limit + 1,
    iterative_deepening_typecheck(NewLimit, Gamma, Rho, Term, Type).

