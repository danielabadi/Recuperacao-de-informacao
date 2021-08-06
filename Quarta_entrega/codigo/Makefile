CC=g++ -std=c++17 # compilador, troque para gcc se preferir utilizar C
CFLAGS=-Wall -Wextra # compiler flags, troque o que quiser, exceto bibliotecas externas
PARSERFLAGS=`pkg-config --cflags --libs gumbo`
EXEC=./tp4 # nome do executavel que sera gerado, nao troque

$(EXEC): tp4.cpp
	$(CC) $(CFLAGS) tp4.cpp -o $(EXEC) $(PARSERFLAGS)


clean: # remove todos os arquivos temporarios que forem gerados pela compilacao
	rm -rf tp4
