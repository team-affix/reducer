:- ensure_loaded(syntax).
:- ensure_loaded(application).
:- ensure_loaded(universes).
:- ensure_loaded(environment).
:- ensure_loaded(variable).
:- use_module(library(error)).

% update_limit
update_limit(Limit, NewLimit) :-
    Limit > 0,
    NewLimit is Limit - 1.


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
typecheck(Limit, Gamma, Rho, Term, Type) :-
    assertion(number(Limit)),
    assertion(is_list(Gamma)),
    assertion(is_list(Rho)),
    assertion(ground(Gamma)),
    assertion(ground(Rho)),
    assertion(
        ground(Term) -> true;
        ground(Type)),
    fail.

% standard case, lookup term,type pair from environment and
%     determine if some application of the retrieved type
%     produces the desired type.
typecheck(Limit, Gamma, Rho, Term, Type) :-
    write('>>>>>standard-case'), nl,
    update_limit(Limit, _),
    % step 1: get term,type pair from environment.
    member([PTerm|PType], Gamma),
    % step 2: copy the PType to get fresh variables
    copy_expr([], PType, PTypeCopy),
    % step 3: determine if the retrieved term,type pair works
    (
        % if Term is ground, then check against PTerm
        ground(Term) ->
        equivalent(Rho, Term, PTerm),
        Type = PTypeCopy ;
        % if Type is ground, then check against PTypeCopy
        equivalent(Rho, Type, PTypeCopy),
        Term = PTerm
    ).

% pi-type typecheck
typecheck(Limit, Gamma, Rho, (X::A)~>B, set@MaxLevel) :-
    update_limit(Limit, NewLimit),
    write('>>>>>pi-type'), nl,
    % step 1: determine mode (typecheck/instance search)
    (
        % if (X::A)~>B is ground, then typecheck A and B
        ground((X::A)~>B) ->
        typecheck(NewLimit, Gamma, Rho, A, set@ALevel),
        declarea(NewLimit, Gamma, Rho, [[X|A]], NewGamma),
        typecheck(NewLimit, NewGamma, Rho, B, set@BLevel)
        ;
        % if Type is ground, then do instance search
        typecheck(NewLimit, Gamma, Rho, ALevel, level),
        typecheck(NewLimit, Gamma, Rho, BLevel, level),
        typecheck(NewLimit, Gamma, Rho, A, set@ALevel),

        (atom(X) -> true ; next_variable(X)),
        declarea(NewLimit, Gamma, Rho, [[X|A]], NewGamma),

        typecheck(NewLimit, NewGamma, Rho, B, set@BLevel),

        (atom(X) -> true ; next_variable(X))
    ),
    % step 3: get the max level of the components.
    maxlevel(ALevel, BLevel, MaxLevel).


% function definition typecheck
typecheck(Limit, Gamma, Rho, A~~>B, (A::AType)~>BType) :-
    write('>>>>>function-definition'), nl,
    update_limit(Limit, NewLimit),
    % step 1: determine mode (typecheck/instance search)
    (
        % if A~~>B is ground, then typecheck A and B
        ground(A~~>B) ->
        typecheck(NewLimit, Gamma, Rho, Level, level),
        write('>>>>>>>>>>>>>>>>>>>LEVEL: '), write(Level), nl,
        typecheck(NewLimit, Gamma, Rho, AType, set@Level),

        %next_variable(A), not needed, A is always ground

        
        declarea(NewLimit, Gamma, Rho, [[A|AType]], NewGamma),

        typecheck(NewLimit, NewGamma, Rho, B, BType)

        ;
        % if Type is ground, then do instance search
        
        %next_variable(A), not needed, A is always ground

        write('>>>>>>>>>>>>>>>>>>>A: '), write(A), nl,
        write('>>>>>>>>>>>>>>>>>>>AType: '), write(AType), nl,
        write('>>>>>>>>>>>>>>>>>>>BType: '), write(BType), nl,

        declarea(NewLimit, Gamma, Rho, [[A|AType]], NewGamma),

        typecheck(NewLimit, NewGamma, Rho, B, BType)
        
    ).

% function application typecheck
typecheck(Limit, Gamma, Rho, A@B, Type) :-
    write('>>>>>function-application'), nl,
    update_limit(Limit, NewLimit),
    % step 1: determine mode (typecheck/instance search)
    (
        % if A@B is ground, then typecheck A and B
        ground(A@B) ->
        typecheck(NewLimit, Gamma, Rho, A, (X::T)~>UnreducedType),

        % might not have to do this, since X is never used...
        %declarea(...),

        (atom(X) -> true ; next_variable(X)),
        NewRho = [[X|B]|Rho],
        typecheck(NewLimit, Gamma, NewRho, B, T),

        reduce(NewRho, UnreducedType, Type)
        ;
        % if Type is ground, then do instance search
        typecheck(NewLimit, Gamma, Rho, Level, level),
        typecheck(NewLimit, Gamma, Rho, T, set@Level),
        typecheck(NewLimit, Gamma, Rho, B, T),

        next_variable(X),
        NewRho = [[X|B]|Rho],

        typecheck(NewLimit, Gamma, NewRho, UnreducedA, (X::T)~>Type),

        reduce(NewRho, UnreducedA, A)
    ).


iterative_deepening_typecheck(Limit, Gamma, Rho, Term, Type) :-
    typecheck(Limit, Gamma, Rho, Term, Type);
    NewLimit is Limit + 1,
    iterative_deepening_typecheck(NewLimit, Gamma, Rho, Term, Type).



:- dynamic persist/1.