set echo=on
rem TEST1.BAT - test driver for CS167B project
rem Compile your code
cl db.cpp
rem Clean up
del dbfile.bin *.tab
rem ==========================================================================================
rem #01 Test setup - create table + insert data
db "create table class ( Student char(12) NOT NULL, Gender char(1), Exams int, Quiz int, Total int )"
rem ------------------------------------------------------------------------------------------
rem #02 Insert 15 rows of data
db "insert into class values ('Siu', 'M', 480, 80, 560)"
db "insert into class values ('Frank', 'M', 600, 100, 700)"
db "insert into class values ('Jordon', 'M', 450, 75, 525)"
db "insert into class values ('Jeff', 'M', 455, 60, 515)"
db "insert into class values ('Ying', 'F', 540, 85, 625)"
db "insert into class values ('David', 'M', 550, 83, 633)"
db "insert into class values ('euna', 'F', 460, 75, 535)"
db "insert into class values ('Victor', 'M', 475, 60, 535)"
db "insert into class values ('Linawati', 'F', 490, 86, 576)"
db "insert into class values ('Stephen', 'M', 520, 76, 596)"
rem ------------------------------------------------------------------------------------------
rem #dir verify file sizes
dir
rem ------------------------------------------------------------------------------------------
rem #03 & #04 Simple select * & verify headings & verify NULL display with -
db "select * from class"
rem ------------------------------------------------------------------------------------------

rem ==========================================================================================
rem Error Testing Section
rem ------------------------------------------------------------------------------------------
rem Setup the table
db "create table test (col1 char(10), col2 int, col3 int not null)"
db "insert into test values ( 'one', 1, 11 )"
db "insert into test values ( 'two', 2, 22 )"
db "select * from test"
rem ------------------------------------------------------------------------------------------
rem #5 "syntax errors in various INSERT statement combinations"
rem
db "insert into test ( 'not_null', 1, '11' )"
db "insert into test values ( 'not_null', 1 )"
rem ------------------------------------------------------------------------------------------
rem #6 "data type mismatch"
rem
db "insert into test values ( 'not_null', 1, 'string' )"
rem ------------------------------------------------------------------------------------------
rem #7 NOT NULL on inserts
rem
db "insert into test values ( 'not_null', 1, NULL )"
rem ------------------------------------------------------------------------------------------
rem End of test1.bat

