#!/bin/bash
a=100
while [ $a -le 295 ]
do
sleep .5
./client ./socket <<EOF 
B $(($a + 0)) GOOG 50 10
B $(($a + 1)) META 50 10
B $(($a + 2)) META 50 10
B $(($a + 3)) BABA 50 10
B $(($a + 4)) GOOG 50 10
EOF
a=$(($a + 5))
done&

b=300
while [ $b -le 495 ]
do
sleep .5
./client ./socket <<EOF 
S $(($b + 0)) META 50 10
S $(($b + 1)) BABA 50 10
S $(($b + 2)) GOOG 50 10
S $(($b + 3)) GOOG 50 10
S $(($b + 4)) META 50 10
EOF
b=$(($b + 5))
done&

c=300
while [ $c -le 495 ]
do
sleep .5
./client ./socket <<EOF 
C $(($c + 0))
C $(($c + 1))
C $(($c + 2))
C $(($c + 3))
C $(($c + 4))
EOF
c=$(($c + 5))
done&