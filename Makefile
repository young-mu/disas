CC = g++
#CFLAGS = -DDEBUG

all: disas

disas: Disassembler.o disas.o
	@echo [LINK] $@
	@$(CC) $^ -o $@

disas.o : disas.cc
	@echo [COMP] $@
	@$(CC) -c $< -o $@ $(CFLAGS)

Disassembler.o : Disassembler.cc Disassembler.h
	@echo [COMP] $@
	@$(CC) -c $< -o $@ $(CFLAGS)
	
.PHONY: clean
clean:
	@echo [CLEAN]
	@rm disas disas.o Disassembler.o

