#!/bin/bash
for filename in `find . | egrep '\.gcda'`;
do
  gcov -n $filename > /dev/null;
done
