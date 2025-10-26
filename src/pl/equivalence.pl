:- ensure_loaded(syntax).
:- ensure_loaded(copy).
:- ensure_loaded(reduction).


% determine if two types are equivalent up to renamings.
:- table equivalent/3.

equivalent(_, A, A) :-
    % if they are exactly the same, dont bother reducing.
    % (this handles the case where lhs xor rhs are variable)
    ground(A),
    !.

equivalent(Defs, A, B) :-
    % reduce A and B
    reduce(Defs, A, AR),
    reduce(Defs, B, BR),
    % copy both with a COMMON starting index
    variable_index_flag(VI, VI),
    copy_expr([], AR, ARC),
    % (reset the variable index flag)
    variable_index_flag(_ , VI),
    copy_expr([], BR, BRC),
    % check if they are equivalent
    ARC == BRC.

