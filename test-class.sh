#!/bin/bash

./db "create table class (sid int, Student char(12) NOT NULL, Gender char(1), Exams int, Quiz int, Total int)" ;
./db "insert into class values (1, 'Siu', 'M', 480, 80, 560)" ;
./db "insert into class values (2, 'Frank', 'M', 600, 100, 700)" ;
./db "insert into class values (3, 'Jordon', 'M', 450, 75, 525)" ;
./db "insert into class values (4, 'Jeff', 'M', 455, 60, 515)" ;
./db "insert into class values (5, 'Ying', 'F', 540, 85, 625)" ;
./db "insert into class values (6, 'David', 'M', 550, 83, 633)" ;
./db "insert into class values (7, 'euna', 'F', 460, 75, 535)" ;
./db "insert into class values (8, 'Victor', 'M', 475, 60, 535)" ;
./db "insert into class values (9, 'Linawati', 'F', 490, 86, 576)" ;
./db "insert into class values (10, 'Stephen', 'M', 520, 76, 596)" ;

./db "create table test (col1 int, col2 char(10), col3 int)";
./db "insert into test values (1, 'Siu', 11)";
./db "insert into test values (2, 'Frank', 12)";
./db "insert into test values (3, 'c', 13)";
./db "insert into test values (4, 'd', 14)";
./db "insert into test values (5, 'e', 15)";

./db "create table foo (gen char(5), col5 int)";
./db "insert into foo values ('M', 23)";
./db "insert into foo values ('F', 24)";
./db "insert into foo values ('M', 25)";

/usr/bin/clear;

echo "class:" ;
./db "select * from class" ;

echo "test:" ;
./db "select * from test" ;

echo "foo:" ;
./db "select * from foo" ;
