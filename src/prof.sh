#!/bin/sh
execute="cft.out"
cmd="./$execute $@ && gprof ./$execute gmon.out > prof_$execute.txt && rm gmon.out && vim prof_$execute.txt"
echo $cmd
eval $cmd

