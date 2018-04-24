#!/bin/bash

./db "create table class ( Student char(12), Gender char(1), Exams int, Quiz int, Total int )" ;

./db "insert into class values ('Siu', 'M', 480, 80, 560)" ;

./db "insert into class values ('Frank', 'M', 600, 100, 700)" ;

./db "insert into class values ('Jordon', 'M', 450, 75, 525)" ;

./db "insert into class values ('Jeff', 'M', 455, 60, 515)" ;

./db "insert into class values ('Ying', 'F', 540, 85, 625)" ;

./db "insert into class values ('David', 'M', 550, 83, 633)" ;

./db "insert into class values ('euna', 'F', 460, 75, 535)" ;

./db "insert into class values ('Victor', 'M', 475, 60, 535)" ;

./db "insert into class values ('Linawati', 'F', 490, 86, 576)" ;

./db "insert into class values ('Stephen', 'M', 520, 76, 596)" ;

./db "select * from class" ;

