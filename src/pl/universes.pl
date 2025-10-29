:- ensure_loaded(syntax).


% define level/1
level(lzero).
level(lsuc@X) :-
    level(X).


% define maxlevel/3
maxlevel(lzero, lzero, lzero) :-
    !.
maxlevel(lzero, lsuc@X, lsuc@X) :-
    !.
maxlevel(lsuc@X, lzero, lsuc@X) :-
    !.
maxlevel(lsuc@X, lsuc@Y, lsuc@Z) :-
    !,
    maxlevel(X, Y, Z).

