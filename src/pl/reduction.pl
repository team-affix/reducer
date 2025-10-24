:- ensure_loaded(syntax).
:- ensure_loaded(application).


% define reduce/3

reduce(Defs, A@B, R) :-
    % A needs reducing
    reduce(Defs, A, AR),
    A \= AR,
    !,
    reduce(Defs, AR@B, R).

reduce(Defs, A@B, R) :-
    % B needs reducing
    reduce(Defs, B, BR),
    B \= BR,
    !,
    reduce(Defs, A@BR, R).

reduce(Defs, (A::B)~>C, R) :-
    % B needs reducing
    reduce(Defs, B, BR),
    B \= BR,
    !,
    reduce(Defs, (A::BR)~>C, R).

reduce(Defs, (A::B)~>C, R) :-
    % C needs reducing
    reduce(Defs, C, CR),
    C \= CR,
    !,
    reduce(Defs, (A::B)~>CR, R).

% Beta reduction: ((X::Type)~>Body) @ Arg reduces to Body[X := Arg]
reduce(Defs, ((X::Type)~>Body)@Arg, R) :-
    nonvar(Type),
    !,
    % Use apply to perform substitution
    apply((X::Type)~>Body, [], [Arg], [Type], Substituted),
    reduce(Defs, Substituted, R).

reduce(Defs, A, BR) :-
    member([A|B], Defs),
    reduce(Defs, B, BR),
    !.
    
reduce(Defs, A, A) :-
    \+ member([A|_], Defs),
    !.

