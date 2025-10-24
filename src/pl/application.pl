:- ensure_loaded(syntax).
:- ensure_loaded(equivalence).


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

