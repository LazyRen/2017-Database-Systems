# Disk Based B+ Tree
original BPT source code is written by Amittai Aviram (http://www.amittai.com)

modified by Dae In Lee
# Features

This project implemented B+ Tree data structure to construct single-user diskbased DB.

Each "data" is constructed with <key:value>

The following commands are available to manipulate DB.

	init_db(num_buf);
	shutdown_db();
	open_table(pathname);
	close_table(table_id);

The following commands are available to insert, delete or search data.

	print_tree(table_id)
	find(table_id, key)
	find_and_print(table_id, key)
	insert(table_id, key, value)
	delete(table_id, key)

* **You must initialize DB** by calling init_db(num_buf).
Number of buffers to be used will be decided according to the num_buf variable.
* each DB(program) can open up to 10 tables by calling open_table(pathname).
you must open at least one table before any command execution.
* If *toggle_bs* from *page.h* is set true and num_buf >= 100, program will use
binary search algorithm for the buffer management.
It may decrease the performance of DB if insertion/deletion is the majority of operation
thus it is set to false as default.
* data is sorted and stored into B+ tree by key
* key is int64_t type, and value must be less than 120 characters
* Each internal page can hold up to 248 entries(child page)
* Each leaf page can hold up to 31 recordes.
* Hash Table has been implemented for the buffer manager.
It significiently increase the performance of buffer manager compare to linear/binary search with any given set of commands. Check performace for more info.

# Code Guidance

please check [bpt.pdf][pdflink] for more information about the codes and implementation.
[pdflink]: https://hconnect.hanyang.ac.kr/2017_ITE2038_11735/2017_ITE2038_2014004893/blob/master/bpt.pdf

# Performance

### Project2 - No Buffer
	-------------- sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 218.19 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 201.09 secs
	--------------      Delete Test       --------------
	Delete All Records sequential
	Result: 1048576/1048576 (100.00) 175.17 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 201.31 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 260.35 secs
	
### Project3 - 16 Buffer Pool with Linear Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 43.47 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 188.41 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 19.48 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 32.29 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 191.25 secs

### Project3 - 160 Buffer Pool with Linear Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 35.27 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 197.66 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 25.02 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 19.02 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 209.83 secs

### Project3 - 1600 Buffer Pool with Linear Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 106.96 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 300.40 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 18.88 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 82.26 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 329.03 secs

### Project3 - 16 Buffer Pool with Binary Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 41.16 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 164.46 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 25.49 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 29.77 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 206.10 secs

### Project3 - 160 Buffer Pool with Binary Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 33.19 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 170.24 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 26.08 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 16.65 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 181.39 secs

### Project3 - 1600 Buffer Pool with Binary Search
	-------------- Sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 35.84 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 184.18 secs
	--------------      Delete Test       --------------
	Delete All Records Sequential
	Result: 1048576/1048576 (100.00) 17.50 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 18.70 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 174.62 secs

### Project3 - 16 Buffer Pool with Hash Table
	-------------- sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 42.65 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 168.77 secs
	--------------      Delete Test       --------------
	Delete All Records sequential
	Result: 1048576/1048576 (100.00) 20.47 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 22.30 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 186.89 secs

### Project3 - 160 Buffer Pool with Hash Table
	-------------- sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 37.51 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 181.51 secs
	--------------      Delete Test       --------------
	Delete All Records sequential
	Result: 1048576/1048576 (100.00) 20.17 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 14.76 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 177.05 secs

### Project3 - 1600 Buffer Pool with Hash Table
	-------------- sequential Insert Test --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 31.70 secs
	--------------   Random Insert Test   --------------
	Large(2^20) Test
	Result: 1048576/1048576 (100.00) 155.08 secs
	--------------      Delete Test       --------------
	Delete All Records sequential
	Result: 1048576/1048576 (100.00) 14.46 secs
	Delete All Records Reversal
	Result: 1048576/1048576 (100.00) 13.31 secs
	Random_Delete_ALL(2^20) Test
	Result: 1048576/1048576 (100.00) 137.73 secs