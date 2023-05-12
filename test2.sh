#!/usr/bin/expect -f

spawn ./tsh

expect "tsh>"
send "grep int tsh.c > result1.txt\r"

expect "tsh>"
send "grep \"if.*NULL\" tsh.c > result2.txt\r"

expect "tsh>"
send "ps > result3.txt\r"

expect "tsh>"
send "grep \"int \" < tsh.c > result4.txt\r"

expect "tsh>"
send "exit\r"

expect eof
