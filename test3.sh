#!/bin/bash

grep int tsh.c > original1.txt
grep "if.*NULL" tsh.c > original2.txt
ps > original3.txt
grep "int " < tsh.c > original4.txt
