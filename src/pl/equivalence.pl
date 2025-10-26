:- ensure_loaded(syntax).
:- ensure_loaded(copy).
:- ensure_loaded(reduction).


% determine if two types are equivalent up to renamings.
:- table equivalent/4.

equivalent(NI, Defs, A, B) :-
    % reduce A and B
    reduce(NI , NI1, Defs, A, AR),
    reduce(NI1, NI2, Defs, B, BR),
    % copy both with a COMMON starting index
    copy_expr(NI2, _, [], AR, ARC),
    copy_expr(NI2, _, [], BR, BRC),
    % check if they are equivalent
    ARC == BRC.

