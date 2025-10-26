:- ensure_loaded(syntax).
:- ensure_loaded(copy).
:- ensure_loaded(equivalence).


% expression reducer
:- table reduce/5.

% function signature reduction
reduce(NI, NewNI, Defs, (A::B)~>C, (X::Y)~>Z) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_signature(NI, NewNI, Defs, (A::B)~>C, (X::Y)~>Z).

% function definition reduction
reduce(NI, NewNI, Defs, A~>B, X~>Y) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_definition(NI, NewNI, Defs, A~>B, X~>Y).
    
% function application reduction
reduce(NI, NewNI, Defs, A@B, R) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_application(NI, NewNI, Defs, A@B, R).

% cond reduction
reduce(NI, NewNI, Defs, A?B, R) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_cond(NI, NewNI, Defs, A?B, R).

% delta reduction
reduce(NI, NewNI, Defs, A, BR) :-
    % A should be an atom
    assertion(atom(A)),
    % look up definition for A
    member([A|B], Defs),
    !,
    % create a copy of the definition,
    %     introducing new variables for all params
    copy_expr(NI, NI1, [], B, BC),
    % reduce the copy
    reduce(NI1, NewNI, Defs, BC, BR).
    
% base case for reduction
reduce(NI, NI, _, A, A).


% function signature reducer
:- table reduce_function_signature/5.

reduce_function_signature(NI, NewNI, Defs, (A::B)~>C, (A::Y)~>Z) :-
    % reduce B
    reduce(NI, NI1, Defs, B, Y),
    % reduce the body.
    reduce(NI1, NewNI, Defs, C, Z).


% function definition reducer
:- table reduce_function_definition/5.

reduce_function_definition(NI, NewNI, Defs, A~>B, A~>Y) :-
    % reduce the body.
    reduce(NI, NewNI, Defs, B, Y).


% function application reducer
:- table reduce_function_application/5.

% if lhs becomes a function definition
reduce_function_application(NI, NewNI, Defs, A@B, R) :-
    % reduce A
    reduce(NI, NI1, Defs, A, X~>Y),
    !,
    % make sure X is not already defined
    assertion(\+ member([X|_], Defs)),
    % reduce B
    reduce(NI1, NI2, Defs, B, BR),
    % then beta-reduce
    reduce(NI2, NewNI, [[X|BR]|Defs], Y, R).

% if lhs does not become a function definition
reduce_function_application(NI, NewNI, Defs, A@B, X@Y) :-
    % reduce A
    reduce(NI, NI1, Defs, A, X),
    % reduce B
    reduce(NI1, NewNI, Defs, B, Y).


% cond reducer
:- table reduce_cond/5.

% if eval succeeds
reduce_cond(NI, NewNI, Defs, A?B, R) :-
    % reduce A
    reduce(NI, NI1, Defs, A, AR),
    % reduce B
    reduce_cond_body(NI1, NI2, Defs, B, BR),
    eval_cond_body(NI2, NewNI, Defs, AR, BR, R),
    !.

% if eval fails
reduce_cond(NI, NewNI, Defs, A?B, X?Y) :-
    % reduce A
    reduce(NI, NI1, Defs, A, X),
    % reduce B
    reduce_cond_body(NI1, NewNI, Defs, B, Y).


% cond body reducer
:- table reduce_cond_body/5.

reduce_cond_body(NI, NI, _, end, end).
reduce_cond_body(NI, NewNI, Defs, B=>C//D, BR=>CR//DR) :-
    !,
    reduce(NI, NI1, Defs, B, BR),
    reduce(NI1, NI2, Defs, C, CR),
    reduce_cond_body(NI2, NewNI, Defs, D, DR).

% cond body evaluation
:- table eval_cond_body/6.
eval_cond_body(NI, NewNI, Defs, AR, B=>C//_, CR) :-
    equivalent(NI, Defs, AR, B),
    !,
    reduce(NI, NewNI, Defs, C, CR).

eval_cond_body(NI, NewNI, Defs, AR, _=>_//D, DR) :-
    eval_cond_body(NI, NewNI, Defs, AR, D, DR).

