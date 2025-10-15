% helper operator for representing function application.
:- op(500, yfx, @).

% helper operator for defining function types.
:- op(550, xfy, ~>).

% helper operator for representing named types.
:- op(600, xfy, ::).


% operator for determining if some number of
%     partial applications of a type produces a result type.
% schema: apply(Type, Args, Params, Result)
apply(A, [], [], A).
apply((X::A)~>B, [X|RestArgs], [A|RestParams], R) :-
    apply(B, RestArgs, RestParams, R).


% build application list from list of terms.
express_application(F, [], F).
express_application(F, [X|XT], R) :-
    express_application(F@X, XT, R).


% typecheck a list of terms and types.
typecheck_all(_, [], []).
typecheck_all(RLimit, [X|XT], [Y|YT]) :-
    RLimit > 0,
    NewRLimit is RLimit - 1,
    typecheck(NewRLimit, X, Y),
    typecheck_all(NewRLimit, XT, YT).


%:- table typecheck/3.

typecheck(RLimit, Term, Type) :-
    % step 1: get postulate of given type
    postulate(PTerm, PType),
    % step 2: determine if some application of the
    %     postulated type produces the desired type.
    apply(PType, Args, Params, Type),
    % step 3: express the application
    express_application(PTerm, Args, Term),
    % step 4: check if the requirements can be met.
    RLimit > 0,
    NewRLimit is RLimit - 1,
    typecheck_all(NewRLimit, Args, Params),
    % step 5: check if the term is well-typed.
    typecheck(NewRLimit, Type, set).

% base cases for typechecking
% - set is a type
typecheck(_, set, set).
% - function types are types (so long as their components are types)
typecheck(RLimit, (_::A)~>B, set) :-
    RLimit > 0,
    NewRLimit is RLimit - 1,
    typecheck(NewRLimit, A, set),
    typecheck(NewRLimit, B, set).


% make sure that the type table is cached for quick lookups.
%:- table postulate/2.

% postulate schema:
% postulate(Term, Type)
% Term: term to postulate a type for.
% Type: type of the term.

postulate(bool     , set).
postulate(int      , set).
postulate(double   , set).
postulate(string   , set).
postulate(vector   , (_ :: set) ~> set). % X itself has to be a type.
postulate(sum_type , (X :: set) ~> (_ :: X) ~> set).
postulate(map      ,
    (X :: set) ~>
    (Y :: set) ~>
    (lessthan(X) :: (_ :: X) ~> (_ :: X) ~> bool) ~>
    (default(Y) :: Y) ~> set).

postulate(false                 , bool).
postulate(true                  , bool).
postulate(default(int)          , int).
postulate(0                     , int).
postulate(default(double)       , double).
postulate(default(string)       , string).
postulate(default(vector@X)     , vector@X).
postulate(cons(X)               , (_ :: vector@X) ~> (_ :: X) ~> vector@X).
postulate(default(sum_type@X@Y) , sum_type@X@Y).
postulate(square                , (_ :: int) ~> int).
postulate(suc                   , (_ :: int) ~> int).
postulate(size(vector(X))       , (_ :: vector@X) ~> int).
postulate(size(map(X, Y))       , (_ :: map@X@Y@lessthan(X)@default(Y)) ~> int).
postulate(lessthan(int)         , (_ :: int) ~> (_ :: int) ~> bool).
postulate(default(map(X, Y))    , map@X@Y@lessthan(X)@default(Y)).
