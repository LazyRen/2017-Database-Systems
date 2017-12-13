#!/usr/bin/python3

from subprocess import Popen, PIPE
import os
from random import shuffle
from timeit import default_timer as timer
from math import log, floor
from time import sleep
import struct

EXECUTABLE_NAME = 'main'
TABLE1 = 'DATA[1]'
TABLE2 = 'DATA[10]'
JOIN_RESULT = 'join_result.txt'
INSERT_FMT_STR = 'i %d %d %s\n'
OPEN_FMT_STR = 'o %s\n'
JOIN_FMT_STR = 'j %d %d %s\n'
QUIT_FMT_STR = 'q\n'

RESULT_FMT_STR = '%d,%s,%d,%s\n'


def test_case():
    f = open('log.txt', 'w')
    p = Popen("./" + EXECUTABLE_NAME, stdin=PIPE, stdout=PIPE, shell=True)
    succ = 0

    arr = list(range(2**21))
    shuffle(arr)

    dup = arr[: 2 ** 19]
    arr1 = arr[2 ** 19: 2 ** 20]
    arr2 = arr[2 ** 20:]

    dup.sort()
    arr1.sort()
    arr2.sort()

    p.stdin.write((OPEN_FMT_STR % (TABLE1)).encode('utf-8'))
    f.write(OPEN_FMT_STR % (TABLE1))
    p.stdin.flush()
    table1 = int(p.stdout.readline().decode('utf-8').strip())
    p.stdin.write((OPEN_FMT_STR % (TABLE2)).encode('utf-8'))
    f.write(OPEN_FMT_STR % (TABLE2))
    p.stdin.flush()
    table2 = int(p.stdout.readline().decode('utf-8').strip())

    
    for i in dup:
        p.stdin.write((INSERT_FMT_STR % (table1, i, 'a' + str(i))).encode('utf-8'))
        p.stdin.write((INSERT_FMT_STR % (table2, i, 'b' + str(i))).encode('utf-8'))
        f.write(INSERT_FMT_STR % (table1, i, 'a' + str(i)))
        f.write(INSERT_FMT_STR % (table2, i, 'b' + str(i)))

    print('INSERT 1 FINISH')

    for i in arr1:
        p.stdin.write((INSERT_FMT_STR % (table1, i, 'a' + str(i))).encode('utf-8'))
        f.write(INSERT_FMT_STR % (table1, i, 'a' + str(i)))

    print('INSERT 2 FINISH')
        
    for i in arr2:
        p.stdin.write((INSERT_FMT_STR % (table2, i, 'b' + str(i))).encode('utf-8')) 
        f.write(INSERT_FMT_STR % (table2, i, 'b' + str(i)))     
    p.stdin.flush()

    print('INSERT 3 FINISH')
    p.stdin.write((JOIN_FMT_STR % (table1, table2, JOIN_RESULT)).encode('utf-8'))
    start = timer()
    f.write(JOIN_FMT_STR % (table1, table2, JOIN_RESULT))
    p.stdin.flush()
    p.stdout.readline()
    end = timer()
    print('JOIN FINISH')
    p.stdin.write(QUIT_FMT_STR.encode('utf-8'))    
    p.stdin.flush()

    sleep(0.1)

    joinr = open(JOIN_RESULT, 'r')
    lines = joinr.readlines()

    succ = 0
    for i in range(len(dup)):
        if (lines[i] == RESULT_FMT_STR %(dup[i], 'a'+str(dup[i]), dup[i], 'b'+str(dup[i]))):
            succ += 1
            
    return succ, end - start


os.system('make clean > /dev/null')
os.system('make re > /dev/null')
os.system('rm -f ' + TABLE1)
os.system('rm -f ' + TABLE2)
os.system('rm -f ' + JOIN_RESULT)


succ, elapse = test_case()

print("%d/%d %.2f" % (succ, 2 **19, elapse))