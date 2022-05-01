#!/usr/bin/env bash

 a=0

 while [ $a -ne -1 ]
 do
     a=`expr $a + 1`
     echo $a
     ./shell1.sh > NULL
     if [[ $? -eq 1 ]]; then
        echo "Times: $a"
        a=`expr -1`
     fi
 done