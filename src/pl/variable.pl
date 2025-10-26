% gets/sets the variable index flag
variable_index_flag(Prev, New) :-
    flag(variable_index, Prev, New).
    
% define next_variable/1
% gets the next available variable name, and
%     increments the index for the next call.
next_variable(R) :-
    variable_index_flag(Index, Index+1),
    atom_concat(var, Index, R).

