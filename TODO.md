CMakeLists.txt:
1. clean up flex/bison generated files (rm parser/*pp)
2. after changes in tokens.l, tokens.cpp are not regenerated

Parser:
1. make flex/bison compatable with C++11
2. AST (node.h):
2.1 remove LLVM deps
2.2. add check that function was not defined previously

Language:
1. Could we have nested function declarations? Assume, no.
2. Could we have functions with same names? Assume, no
3. Could we use function before its declaration? Assume, no, it is close to
[C++ way](http://stackoverflow.com/questions/29967202/why-cant-i-define-a-function-inside-another-function).
4. What is the type of x if we use x.y somewhere in the code? Assume, structure.
    1. Which value will have X.Y if 
     ``
     X.Y = 1
     Z = X
     Z.Y = 2
     ``
     Assume 1. 
5. We could not have assignments or field of structure as function argument.
6. Variables names are char or could be strings?

Checks:
1. Check that function should return a value, if it used in assigning a variable.
2. Check usage of structure vs INT, see following incorrect example:
``
    Z = INT
    W = Z.V
``