# reducer

C++ repository for boolean algebraic reduction. Utilizes Monte Carlo Tree Search for finding approximately optimal forms.

This project uses clangd as intellisense and g++ as compiler.

clangd expects c++12 stdlib headers to be installed. This can be done with:

```
sudo apt update
sudo apt install libstdc++-12-dev
```

## Full System Definition

This is a machine learning system designed to derive discrete function application models of data.

### Binning Model
This machine learning system utilizes something known as a "binning model" which restricts the set of possible models to those which PERFECTLY fit the training data. This makes the reward for monte carlo tree search non-competitive, and we can thus search for minimal models.

### Associative Model
This machine learning system is designed to produce associative models of data. This means, if you have (input,output) pairs, we learn an association between inputs and outputs, RATHER THAN learning HOW TO PREDICT outputs from inputs. (Generally, with associative models, the association can go backwards, to predict input from output, etc.) However, it is important to note that it is STILL POSSIBLE using this system, to have it generate predictive models (ones which only function in the forward sense).

### Helper Functions
This system allows for the procedural creation and definition of helper functions, which can have both parameters and captured values.

Uses for helper functions are:
1. For variables (nullary helpers that have captured some external value in the program)
2. For functional factoring to reduce graph complexity.
3. For arguments supplied to a given function, those arguments will be (since they are also variables) in the form of local helper functions.

### Operators
All operators have:
- An arity
- A definition



FUTURE NOTE: COMBINE bool_op_data AND helpers TO BE THE SAME THING.

HELPERS CAN BE DEFINED BY HUMANS AS:
- Using external computation (in which case the human will need to specify the "complexity" of the function)
- Using ONLY prespecified helper functions (in which case the program will compute the complexity automatically)



Types:
- Types are stored in a global array
- Type codes are used to reference them by index

Ops:
- Ops are stored in a global array
- Op codes are used to reference them by index
