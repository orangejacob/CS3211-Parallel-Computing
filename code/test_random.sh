#!/bin/bash
a=1000
while [ $a -le 2995 ]
do
sleep .5
./client ./socket <<EOF 
B $(($a + 0)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($a + 1)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($a + 2)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($a + 3)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($a + 4)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
a=$(($a + 5))
done&

b=3000
while [ $b -le 4995 ]
do
sleep 1
./client ./socket <<EOF 
B $(($b + 0)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($b + 1)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($b + 2)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($b + 3)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($b + 4)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
b=$(($b + 5))
done&

c=5000
while [ $c -le 6995 ]
do
sleep .2
./client ./socket <<EOF 
B $(($c + 0)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($c + 1)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($c + 2)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($c + 3)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
B $(($c + 4)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
c=$(($c + 5))
done&

d=7000
while [ $d -le 8995 ]
do
sleep .5
./client ./socket <<EOF 
S $(($d + 0)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($d + 1)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($d + 2)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($d + 3)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($d + 4)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
d=$(($d + 5))
done

e=9000
while [ $e -le 10995 ]
do
sleep 1
./client ./socket <<EOF 
S $(($e + 0)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($e + 1)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($e + 2)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($e + 3)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($e + 4)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
e=$(($e + 5))
done&

f=12000
while [ $f -le 13995 ]
do
sleep .5
./client ./socket <<EOF 
S $(($f + 0)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($f + 1)) GOOG $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($f + 2)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($f + 3)) META $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
S $(($f + 4)) BABA $((25 + $RANDOM % 100)) $((25 + $RANDOM % 100))
EOF
f=$(($f + 5))
done