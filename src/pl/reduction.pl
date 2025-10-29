:- ensure_loaded(syntax).
:- ensure_loaded(copy).
:- ensure_loaded(equivalence).


% expression reducer
% NOTE: NOT TABLED, USES GLOBAL VARIABLE INDEX

% early checks
reduce(Defs, A, _) :-
    assertion(is_list(Defs)),
    assertion(ground(A)),
    fail.

% function signature reduction
reduce(Defs, (A::B)~>C, (X::Y)~>Z) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_signature(Defs, (A::B)~>C, (X::Y)~>Z).

% function definition reduction
reduce(Defs, A~~>B, X~~>Y) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_definition(Defs, A~~>B, X~~>Y).
    
% function application reduction
reduce(Defs, A@B, R) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_function_application(Defs, A@B, R).

% cond reduction
reduce(Defs, A?B, R) :-
    % if we make it here, then
    %     this is the only way to reduce the lhs
    !,
    reduce_cond(Defs, A?B, R).

% delta reduction
reduce(Defs, A, BR) :-
    % A should be an atom
    assertion(atom(A)),
    % look up definition for A
    member([A|B], Defs),
    !,
    % create a copy of the definition,
    %     introducing new variables for all params
    copy_expr([], B, BC),
    % reduce the copy
    reduce(Defs, BC, BR).
    
% base case for reduction
reduce(_, A, A) :-
    % A must be an atom
    assertion(atom(A)).


% function signature reducer

reduce_function_signature(Defs, (A::B)~>C, (A::Y)~>Z) :-
    % reduce B
    reduce(Defs, B, Y),
    % reduce the body.
    reduce(Defs, C, Z).


% function definition reducer

reduce_function_definition(Defs, A~~>B, A~~>Y) :-
    % reduce the body.
    reduce(Defs, B, Y).


% function application reducer

% if lhs becomes a function definition
reduce_function_application(Defs, A@B, R) :-
    % reduce A
    reduce(Defs, A, X~~>Y),
    !,
    % make sure X is not already defined
    assertion(\+ member([X|_], Defs)),
    % reduce B
    reduce(Defs, B, BR),
    % then beta-reduce
    reduce([[X|BR]|Defs], Y, R).

% if lhs does not become a function definition
reduce_function_application(Defs, A@B, X@Y) :-
    % reduce A
    reduce(Defs, A, X),
    % reduce B
    reduce(Defs, B, Y).


% cond reducer

% if eval succeeds
reduce_cond(Defs, A?B, R) :-
    % reduce A
    reduce(Defs, A, AR),
    % reduce B
    reduce_cond_body(Defs, B, BR),
    eval_cond_body(Defs, AR, BR, R),
    !.

% if eval fails
reduce_cond(Defs, A?B, X?Y) :-
    % reduce A
    reduce(Defs, A, X),
    % reduce B
    reduce_cond_body(Defs, B, Y).


% cond body reducer

reduce_cond_body(_, end, end) :-
    !.
reduce_cond_body(Defs, B=>C//D, BR=>CR//DR) :-
    !,
    reduce(Defs, B, BR),
    reduce(Defs, C, CR),
    reduce_cond_body(Defs, D, DR).

% cond body evaluation
eval_cond_body(Defs, AR, B=>C//_, CR) :-
    equivalent(Defs, AR, B),
    !,
    reduce(Defs, C, CR).

eval_cond_body(Defs, AR, _=>_//D, DR) :-
    eval_cond_body(Defs, AR, D, DR).

