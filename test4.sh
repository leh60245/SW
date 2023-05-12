#!/bin/bash

if diff -q result1.txt original1.txt >/dev/null ; then
	echo "테스트1 성공"
else
	echo "테스트1 실패"
fi

if diff -q result2.txt original2.txt >/dev/null ; then
	echo "테스트2 성공"
else
	echo "테스트2 실패"
fi

if diff -q result3.txt original3.txt >/dev/null ; then
	echo "테스트3 성공"
else
	echo "테스트3 실패"
fi

if diff -q result4.txt original4.txt >/dev/null ; then
	echo "테스트4 성공"
else
	echo "테스트4 실패"
fi
