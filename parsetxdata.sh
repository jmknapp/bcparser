#!/bin/ksh
lower=$1
upper=$2
let i=lower

while [ $i -le $upper ]
do
    print "$i \c" 
    t0=`date +%s`
    /home/jmknapp/bcparser/parsetxdata $i >> /home/jmknapp/parsetxdata.out 2>&1
    t1=`date +%s`
    let t=t1-t0
    print $t
    let i=i+1
done
