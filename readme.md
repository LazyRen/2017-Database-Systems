# Disk Based B+ Tree
======================
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
# Each internal page can hold up to 248 entries(child page)
# Each leaf page can hold up to 31 recordes.

# Code Guidance

please check bpt.pdf for more information about the codes and implementation.

# Performance