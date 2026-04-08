# MAKEFILE - Sistema de Processamento de Imagens
# Disciplina: Sistemas Operacionais (UNIVALI)

# Compila ambos os executáveis
all: sender worker

# Compilação do Processo Emissor (Sender)
# Reúne a lógica principal e a manipulação de arquivos PGM

sender: sender.c pgm.c
	gcc -Wall sender.c pgm.c -o sender

# Compilação do Processo Trabalhador (Worker)
# Reúne a lógica principal, manipulação PGM, filtros de imagem
# e a estrutura de fila sincronizada.
# IMPORTANTE: -lpthread é necessário para o suporte a threads (POSIX)

worker: worker.c pgm.c filters.c queue.c
	gcc -Wall worker.c pgm.c filters.c queue.c -o worker -lpthread

clean:
	rm -f sender worker
