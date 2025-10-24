:- ensure_loaded(syntax).


% define ground_term/2
% schema: ground_term(!Index, ?>!Term)
ground_term(Index, Term) :-
    number(Index),
    atom(Term).
ground_term(Index, Term) :-
    number(Index),
    var(Term),
    atom_concat(tm, Index, Term).


% ground_type/2
% schema: ground_type(!Index, ?>!Type)
% Env: environment
% Type: input type to ground.
ground_type(Index, Type) :-
    number(Index),
    atom(Type).

ground_type(Index, A@B) :-
    number(Index),
    nonvar(A), % even though they may be nonground,
    nonvar(B), % we need them to be nonvar for the grounding to work.
    ground_type(Index, A),
    NextIndex is Index + 1,
    ground_type(NextIndex, B).

ground_type(Index, (A::B)~>C) :-
    number(Index),
    % B should be nonvar at this point, as:
    %     if B was var at any point, and the whole thing is a valid type,
    %     then B should have been bound to an earlier binder name, which
    %     should have been made an atom by this point.
    %     and if B is itself a function type, then it may contain
    %     more binders, which can be variables, but it is still nonvar
    %     as function types are compound terms.
    nonvar(B),
    % step 1: make sure A is an atom
    ground_term(Index, A),
    % step 2: ground B.
    ground_type(Index, B),
    % step 3: compute the next index for naming.
    NextIndex is Index + 1,
    % step 4: ground C.
    ground_type(NextIndex, C).

