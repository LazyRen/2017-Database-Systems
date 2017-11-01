#!/usr/bin/python3

'''
	test program
'''

from subprocess import Popen, PIPE
import os
from random import shuffle
from timeit import default_timer as timer
FIND_RESULT_FMTS = "Key: %d, Value: %s"
INSERT_CMD_FMTS = "i %d %s\n"
FIND_CMD_FMTS = "f %d\n"
RESULT_FMTS = "Result: %d/%d (%.2f) %.2f secs"

def test_case(arr):
	f = open("log.txt", "w")
	p = Popen("./main", stdin=PIPE, stdout=PIPE, shell=True)
	succ = 0;

	# insert start
	start = timer()
	for i in arr:
		input_d = INSERT_CMD_FMTS % (i, 'a' + str(i))
		p.stdin.write(input_d.encode("utf-8"))
		f.write(input_d)
		p.stdin.flush()
	end = timer()

	# validate 
	for i in arr:
		input_d = FIND_CMD_FMTS % (i)
		p.stdin.write(input_d.encode("utf-8"))
		p.stdin.flush()
		result = p.stdout.readline().decode('utf-8').strip()
		if (result == (FIND_RESULT_FMTS % (i, 'a' + str(i))).strip()):
			succ += 1   

	p.kill()
	f.close()

	return succ, end - start

def test_case_seq(casename, case_size):
	print(casename + " Test")
	
	result, elapse = test_case(range(case_size))
	
	print(RESULT_FMTS % (result, case_size, float(result)/case_size * 100, elapse))
	os.rename("test.db", "last_test.db")
	

def test_case_rnd(casename, case_size):
	print(casename + " Test")
	arr = list(range(case_size))
	shuffle(arr)
	
	result, elapse = test_case(arr)
	
	print(RESULT_FMTS % (result, case_size, float(result)/case_size * 100, elapse))
	os.rename("test.db", "last_test.db")

os.system('make clean > /dev/null')
os.system('make main > /dev/null')

try:
	os.remove("test.db")
except:
	pass

print("---------- Sequantial Insert Test -----------")
# test_case_seq("Small(2^10)", 2 ** 10)
# test_case_seq("Medium(2^15)", 2 ** 15)
# test_case_seq("Large(2^20)", 2 ** 20)


print("--------- Random Insert Test ------------")
# test_case_rnd("Small(2^10)", 2 ** 10)
# test_case_rnd("Medium(2^15)", 2 ** 15)
test_case_rnd("Large(2^20)", 2 ** 20)