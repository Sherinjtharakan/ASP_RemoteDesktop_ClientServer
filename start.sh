#!/bin/bash

count=0

flag=1

while [ $count -lt 12 ];
do

echo "Running the client"
./client L 8080 &

sleep 1

(( count++ ))
done
