%%%%%%%%
% unit test file for the main.pl file.
%%%%%%%%

%%%%%%%
% Test helpers listed here
%%%%%%%

:- consult(main).

%%%%%%%
% Test cases listed here
%%%%%%%

test(ground_term) :-
    ground_term(0, a).

test(ground_term) :-
    ground_term(0, X),
    X == tm0.

test(ground_term) :-
    ground_term(1, X),
    X == tm1.

test(ground_term) :-
    ground_term(1, abc).

test(ground_term) :-
    \+ ground_term(1, a@b).




test(ground_type) :-
    % this should fail because the variable does not
    %     appear as a term in a binder.
    \+ ground_type(0, _).

test(ground_type) :-
    ground_type(0, a).

test(ground_type) :-
    ground_type(0, a@b).

test(ground_type) :-
    % similar to before, _ should have been resolved by now,
    % as if it were a term in a binder, it would have been grounded,
    % meaning it was not a term in a binder.
    \+ ground_type(0, _@b).

test(ground_type) :-
    \+ ground_type(0, a@_).

test(ground_type) :-
    ground_type(0, (a::b)~>c).

test(ground_type) :-
    % here, A appears first as a term in a binder,
    %     which is the only valid place for a variable
    %     to FIRST APPEAR. (it can appear later,
    %     but first must be in a binder)
    ground_type(0, (A::b)~>c),
    A == tm0.

test(ground_type) :-
    ground_type(1, (A::b)~>c),
    A == tm1.

test(ground_type) :-
    ground_type(0, (A::b)~>A),
    A == tm0.

test(ground_type) :-
    ground_type(1, (A::b)~>A),
    A == tm1.

test(ground_type) :-
    \+ ground_type(0, (a::_)~>c).

test(ground_type) :-
    \+ ground_type(0, (a::b)~>_).

test(ground_type) :-
    % fails because the second var was not introduced in a binder.
    \+ ground_type(0, (_::_)~>c).

test(ground_type) :-
    ground_type(0, (a::b)~>(c::d)~>e).

test(ground_type) :-
    ground_type(0, (a::b)~>(C::d)~>e),
    C == tm1.

test(ground_type) :-
    ground_type(1, (a::b)~>(C::d)~>e),
    C == tm2.

test(ground_type) :-
    ground_type(0, (A::b)~>(C::d)~>e),
    A == tm0,
    C == tm1.

test(ground_type) :-
    ground_type(1, (A::b)~>(C::d)~>e),
    A == tm1,
    C == tm2.

test(ground_type) :-
    ground_type(0, (A::b)~>(C::A)~>e),
    A == tm0,
    C == tm1.

test(ground_type) :-
    % surprisingly, this works! It is because,
    % when thinking about scopes, term A has not yet been introduced
    % at the time of processing the type of the A binder, thus they
    % can have the same name, but be different variables.
    ground_type(0, (A::(B::c)~>d)~>e),
    A == tm0,
    B == tm0.
    
test(ground_type) :-
    ground_type(0, (A::b)~>A@c),
    A == tm0.

test(ground_type) :-
    ground_type(0, a@b@c).

test(ground_type) :-
    ground_type(0, (A::x)~>A@b@c),
    A == tm0.

test(ground_type) :-
    ground_type(0, (B::x)~>a@B@c),
    B == tm0.

test(ground_type) :-
    ground_type(0, (C::x)~>a@b@C),
    C == tm0.

test(ground_type) :-
    ground_type(0, myfunc@((a::b)~>c)).

test(ground_type) :-
    ground_type(0, myfunc@((A::b)~>c)),
    A == tm1.

test(ground_type) :-
    ground_type(1, myfunc@((A::b)~>c)),
    A == tm2.

test(ground_type) :-
    ground_type(0, ((A::b)~>c)@d),
    A == tm0.

    



% test the equivalent/3 predicate.
test(equivalent) :-
    equivalent([], a, a).

test(equivalent) :-
    \+ equivalent([], 1, 1).

test(equivalent) :-
    \+ equivalent([], a, b).

test(equivalent) :-
    equivalent([], a, X),
    X == a.

test(equivalent) :-
    equivalent([[a|b]], a, b).

test(equivalent) :-
    equivalent([[a|b],[c|d]], a, b).

test(equivalent) :-
    equivalent([[a|b],[c|d]], c, d).

test(equivalent) :-
    \+ equivalent([[a|b],[c|d]], a, c).

test(equivalent) :-
    \+ equivalent([[a|b],[c|d]], a, d).

test(equivalent) :-
    equivalent([[a|b],[c|d]], a, X),
    X == b.

test(equivalent) :-
    equivalent([[a|b],[c|d]], c, X),
    X == d.

test(equivalent) :-
    equivalent([[a|X]], a, b),
    X == b.

test(equivalent) :-
    equivalent([[a|X]], a, Y),
    Y == X.

test(equivalent) :-
    equivalent([], a@b, a@b).

test(equivalent) :-
    \+ equivalent([], a@b, a@c).

test(equivalent) :-
    \+ equivalent([], a@b, c@b).

test(equivalent) :-
    \+ equivalent([], a@b, c@d).

test(equivalent) :-
    equivalent([[a|c]], a@b, c@b).

test(equivalent) :-
    equivalent([[b|c]], a@b, a@c).

test(equivalent) :-
    \+ equivalent([[l|k]], a@b, c@b).

test(equivalent) :-
    equivalent([[a|X]], a@b, c@b),
    X == c.

test(equivalent) :-
    equivalent([[b|X]], a@b, a@c),
    X == c.

test(equivalent) :-
    equivalent([[a|X],[b|Y]], a@b, c@d),
    X == c,
    Y == d.

test(equivalent) :-
    equivalent([[a|c]], a@b, X@b),
    X == c.

test(equivalent) :-
    equivalent([], a@b@c, a@b@c).

test(equivalent) :-
    equivalent([[a|d]], a@b@c, d@b@c).

test(equivalent) :-
    \+ equivalent([[a|d]], a@b@c, d@e@c).

test(equivalent) :-
    equivalent([[a|d],[b|e]], a@b@c, d@e@c).

test(equivalent) :-
    equivalent([[a|d],[b|e],[c|f]], a@b@c, d@e@f).

test(equivalent) :-
    equivalent([[c|f],[a|d]], a@b@c, d@b@f).

test(equivalent) :-
    equivalent([], a@b@c, a@b@X),
    X == c.

test(equivalent) :-
    equivalent([], a@b@c, a@X@c),
    X == b.

test(equivalent) :-
    equivalent([], (a::b)~>c, (a::b)~>c).

test(equivalent) :-
    % this should fail because
    % a is already mapped to d, and
    % encountering the pi-type will cause
    % a to attempt to be mapped to d again
    \+ equivalent([[a|d]], (a::b)~>c, (d::b)~>c).

test(equivalent) :-
    equivalent([], (a::b)~>c, (d::b)~>c).

test(equivalent) :-
    equivalent([], (a::b)~>c, X),
    X =@= (_::b)~>c.

test(equivalent) :-
    equivalent([], (a::b)~>c, (d::X)~>c),
    X == b.

test(equivalent) :-
    equivalent([], (a::b)~>c, (a::b)~>X),
    X == c.

test(equivalent) :-
    \+ equivalent([], (a::b)~>c, (d::e)~>c).

test(equivalent) :-
    equivalent([[t|int]], (a::t)~>c, (b::int)~>c).

test(equivalent) :-
    equivalent([[t|int]], (a::t)~>c, (b::int)~>X),
    X == c.

test(equivalent) :-
    equivalent([[t|int]], (a::t)~>t, (b::int)~>int).

test(equivalent) :-
    \+ equivalent([[t|int]], (a::t)~>t, (b::int)~>t).

test(equivalent) :-
    equivalent([],
        (t::set)~>(x::t)   ~>sum_type@t@x,
        (ss::set)~>(yy::ss)~>sum_type@ss@yy).

test(equivalent) :-
    equivalent([],
        (t::set)~>(x::t)   ~>sum_type@t@x,
        (ss::set)~>(yy::ss)~>sum_type@ss@yy).

test(equivalent) :-
    equivalent([[t|int]],
        (x::t)  ~>sum_type@t@x,
        (yy::int)~>sum_type@int@yy).

test(equivalent) :-
    equivalent([], (t::set)~>(x::t)   ~>sum_type@t@x, R),
    R =@= (T::set)~>(X::T)~>sum_type@T@X.

test(equivalent) :-
    equivalent([],
        (x::int)~>(y::int)~>int,
        (z::int)~>(w::int)~>int).


% test main
:-
    flag(test_name, _, none),
    forall(
        clause(test(TestName), Body),
        (
            flag(test_name, PrevName, TestName),
            (
                PrevName \= TestName ->
                    flag(test_case_index, _, 0) ;
                    true
            ),
            flag(test_case_index, V, V + 1),
            write(">>>> TEST CASE STARTING:     "),
            write(TestName),
            write(" - "),
            write(V),
            nl,
            call(Body)
        )
    ),
    !.
