#!/usr/bin/env bash

rm *.tab

rm dbfile.bin

rm ./db

rm db.log 

c++ -std=c++11 -m32 db.cpp -o db

/usr/bin/clear

echo "done"
