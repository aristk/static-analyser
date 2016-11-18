CMakeLists.txt:
1. clean up flex/bison generated files (rm parser/*pp)
2. after changes in tokens.l, tokens.cpp are not regenerated

Parser:
1. make flex/bison compatible with C++11 or use AntLR

Checks:
1. Check that function should return a value, if it used in assigning a variable.