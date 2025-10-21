% helper operator for representing function application.
:- op(500, yfx, @).

% helper operator for defining function types.
:- op(550, xfy, ~>).

% helper operator for representing named types.
:- op(600, xfy, ::).


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


% typecheck a list of terms and types.
typecheck_all(_, [], []).
typecheck_all(Limit, [X|XT], [Y|YT]) :-
    typecheck(Limit, X, Y),
    typecheck_all(Limit, XT, YT).


%:- table typecheck/3.

typecheck(Limit, Term, Type) :-
    % step 1: get postulate of given type
    postulate(PTerm, PType),
    % step 2: determine if some application of the
    %     postulated type produces the desired type.
    apply(PType, [], Args, Params, Type),
    % step 3: express the application
    express_application(PTerm, Args, Term),
    % step 4: check if the requirements can be met.
    Limit > 0,
    NewLimit is Limit - 1,
    typecheck_all(NewLimit, Args, Params).

% base cases for typechecking
% - function types are types (so long as their components are types)
typecheck(Limit, (_::A)~>B, set@MaxLevel) :-
    Limit > 0,
    NewLimit is Limit - 1,
    typecheck(NewLimit, A, set@ALevel),
    typecheck(NewLimit, B, set@BLevel),
    maxlevel(ALevel, BLevel, MaxLevel).


% make sure that the type table is cached for quick lookups.
%:- table postulate/2.
:- dynamic postulate/2.

% postulate schema:
% postulate(Term, Type)
% Term: term to postulate a type for.
% Type: type of the term.

% define level constructors
postulate(level, set@lzero).  % circular, yes I know.
postulate(lzero, level).
postulate(lsuc,  (l :: level) ~> level).

% define set
postulate(set, (l :: level) ~> set@(lsuc@l)).

postulate(bool     , set@lzero).
postulate(int      , set@lzero).
postulate(double   , set@lzero).
postulate(string   , set@lzero).
postulate(vector   , (t :: set@lzero) ~> set@lzero). % X itself has to be a type.
postulate(sum_type , (t :: set@lzero) ~> (x :: t) ~> set@lzero).
postulate(map      , (k :: set@lzero) ~> (v :: set@lzero) ~> set@lzero).

postulate(false                 , bool).
postulate(true                  , bool).
postulate(lessthan              , (lhs :: int) ~> (rhs :: int) ~> bool).
postulate('0'                   , int).
postulate(default_int           , int).
postulate(square                , (x :: int) ~> int).
postulate(suc                   , (x :: int) ~> int).
postulate(size                  , (t :: set@lzero) ~> (v :: vector@t) ~> int).
postulate(default_double        , double).
postulate(default_string        , string).
postulate(default_vector        , (t :: set@lzero) ~> vector@t).
postulate(cons                  , (t :: set@lzero) ~> (v :: vector@t) ~> (x :: t) ~> vector@t).
postulate(default_sum_type      , (t :: set@lzero) ~> (x :: t) ~> sum_type@t@x).
postulate(default_map           ,
    (k :: set@lzero) ~>
    (v :: set@lzero) ~>
    (compare :: (lhs :: k) ~> (rhs :: k) ~> bool) ~>
    (default :: v) ~>
    (kr :: strictly_ordered@k@compare) ~>
    (vr :: default_constructable@v@default) ~>
    map@k@v).


% declare/3
% schema: declare(RLimit, Term, Type)
% declare a term and its type, given some checking recursion limit.
declare(RLimit, Term, Type) :-
    % step 1: make sure the term is an atom
    atom(Term),
    % step 2: make sure the type is ground
    ground(Type),
    % step 3: make sure the term is not already declared.
    \+ postulate(Term, _),
    % step 4: make sure the type is valid (belongs to a universe).
    typecheck(RLimit, Type, set@L),
    level(L),
    % step 5: declare the term and its type.
    assertz(postulate(Term, Type)).



iterative_deepening_typecheck(Limit, Term, Type) :-
    typecheck(Limit, Term, Type);
    NewLimit is Limit + 1,
    iterative_deepening_typecheck(NewLimit, Term, Type).
