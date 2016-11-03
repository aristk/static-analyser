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
func <func>(X, Y, …) // function declaration
<func>(X, Y, …) // call of function without return
X = <func>(X, Y, …) // call of function with return
return X
```
There is no dedicated variable declaration: if variable is not used previously, then it is just newly created. All 
variables are local and integers. 

Function are [called by value](https://www.codingunit.com/c-tutorial-call-by-value-or-call-by-reference). 
[Side effect](https://en.wikipedia.org/wiki/Side_effect_(computer_science)) could be obtained only when you modify a 
field of an argument:
```
func foo(a, b) {
 a.x = b
}
```

## Scanner and Parser
Easiest, fastest and widely used way to create parsers for new languages is flex/bison. To create a rough parser for 
the language an adaptation of [Toy LLVM compiler](http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/) 
was made. In the future it is 
planned to be improved.
