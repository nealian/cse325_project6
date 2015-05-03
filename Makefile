CC = gcc
CCOPTS = -c -g -Wall
LINKOPTS = -g -lrt 

TEX = pdflatex
README = README.tex

EXEC=
OBJECTS=disk.o sanic_fs.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LINKOPTS) -o $@ $^

%.o:%.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	- $(RM) $(EXEC)
	- $(RM) $(OBJECTS)
	- $(RM) *~
	- $(RM) core.*
	- $(RM) *.aux *.log *.pdf

test: $(EXEC)
	./$(EXEC)

doc: $(README)
	$(TEX) $(README)
