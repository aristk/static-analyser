#!/bin/bash
for filename in `find . | egrep '\.gcov'`;
do 
  rm -rf $filename;
done
