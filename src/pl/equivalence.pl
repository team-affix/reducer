:- ensure_loaded(syntax).
:- ensure_loaded(copy).
:- ensure_loaded(reduction).


% determine if two types are equivalent up to renamings.

equivalent(Defs, A, B) :-
    % reduce A and B
    reduce(Defs, A, AR),
    reduce(Defs, B, BR),
    % determine if they are alpha-equivalent
    copy_expr([], AR, BR).

