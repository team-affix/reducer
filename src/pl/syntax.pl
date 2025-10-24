% helper operator for representing function application.
:- op(600, yfx, @).

% helper operator for defining function types.
:- op(601, xfy, ~>).

% helper operator for representing named types.
:- op(602, xfy, ::).

% helper operator for representing function definitions.
:- op(603, fy, lambda).

% helper operator for representing a cond statement
:- op(604, xfy, ?).

% helper operator for representing a value->result in a cond
:- op(605, xfy, =>).

% helper operator for representing else in a cond
:- op(606, xfy, //).

