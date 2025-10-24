:- ensure_loaded(syntax).


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

