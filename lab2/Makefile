all: ftp parser
	@echo "Compiling application..."
	gcc main.c ftp.o parser.o -lm 
	
ftp:
	@echo "Compiling ftp..."
	gcc -c ftp.c

parser:
	@echo "Compiling parser..."
	gcc -c parser.c -lm
