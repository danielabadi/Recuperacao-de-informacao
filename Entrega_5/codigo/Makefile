CC=g++ -std=c++17 # compilador, troque para gcc se preferir utilizar C
CFLAGS=-Wall -Wextra # compiler flags, troque o que quiser, exceto bibliotecas externas
EXEC=./tp5 # nome do executavel que sera gerado, nao troque

$(EXEC): tp5.cpp
	$(CC) $(CFLAGS) tp5.cpp -o $(EXEC)


clean: # remove todos os arquivos temporarios que forem gerados pela compilacao
	rm -rf tp5
