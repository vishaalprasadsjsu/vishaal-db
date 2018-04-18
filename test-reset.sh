#!/usr/bin/env bash

rm *.tab

rm dbfile.bin

rm ./db

c++ -m32 db.cpp -o db

/usr/bin/clear

echo "done"