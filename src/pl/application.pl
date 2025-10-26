:- ensure_loaded(syntax).
:- ensure_loaded(equivalence).
:- use_module(library(error)).


% operator for determining if some number of
%     partial applications of a type produces a result type.
% schema: apply(Type, Args, Params, Result)

% early checks
apply(A, Mappings, _, _, _) :-
    assertion(ground(A)),
    assertion(is_list(Mappings)),
    fail.

% base case, no partial applications.
apply(A, Mappings, [], [], B) :-
    equivalent(Mappings, A, B).

% general case, partial application.
apply((X::A)~>B, Mappings, [XY|RestArgs], [AY|RestParams], R) :-
    atom(X),
    equivalent(Mappings, A, AY),
    apply(B, [[X|XY]|Mappings], RestArgs, RestParams, R).


% build application list from list of terms.

express_application(F, [], F) :-
    !.
express_application(F, [X|XT], R) :-
    express_application(F@X, XT, R).

