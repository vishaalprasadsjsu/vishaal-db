CS 157B Database Project 
========================
A basic relational database capable of processing basic SQL statements. 

```bash 
$ ./db "select * from class" 
dbfile.bin size = 228
SELECT statement
+----------------+----------------+----------------+----------------+----------------+
|Student         |Gender          |Exams           |Quiz            |Total           |
+----------------+----------------+----------------+----------------+----------------+
|Siu             |M               |             480|              80|             560|
|Frank           |M               |             600|             100|             700|
|Jordon          |M               |             450|              75|             525|
|Jeff            |M               |             455|              60|             515|
|Ying            |F               |             540|              85|             625|
|David           |M               |             550|              83|             633|
|euna            |F               |             460|              75|             535|
|Victor          |M               |             475|              60|             535|
|Linawati        |F               |             490|              86|             576|
|Stephen         |M               |             520|              76|             596|
+----------------+----------------+----------------+----------------+----------------+
10 rows selected.
```

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
  * `SELECT <col>, <aggregate>(<col2>) ... GROUP BY <col>`
  * `... ORDER BY`
  * `... WHERE`
