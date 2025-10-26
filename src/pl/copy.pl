:- ensure_loaded(syntax).
:- ensure_loaded(variable).

:- use_module(library(error)).

% copy an expression
% NOTE: NOT TABLED, USES GLOBAL VARIABLE INDEX

% early checks
copy_expr(Renames, A, _) :-
    assertion(is_list(Renames)),
    assertion(ground(A)),
    fail.

% copy a function signature
copy_expr(Renames, (A::B)~>C, (X::Y)~>Z) :-
    !,
    copy_function_signature(Renames, (A::B)~>C, (X::Y)~>Z).

% copy a function definition
copy_expr(Renames, A~>B, X~>Y) :-
    !,
    copy_function_definition(Renames, A~>B, X~>Y).

% copy function application
copy_expr(Renames, A@B, X@Y) :-
    !,
    copy_function_application(Renames, A@B, X@Y).

% copy cond
copy_expr(Renames, A?B, X?Y) :-
    !,
    copy_cond(Renames, A?B, X?Y).

% copy a renamed atom
copy_expr(Renames, A, X) :-
    % make sure A is an atom
    assertion(atom(A)),
    % get the rename for A
    member([A|X], Renames),
    !.

% copy an unrenamed atom
copy_expr(_, A, A) :-
    % make sure A is an atom
    assertion(atom(A)).


% copy a function signature

copy_function_signature(Renames, (A::B)~>C, (X::Y)~>Z) :-
    % make sure A is not already renamed
    assertion(\+ member([A|_], Renames)),
    % create new variable name
    next_variable(X),
    % copy B
    copy_expr(Renames, B, Y),
    % copy C
    copy_expr([[A|X]|Renames], C, Z).


% copy a function definition

copy_function_definition(Renames, A~>B, X~>Y) :-
    % make sure A is not already renamed
    assertion(\+ member([A|_], Renames)),
    % create new variable name
    next_variable(X),
    % copy B
    copy_expr([[A|X]|Renames], B, Y).


% copy a function applications

copy_function_application(Renames, A@B, X@Y) :-
    % copy A
    copy_expr(Renames, A, X),
    % copy B
    copy_expr(Renames, B, Y).


% copy a cond

copy_cond(Renames, A?B, X?Y) :-
    % copy A
    copy_expr(Renames, A, X),
    % copy B
    copy_cond_body(Renames, B, Y).


% copy a cond body

copy_cond_body(_, end, end) :-
    !.

copy_cond_body(Renames, A=>B//C, X=>Y//Z) :-
    % copy A
    copy_expr(Renames, A, X),
    % copy B
    copy_expr(Renames, B, Y),
    % copy C
    copy_cond_body(Renames, C, Z).

