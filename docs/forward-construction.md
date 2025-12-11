# Forward Construction

I am considering a new approach to implementing AML, brought on by my new thoughts of application.

I will refer to this new approach as forward construction (FC).

In this new system, the general approach is to strive away from constraint-based search and away from on-demand argumentation/search.

In this new system, there is only one lookup procedure to our map, which is based not on what type is "reachable from" a function, but instead, what type IS an inhabitant EXACTLY.

When given a structure like a function `succ : Nat -> Nat`, instead of asking: "can this function return Nat?" we ask: IS THIS FUNCTION Nat? Of course, it is not, and so that takes me to the next part:

## Application

application is a technique by which, given a function f with some type signature, we attempt to inhabit the parameter type of the function, and then apply the function to that inhabitant in order to construct an inhabitant of the codomain. This can fail, if there are no known inhabitants of the domain (parameter type).

Thus, when we need inhabitants of a given type, we do not ask (in an on-demand way) "hey, is there a way I could GET this type?" instead, we ask "what are the CURRENT inhabitants that I KNOW OF that have this type?" and we rely on earlier inhabitation / construction to "have our back" later on.

This new system has the potential to be significantly simpler to implement, and hopefully, it will maintain the same pros/cons of using MCTS.

## Generic Idea

Throughout this system, there will be one basic idea, and it is that we have a signature (**sigma**) and that signature is added to through the use of rules of inference (of which application is an example), which preserve the consistency of the system and yield more derived inhabitants of types. These inhabitants are later made available for lookup through a map system, making the inhabitation of their types easy for when needing those inhabitants for supplying to a function as an argument, or for other reasons.

What this technique escapes is constraint-based search, where at any given point in time we "need a specific type". In this setting, we only construct things in a forward capacity as previously mentioned, and this makes no claims about the output types of certain rules of inference.

### Rules of Inference for Sigma

In our signature, we will have several different dependent type theoretic constructs defining the types of inhabitants. Depending on the constructs used, different rules of inference apply.

1. Application: Given a declaration: `t : (A : B) -> C`, try to inhabit the domain with some argument (of type `B`), (assume `b` is the inhabitant in this example), then produce a new declaration: `(t b) : C`
1. Abstraction: We can, at any point, introduce a new lambda abstraction into the system by way of:
    1. introducing a new scope frame, with the whole enclosing signature visible.
    1. introducing a new variable *k* inside this temporary scope with ANY type that MCTS chooses (could be chosen from the set of known types)
    1. return a body (note: if we want nested functions, this can be done by invoking abstraction ROI inside the first new scope from the first abstraction ROI)
    1. discharging the new variables from the scope, and wrap a lambda binder around the generated body.
1. 

## Binning Model Derivation

Due to the fact that we have no ability to ask: what could we apply to get a specific type, we must rely on the binning functions to be constructed from the ground up, at some point during derivation. Note that the binning functions expect `n` arguments, where `n` is the number of inputs in your data. The functions must output something of type `bool`.

What we could do is: create `n` lambda abstractions (new scopes), and do some derivation (rules of inference) in that new scope. Then, after some limit has been reached or MCTS chooses to finish, allow MCTS to choose an inhabitant  of `bool` to return from the function. This will act as the body of the function. However, due to the limit in probability that the work of inference that has been done led to the derivation of a bool, we can also use truthiness, where the inhabitant itself is given a truthiness score by interrogating the lambda calculus structure.

