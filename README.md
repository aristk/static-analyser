[![license: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![linux build](https://travis-ci.org/aristk/static-analyser.svg?branch=master)](https://travis-ci.org/aristk/static-analyser)
<a href="https://scan.coverity.com/projects/aristk-static-analyser">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/10890/badge.svg"/>
</a>
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
### Language limitations

1. There is no dedicated variable declaration: if variable is not used previously, then it is just newly created. All 
variables are local and integers. 

2. It is possible to assign a = 1 even if a.x = 1 previously. Both are just not connected valid names.

3. Function are [called by value](https://www.codingunit.com/c-tutorial-call-by-value-or-call-by-reference). 
[Side effect](https://en.wikipedia.org/wiki/Side_effect_(computer_science)) could be obtained only when you modify a 
field of an argument:
```
func foo(a, b) {
 a.x = b
}
```
4. We could not have nested function declarations.
5. We could not have functions with same names.
6. We could use function before its declaration. It is close to
   [C++ way](http://stackoverflow.com/questions/29967202/why-cant-i-define-a-function-inside-another-function).
7. We could not have assignments or field of structure as function argument.
8. Variables names are strings.

## Static analyses
Main intension of static analyses is to find not trivial constant values that come from comparisions. For example, in the following example, variable X will be always equal to 1:
```
Y = Z
X = Y == Z
```
Due to function invocations this task is not very simple.

## Scanner and Parser
Easiest, fastest and widely used way to create parsers for new languages is flex/bison. To create a rough parser for 
the language an adaptation of [Toy LLVM compiler](http://gnuu.org/2009/09/18/writing-your-own-toy-compiler/) 
was made. In the future it is 
planned to be improved.
