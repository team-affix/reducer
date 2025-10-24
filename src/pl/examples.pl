:- ensure_loaded(environment).


my_env(Env) :-
    default_environment(DefaultEnv),
    declarez(10, [
        [bool|set@lzero],
        [int|set@lzero],
        [double|set@lzero],
        [string|set@lzero],
        [vector|(t :: set@lzero) ~> set@lzero],
        [sum_type|(t :: set@lzero) ~> (x :: t) ~> set@lzero],
        [map|(k :: set@lzero) ~> (v :: set@lzero) ~> set@lzero],
        [false|bool],
        [true|bool],
        [lessthan|(lhs :: int) ~> (rhs :: int) ~> bool],
        ['0'|int],
        [default_int|int],
        [square|(x :: int) ~> int],
        [suc|(x :: int) ~> int],
        [size|(t :: set@lzero) ~> (v :: vector@t) ~> int],
        [default_double|double],
        [default_string|string],
        [default_vector|(t :: set@lzero) ~> vector@t],
        [cons|(t :: set@lzero) ~> (v :: vector@t) ~> (x :: t) ~> vector@t],
        [default_sum_type|(t :: set@lzero) ~> (x :: t) ~> sum_type@t@x]
    ], DefaultEnv, Env).

