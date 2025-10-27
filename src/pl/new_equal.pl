:- consult(syntax).
:- consult(copy).
:- consult(variable).


% check_limit(PrevLimit, NewLimit)
check_limit(PrevLimit, NewLimit) :-
    PrevLimit > 0,
    NewLimit is PrevLimit - 1.


equal_expr(L, _, A, A) :-
    check_limit(L, _).

equal_expr(L, Rho, A, X) :-
    check_limit(L, L2),
    member([A|Y], Rho),
    copy_expr([], Y, YC),
    equal_expr(L2, Rho, YC, X).
    
equal_expr(L, Rho, (A::B)~>C, (X::Y)~>Z) :-
    check_limit(L, L2),
    (atom(A);next_variable(A)),
    (atom(X);next_variable(X)),
    assertion(\+ member([A|_], Rho)),
    equal_expr(L2, Rho, B, Y),
    equal_expr(L2, [[A|X]|Rho], C, Z).
    
equal_expr(L, Rho, A~>B, X~>Y) :-
    check_limit(L, L2),
    (atom(A);next_variable(A)),
    (atom(X);next_variable(X)),
    assertion(\+ member([A|_], Rho)),
    equal_expr(L2, [[A|X]|Rho], B, Y).

equal_expr(L, Rho, A@B, X@Y) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, A, X),
    equal_expr(L2, Rho, B, Y).
    
equal_expr(L, Rho, A@B, R) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, A, X~>Y),
    (atom(X);next_variable(X)),
    assertion(\+ member([X|_], Rho)),
    equal_expr(L2, Rho, B, C),
    %(atom(C);next_variable(C)),
    equal_expr(L2, [[X|C]|Rho], Y, R).
    
equal_expr(L, Rho, A?B, X?Y) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, A, X),
    equal_cond_body(L2, Rho, B, Y).
    
equal_expr(L, Rho, A?B, R) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, A, AR),
    equal_cond_body(L2, Rho, B, BR),
    eval_cond_body(L2, Rho, AR, BR, R).

equal_expr(L, Rho, A, B) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, B, A).


equal_cond_body(L, _, end, end) :-
    check_limit(L, _).

equal_cond_body(L, Rho, A=>B//C, X=>Y//Z) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, A, X),
    equal_expr(L2, Rho, B, Y),
    equal_cond_body(L, Rho, C, Z).


% cond body evaluation
:- table eval_cond_body/4.
eval_cond_body(L, Rho, AR, B=>C//_, CR) :-
    check_limit(L, L2),
    equal_expr(L2, Rho, AR, B),
    !,
    equal_expr(L2, Rho, C, CR).

eval_cond_body(L, Rho, AR, _=>_//D, DR) :-
    check_limit(L, _),
    eval_cond_body(L, Rho, AR, D, DR).

