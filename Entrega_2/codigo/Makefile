CC=g++
CFLAGS=-Wall -Wextra -g -std=c++17
CHILKFLAGS=-I ./chilkat-9.5.0-x86_64-linux-gcc/include -L ./chilkat-9.5.0-x86_64-linux-gcc/lib -lchilkat-9.5.0
TRHEADFLAGS=-pthread
EXEC=./tp2

${EXEC}: tp2.cpp
	${CC} tp2.cpp ${CFLAGS} ${CHILKFLAGS} ${TRHEADFLAGS} -o ${EXEC}

