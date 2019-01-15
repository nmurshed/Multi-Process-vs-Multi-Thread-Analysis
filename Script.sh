#!/bin/sh







for  P in 10 20 30 40 50 100
do
for  C in 10 50 100 200 500
do
./m_thread 60 16 $P $C 0.1 30 0.8 0.2 0.2 >> data_thread$P$C.txt

done
done




