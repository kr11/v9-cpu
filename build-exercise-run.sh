#!/bin/sh
rm -f xc xem os_lab*
gcc -o xc -O3 -m32 -Ilinux -Iroot/lib root/bin/c.c
gcc -o xem -O3 -m32 -Ilinux -Iroot/lib root/bin/em.c -lm

#./xc -o os_lab1 -Iroot/lib exercise/os_lab1.c
./xc -o os_lab2 -Iroot/lib exercise/os_lab2.c
#./xem os_lab1
./xem os_lab2
