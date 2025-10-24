:- ensure_loaded(syntax).
:- ensure_loaded(grounding).

:- table reduce/3.

% define reduce/3

% function signature reduction
reduce(NI, NewNI, Defs, (A::B)~>C, (X::Y)~>Z) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    % make sure A is not already defined
    \+ member([A|_], Defs),
    % ground A
    ground_term(NI, NI1, A),
    % ground X (must be ground to prevent infinite recursion)
    ground_term(NI1, NI2, X),
    % ground B
    ground_type(NI2, NI3, B),
    % reduce B
    reduce(NI3, NI4, Defs, B, Y),
    % append the new alpha-equivalence mapping to the defs
    %     and reduce the body.
    reduce(NI4, NewNI, [[A|X]|Defs], C, Z).

% function definition reduction
reduce(NI, NewNI, Defs, A~>B, X~>Y) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    % make sure A is not already defined
    \+ member([A|_], Defs),
    % ground A
    ground_term(NI, NI1, A),
    % ground X (must be ground to prevent infinite recursion)
    ground_term(NI1, NI2, X),
    % append the new alpha-equivalence mapping to the defs
    %     and reduce the body.
    reduce(NI2, NewNI, [[A|X]|Defs], B, Y).

reduce(NI, NewNI, Defs, (A~>B)@C, R) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    % make sure A is not already defined
    \+ member([A|_], Defs),
    % ground A
    ground_term(NI, NI1, A),
    % append the new alpha-equivalence mapping to the defs
    %     and reduce the body.
    reduce(NI1, NewNI, [[A|C]|Defs], B, R).

reduce(NI, NewNI, Defs, A@B, R) :-
    % reduce A
    reduce(NI, NI1, Defs, A, AR),
    % reduce B
    reduce(NI1, NI2, Defs, B, BR),
    (
        % check: is AR a function definition?
        AR = X~>Y ->
        % then beta-reduce
        reduce(NI2, NewNI, [[X|BR]|Defs], Y, R)
        ;
        % otherwise, delta reduce
        member([AR@BR|C], Defs) ->
        reduce(NI2, NewNI, Defs, C, R)
        ;
        % otherwise, return the original term

    ).

% delta reduction if term defined
reduce(NI, NewNI, Defs, A, BR) :-
    member([A|B], Defs),
    !,
    reduce(NI, NewNI, Defs, B, BR).
    
% delta reduction if term undefined
reduce(NI, NI, _, A, A).

