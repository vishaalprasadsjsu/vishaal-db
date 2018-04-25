CS 157B Database Project 
========================
An implementation of a basic relational database capable of processing basic SQL statements. 

Compiling 
---------

Use a c++ 11th standard compiler, and compile in 32-bit mode. 
```bash 
$ g++ -std=c++11 -m32 ./db.cpp -o db
```

Running
-------
Run the compiled file (`db`) with the SQL query in a String as the only argument.
```bash 
$ ./db "CREATE TABLE class (student char(12) NOT NULL, gender char(1), exams int, quiz int, total int)"
``` 

### Supported Features 
* Only supports positive `int` and `char` types
* Supports the following SQL statement types: 
  * `INSERT`
  * `UPDATE` 
  * `DELETE` 
  * `SELECT *` or `SELECT <col>, <col>, ...`
  * `SELECT <aggregate>` (i.e. `SUM`, `AVG` and `COUNT`)
  * `... ORDER BY`
  * `... WHERE`
