%%%%%%%%
% unit test file for the main.pl file.
%%%%%%%%

%%%%%%%
% Test helpers listed here
%%%%%%%

wipe_database :-
    retractall(theorem(_, _, _)),
    retractall(redir(_, _, _)),
    !.

test(TestName, LastCase) :-
    write(">>>> TEST STARTING: "),
    write(TestName),
    nl,
    test_case(TestName, LastCase).

test_case(TestName, LastCase) :-
    forall(
        between(0, LastCase, I),
        (
            write(">>>> TEST CASE STARTING:     "),
            write(I),
            nl,
            Function =.. [TestName, I],
            call(Function)
        )
    ).


:- consult(main).

%%%%%%%
% Test cases listed here
%%%%%%%

% test the equivalent/3 predicate.
test_equivalent(0) :-
    equivalent([], a, a).

test_equivalent(1) :-
    \+ equivalent([], 1, 1).

test_equivalent(2) :-
    \+ equivalent([], a, b).

test_equivalent(3) :-
    equivalent([], a, X),
    X == a.

test_equivalent(4) :-
    equivalent([[a|b]], a, b).

test_equivalent(5) :-
    equivalent([[a|b],[c|d]], a, b).

test_equivalent(6) :-
    equivalent([[a|b],[c|d]], c, d).

test_equivalent(7) :-
    \+ equivalent([[a|b],[c|d]], a, c).

test_equivalent(8) :-
    \+ equivalent([[a|b],[c|d]], a, d).

test_equivalent(9) :-
    equivalent([[a|b],[c|d]], a, X),
    X == b.

test_equivalent(10) :-
    equivalent([[a|b],[c|d]], c, X),
    X == d.

test_equivalent(11) :-
    equivalent([[a|X]], a, b),
    X == b.

test_equivalent(12) :-
    equivalent([[a|X]], a, Y),
    Y == X.

test_equivalent(13) :-
    equivalent([], a@b, a@b).

test_equivalent(14) :-
    \+ equivalent([], a@b, a@c).

test_equivalent(15) :-
    \+ equivalent([], a@b, c@b).

test_equivalent(16) :-
    \+ equivalent([], a@b, c@d).

test_equivalent(17) :-
    equivalent([[a|c]], a@b, c@b).

test_equivalent(18) :-
    equivalent([[b|c]], a@b, a@c).

test_equivalent(19) :-
    \+ equivalent([[l|k]], a@b, c@b).

test_equivalent(20) :-
    equivalent([[a|X]], a@b, c@b),
    X == c.

test_equivalent(21) :-
    equivalent([[b|X]], a@b, a@c),
    X == c.

test_equivalent(22) :-
    equivalent([[a|X],[b|Y]], a@b, c@d),
    X == c,
    Y == d.

test_equivalent(23) :-
    equivalent([[a|c]], a@b, X@b),
    X == c.

test_equivalent(24) :-
    equivalent([], a@b@c, a@b@c).

test_equivalent(25) :-
    equivalent([[a|d]], a@b@c, d@b@c).

test_equivalent(26) :-
    \+ equivalent([[a|d]], a@b@c, d@e@c).

test_equivalent(27) :-
    equivalent([[a|d],[b|e]], a@b@c, d@e@c).

test_equivalent(28) :-
    equivalent([[a|d],[b|e],[c|f]], a@b@c, d@e@f).

test_equivalent(29) :-
    equivalent([[c|f],[a|d]], a@b@c, d@b@f).

test_equivalent(30) :-
    equivalent([], a@b@c, a@b@X),
    X == c.

test_equivalent(31) :-
    equivalent([], a@b@c, a@X@c),
    X == b.

test_equivalent(32) :-
    equivalent([], (a::b)~>c, (a::b)~>c).

test_equivalent(33) :-
    % this should fail because
    % a is already mapped to d, and
    % encountering the pi-type will cause
    % a to attempt to be mapped to d again
    \+ equivalent([[a|d]], (a::b)~>c, (d::b)~>c).

test_equivalent(34) :-
    equivalent([], (a::b)~>c, (d::b)~>c).

test_equivalent(35) :-
    equivalent([], (a::b)~>c, X),
    X =@= (_::b)~>c.

test_equivalent(36) :-
    equivalent([], (a::b)~>c, (d::X)~>c),
    X == b.

test_equivalent(37) :-
    equivalent([], (a::b)~>c, (a::b)~>X),
    X == c.

test_equivalent(38) :-
    \+ equivalent([], (a::b)~>c, (d::e)~>c).

test_equivalent(39) :-
    equivalent([[t|int]], (a::t)~>c, (b::int)~>c).

test_equivalent(40) :-
    equivalent([[t|int]], (a::t)~>c, (b::int)~>X),
    X == c.

test_equivalent(41) :-
    equivalent([[t|int]], (a::t)~>t, (b::int)~>int).

test_equivalent(42) :-
    \+ equivalent([[t|int]], (a::t)~>t, (b::int)~>t).

test_equivalent(43) :-
    equivalent([],
        (t::set)~>(x::t)   ~>sum_type@t@x,
        (ss::set)~>(yy::ss)~>sum_type@ss@yy).

test_equivalent(44) :-
    equivalent([],
        (t::set)~>(x::t)   ~>sum_type@t@x,
        (ss::set)~>(yy::ss)~>sum_type@ss@yy).

test_equivalent(45) :-
    equivalent([[t|int]],
        (x::t)  ~>sum_type@t@x,
        (yy::int)~>sum_type@int@yy).

test_equivalent(46) :-
    equivalent([], (t::set)~>(x::t)   ~>sum_type@t@x, R),
    R =@= (T::set)~>(X::T)~>sum_type@T@X.

test_equivalent(47) :-
    equivalent([],
        (x::int)~>(y::int)~>int,
        (z::int)~>(w::int)~>int).

% test main
:-
    TESTS = [
        [test_equivalent|47]
    ],

    forall(
        member([TestName|LastCase], TESTS),
        test(TestName, LastCase)
    ),
    !.
