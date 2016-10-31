# A Static Analyser for simple dynamic language
Given a dynamic language (i.e. variables are used without declarations) that is described in following section.
Goal is to create simple static analyser for this language. 
## Language specification
We could have functions and following operators:
```
X = Y
X = INT
X = Y.Z
X.Y = Z
X.Y = INT
X = Y == Z
X = Y != Z
func <func>(X, Y, …)
X = <func>(X, Y, …)
return X
```
There is no dedicated variable declaration: if variable is not used previously, then it is just newly created. All variables are local and integers.

## Scanner and Parser
Easiest, fastest and widely used way to create parsers for new languages is flex/bison. To create a rough parser for the language an adaptation of http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/ was made. In the future it is planned to be improved.
