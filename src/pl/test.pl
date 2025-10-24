%%%%%%%%
% unit test file for the modular type checker.
%%%%%%%%

%%%%%%%
% Test helpers listed here
%%%%%%%

:- ensure_loaded(examples).

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




test(apply) :-
    apply(a, [], Args, Params, a),
    Args == [],
    Params == [].

test(apply) :-
    \+ apply(a, [], _, _, b).

test(apply) :-
    apply((a::b)~>c, [], Args, Params, (d::b)~>c),
    Args == [],
    Params == [].

test(apply) :-
    apply((a::b)~>c, [], Args, Params, c),
    Args =@= [_],
    Params == [b].

test(apply) :-
    % fails because apply calls equivalent, which fails because
    % the lhs is nonground.
    \+ apply((_::b)~>c, [], _, _, (a::b)~>c).

test(apply) :-
    apply((a::b)~>a, [], Args, Params, (f::b)~>X),
    Args == [],
    Params == [],
    X == f.

test(apply) :-
    % I originally thought it would fail, but technically,
    %     if we partially apply the lhs with the rhs, and
    %     the rhs is of type b, then the result should be
    %     of type (d::b)~>d.
    % NOTE: when partially applying ZERO times, it DOES FAIL
    % (thus, we must apply once)
    % NOTE: if wondering why it is weird, its because normally,
    %     we would not have something in the form: (a::b)~>a,
    %     since b is normally a non-universe type, making 'a' a non-type,
    %     after which it cannot exist on the rhs of the arrow ~>.
    apply((a::b)~>a, [], Args, Params, (d::c)~>d),
    Args == [(d::c)~>d],
    Params == [b].

test(apply) :-
    apply((a::b)~>a, [], Args, Params, int),
    Args == [int],
    Params == [b].

test(apply) :-
    \+ apply((a::b)~>c, [], _, _, (d::e)~>c).

test(apply) :-
    apply((a::b)~>(c::d)~>e, [], Args, Params, e),
    Args =@= [_, _],
    Params == [b, d].

test(apply) :-
    apply((a::b)~>(c::d)~>e, [], Args, Params, (f::d)~>e),
    Args =@= [_],
    Params == [b].

test(apply) :-
    apply(a, [], Args, Params, X),
    Args == [],
    Params == [],
    X == a.

test(apply) :-
    apply((a::b)~>c, [], Args, Params, (X::Y)~>Z),
    Args == [],
    Params == [],
    var(X), % X remains unbound as it is only used as an identifier.
    Y == b, % Y gets bound as its counterpart, b, is a concrete type
    Z == c. % Z gets bound as its counterpart, c, is a concrete type

test(apply) :-
    apply((a::b)~>c@a, [], Args, Params, (X::b)~>Y),
    Args == [],
    Params == [],
    var(X),
    Y == c@X. % equivalent to c@a except for name (a became X)

 test(apply) :-
    apply(a@b, [], Args, Params, X),
    Args == [],
    Params == [],
    X == a@b.

test(apply) :-
    apply(a@b, [], Args, Params, a@X),
    Args == [],
    Params == [],
    X == b.

test(apply) :-
    apply(a@b@c, [], Args, Params, a@b@X),
    Args == [],
    Params == [],
    X == c.

test(apply) :-
    apply(a@b@c, [], Args, Params, a@X@c),
    Args == [],
    Params == [],
    X == b.

test(apply) :-
    apply((t::s)~>(x::t)~>sum_type@t@x, [], Args, Params, sum_type@int@zero),
    Args == [int, zero],
    Params == [s, int].

test(apply) :-
    apply((t::s)~>(x::t)~>sum_type@t@x, [], Args, Params, sum_type@X@Y),
    Args = [Z|Rest],
    Z == X,
    var(Y),
    Rest =@= [_],
    Params == [s, X].

test(apply) :-
    apply((t::s)~>(x::t)~>sum_type@t@x, [], Args, Params, K@X@Y),
    Args = [Z|Rest],
    Z == X,
    Rest =@= [_],
    var(Y),
    Params == [s, X],
    K == sum_type.




test(express_application) :-
    express_application(a, [], a).

test(express_application) :-
    express_application(a, [b], a@b).

test(express_application) :-
    express_application(a, [b, c], a@b@c).

test(express_application) :-
    express_application(a, [X], a@X),
    var(X).

test(express_application) :-
    express_application(a@b, [c], a@b@c).

test(express_application) :-
    express_application(a, [], X),
    X == a.
    
test(express_application) :-
    express_application(a, [b], X),
    X == a@b.

    
    

test(level) :-
    level(lzero).

test(level) :-
    level(lsuc@lzero).

test(level) :-
    level(lsuc@X),
    X == lzero.

test(level) :-
    level(lsuc@(lsuc@lzero)).
    
    
    
    
test(maxlevel) :-
    maxlevel(lzero, lzero, lzero).

test(maxlevel) :-
    maxlevel(lzero, lsuc@lzero, lsuc@lzero).

test(maxlevel) :-
    maxlevel(lsuc@lzero, lzero, lsuc@lzero).

test(maxlevel) :-
    maxlevel(lsuc@lzero, lsuc@lzero, lsuc@lzero).

test(maxlevel) :-
    maxlevel(lsuc@X, lzero, lsuc@X),
    var(X).

test(maxlevel) :-
    maxlevel(lzero, lsuc@(lsuc@lzero), lsuc@(lsuc@lzero)).

test(maxlevel) :-
    maxlevel(lsuc@(lsuc@lzero), lzero, lsuc@(lsuc@lzero)).
    
test(maxlevel) :-
    maxlevel(lsuc@(lsuc@lzero), lsuc@(lsuc@lzero), lsuc@(lsuc@lzero)).



    
test(default_environment) :-
    default_environment(Env),
    Env == [
        [level|set@lzero],
        [lzero|level],
        [lsuc|(l::level) ~> level],
        [set|(l::level) ~> set@(lsuc@l)]
    ].




test(can_declare) :-
    default_environment(Env),
    can_declare(10, a, level, Env).

test(can_declare) :-
    default_environment(Env),
    % fails as level2 is not a valid type.
    \+ can_declare(10, a, level2, Env).

test(can_declare) :-
    default_environment(Env),
    can_declare(10, a, set@lzero, Env).

test(can_declare) :-
    default_environment(Env),
    % set already defined
    \+ can_declare(10, set, level, Env).

test(can_declare) :-
    default_environment(Env),
    can_declare(10, a, (l::level) ~> level, Env).

test(can_declare) :-
    default_environment(Env),
    can_declare(10, X, level, Env),
    X == tm4.

test(can_declare) :-
    default_environment(Env),
    % fails as _ is not a valid type. this is because
    %     if a variable appears in a type, it must be introduced in a binder.
    \+ can_declare(10, a, _, Env).

test(can_declare) :-
    default_environment(Env),
    can_declare(10, a, (l::set@lzero) ~> level, Env).

test(can_declare) :-
    default_environment(Env),
    can_declare(10, int, set@lzero, Env).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, zero, int, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, square, (x::int) ~> int, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, square, (X::int) ~> int, NewEnv),
    X == tm5.

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, sum_type, (t::set@lzero)~>(x::t)~>set@lzero, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, default_sum_type, (t::set@lzero)~>(x::t)~>sum_type@t@x, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    % fails as sum_type@t@t is not a valid type.
    \+ can_declare(10, default_sum_type, (t::set@lzero)~>(x::t)~>sum_type@t@t, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    % fails as sum_type@x@x is not a valid type.
    \+ can_declare(10, default_sum_type, (t::set@lzero)~>(x::t)~>sum_type@x@x, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    % fails as sty@t@x is not a valid type.
    \+ can_declare(10, default_sum_type, (t::set@lzero)~>(x::t)~>sty@t@x, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    can_declare(10, my_sum_val, sum_type@int@zero, NewEnv).

test(can_declare) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    append(Env, Additions, NewEnv),
    % fails as sum_type@int@int is not a valid type.
    \+ can_declare(10, my_sum_val, sum_type@int@int, NewEnv).




test(declarea) :-
    default_environment(Env),
    declarea(10, [], Env, NewEnv),
    NewEnv == Env.

test(declarea) :-
    default_environment(Env),
    % fails because the depth limit is too shallow for typechecking.
    \+ declarea(0, [[a|level]], Env, _).

test(declarea) :-
    default_environment(Env),
    declarea(10, [[a|level]], Env, NewEnv),
    NewEnv == [
        [a|level]|Env
    ].

test(declarea) :-
    default_environment(Env),
    declarea(10, [[a|level], [b|level]], Env, NewEnv),
    NewEnv == [
        [b|level],
        [a|level]|Env
    ].

test(declarea) :-
    default_environment(Env),
    declarea(10, [[bool|set@lzero], [true|bool], [false|bool]], Env, NewEnv),
    NewEnv == [
        [false|bool],
        [true|bool],
        [bool|set@lzero]|Env
    ].

test(declarea) :-
    default_environment(Env),
    % fails as bool is not a valid type yet.
    \+ declarea(10, [[true|bool], [false|bool], [bool|set@lzero]], Env, _).

test(declarea) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [not|(x::bool)~>bool]
    ],
    declarea(10, Additions, Env, NewEnv),
    reverse(Additions, ReverseAdditions),
    append(ReverseAdditions, Env, NewEnv).

test(declarea) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    declarea(10, Additions, Env, NewEnv),
    reverse(Additions, ReverseAdditions),
    append(ReverseAdditions, Env, NewEnv).

test(declarea) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarea(10, Additions, Env, NewEnv),
    reverse(Additions, ReverseAdditions),
    append(ReverseAdditions, Env, NewEnv).


    
    
test(declarez) :-
    default_environment(Env),
    declarez(10, [], Env, NewEnv),
    NewEnv == Env.

test(declarez) :-
    default_environment(Env),
    % fails because the depth limit is too shallow for typechecking.
    \+ declarez(0, [[a|level]], Env, _).

test(declarez) :-
    default_environment(Env),
    declarez(10, [[a|level]], Env, NewEnv),
    append(Env, [[a|level]], NewEnv).

test(declarez) :-
    default_environment(Env),
    declarez(10, [[a|level], [b|level]], Env, NewEnv),
    append(Env, [[a|level], [b|level]], NewEnv).

test(declarez) :-
    default_environment(Env),
    declarez(10, [[bool|set@lzero], [true|bool], [false|bool]], Env, NewEnv),
    append(Env, [[bool|set@lzero], [true|bool], [false|bool]], NewEnv).

test(declarez) :-
    default_environment(Env),
    % fails as bool is not a valid type yet.
    \+ declarez(10, [[true|bool], [false|bool], [bool|set@lzero]], Env, _).

test(declarez) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [not|(x::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    append(Env, Additions, NewEnv).

test(declarez) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    declarez(10, Additions, Env, NewEnv),
    append(Env, Additions, NewEnv).

test(declarez) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    append(Env, Additions, NewEnv).




test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, lzero, T),
    T == level.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    % lzero1 is not a valid term.
    \+ typecheck(10, NewEnv, lzero1, _).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, lsuc@lzero, T),
    T == level.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    % lzero1 is not a valid term.
    \+ typecheck(10, NewEnv, lsuc@lzero1, _).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    % level has the wrong type for being an argument of lsuc.
    \+ typecheck(10, NewEnv, lsuc@level, _).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, lsuc, T),
    T =@= (_::level)~>level.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, set@lzero, T),
    T == set@(lsuc@lzero).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, set, T),
    T =@= (X::level)~>set@(lsuc@X).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, level, T),
    T == set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, level, T),
    T == set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, int, T),
    T == set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [int|set@lzero],
        [zero|int]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, zero, T),
    T == int.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, sum_type, T),
    T =@= (X::set@lzero)~>(_::X)~>set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, sum_type@level, T),
    T =@= (_::level)~>set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, sum_type@level@lzero, T),
    T == set@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, default_sum_type, T),
    T =@= (X::set@lzero)~>(Y::X)~>sum_type@X@Y.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, default_sum_type@level, T),
    T =@= (X::level)~>sum_type@level@X.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, default_sum_type@level@lzero, T),
    T == sum_type@level@lzero.

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(10, NewEnv, default_sum_type@level@(lsuc@lzero), T),
    T == sum_type@level@(lsuc@lzero).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [sum_type|(t::set@lzero)~>(x::t)~>set@lzero],
        [default_sum_type|(t::set@lzero)~>(x::t)~>sum_type@t@x]
    ],
    declarez(10, Additions, Env, NewEnv),
    % level has the wrong type for being an argument of lsuc.
    \+ typecheck(10, NewEnv, default_sum_type@level@(lsuc@level), _).

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(10, NewEnv, V, bool), Vs),
    Vs == [false, true].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [not|(x::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(2, NewEnv, V, bool), Vs),
    Vs == [false, true, not@false, not@true].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [and|(x::bool)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(2, NewEnv, V, bool), Vs),
    Vs == [false, true, and@false@false, and@false@true, and@true@false, and@true@true].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [int|set@lzero],
        [zero|int],
        [func|(x::int)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(2, NewEnv, V, bool), Vs),
    Vs == [
        false,
        true,
        func@zero@false,
        func@zero@true
    ].

test(typecheck) :-
    print("24?"),
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [int|set@lzero],
        [zero|int],
        [succ|(x::int)~>int],
        [func|(x::int)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    % partial application!
    findall(V, typecheck(3, NewEnv, V, (z::bool)~>bool), Vs),
    print(Vs),
    Vs == [
        func@zero,
        func@(succ@zero)
    ].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [int|set@lzero],
        [zero|int],
        [succ|(x::int)~>int],
        [func|(x::int)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(3, NewEnv, V, (z::int)~>(w::bool)~>bool), Vs),
    Vs == [
        func
    ].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [int|set@lzero],
        [zero|int],
        [succ|(x::int)~>int],
        [func|(x::int)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    findall(V, typecheck(3, NewEnv, V, (_::int)~>(_::bool)~>bool), Vs),
    Vs == [
        func
    ].

test(typecheck) :-
    default_environment(Env),
    Additions = [
        [bool|set@lzero],
        [false|bool],
        [true|bool],
        [int|set@lzero],
        [zero|int],
        [succ|(x::int)~>int],
        [func|(x::int)~>(y::bool)~>bool]
    ],
    declarez(10, Additions, Env, NewEnv),
    typecheck(3, NewEnv, func@zero, T),
    T =@= (_::bool)~>bool.





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
