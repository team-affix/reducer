:- ensure_loaded(syntax).

:- use_module(library(error)).

% copy an expression
:- table copy_expr/5.

% early checks
copy_expr(NI, _, Renames, A, _) :-
    assertion(number(NI)),
    assertion(is_list(Renames)),
    assertion(ground(A)),
    fail.

% copy a function signature
copy_expr(NI, NewNI, Renames, (A::B)~>C, (X::Y)~>Z) :-
    !,
    copy_function_signature(NI, NewNI, Renames, (A::B)~>C, (X::Y)~>Z).

% copy a function definition
copy_expr(NI, NewNI, Renames, A~>B, X~>Y) :-
    !,
    copy_function_definition(NI, NewNI, Renames, A~>B, X~>Y).

% copy function application
copy_expr(NI, NewNI, Renames, A@B, X@Y) :-
    !,
    copy_function_application(NI, NewNI, Renames, A@B, X@Y).

% copy cond
copy_expr(NI, NewNI, Renames, A?B, X?Y) :-
    !,
    copy_cond(NI, NewNI, Renames, A?B, X?Y).

% copy a renamed atom
copy_expr(NI, NI, Renames, A, X) :-
    % make sure A is an atom
    assertion(atom(A)),
    % get the rename for A
    member([A|X], Renames),
    !.

% copy an unrenamed atom
copy_expr(NI, NI, _, A, A) :-
    % make sure A is an atom
    assertion(atom(A)).


% copy a function signature
:- table copy_function_signature/5.

copy_function_signature(NI, NewNI, Renames, (A::B)~>C, (X::Y)~>Z) :-
    % make sure A is not already renamed
    assertion(\+ member([A|_], Renames)),
    % create new variable name
    atom_concat(var, NI, X),
    NI1 is NI + 1,
    % copy B
    copy_expr(NI1, NI2, Renames, B, Y),
    % copy C
    copy_expr(NI2, NewNI, [[A|X]|Renames], C, Z).


% copy a function definition
:- table copy_function_definition/5.

copy_function_definition(NI, NewNI, Renames, A~>B, X~>Y) :-
    % make sure A is not already renamed
    assertion(\+ member([A|_], Renames)),
    % create new variable name
    atom_concat(var, NI, X),
    NI1 is NI + 1,
    % copy B
    copy_expr(NI1, NewNI, [[A|X]|Renames], B, Y).


% copy a function application
:- table copy_function_application/5.

copy_function_application(NI, NewNI, Renames, A@B, X@Y) :-
    % copy A
    copy_expr(NI, NI1, Renames, A, X),
    % copy B
    copy_expr(NI1, NewNI, Renames, B, Y).


% copy a cond
:- table copy_cond/5.

copy_cond(NI, NewNI, Renames, A?B, X?Y) :-
    % copy A
    copy_expr(NI, NI1, Renames, A, X),
    % copy B
    copy_cond_body(NI1, NewNI, Renames, B, Y).


% copy a cond body
:- table copy_cond_body/5.

copy_cond_body(NI, NI, _, end, end).

copy_cond_body(NI, NewNI, Renames, A=>B//C, X=>Y//Z) :-
    % copy A
    copy_expr(NI, NI1, Renames, A, X),
    % copy B
    copy_expr(NI1, NI2, Renames, B, Y),
    % copy C
    copy_cond_body(NI2, NewNI, Renames, C, Z).

