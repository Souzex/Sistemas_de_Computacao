/*
Lista de features do programa
1. Main c/ tratamento para argumentos ARGV e ARGC - trabalho_sc cache map file
1.1 cache: Capacidade total da memória cache em número total de páginas.
1.2 map: Esquema de mapeamento (1 - direto; 2 - associativo; 3 - associativo por conjunto).
1.3 file: Nome do arquivo de entrada a ser lido pelo programa.
1.3.1 Sequências de referências aos acessos de páginas da memória RAM: um valor de endereço de memória (número inteiro) por linha.
2. Função leitura do arquivo de entrada.
3. Estruturas de dados bem como funções de manipulação PILHA e FILHA.
4. Implementação dos 3 esquemas de mapeamento.
5. Políticas de substituição.
5.1 FIFO
5.2 LRU
5.3 LFU
5.4 RANDOM
6. Arquivo de saída.
6.1 A cada nova referência do arquivo de entrada: Imprimir a lista de todas as páginas RAM armazenadas na memória cache.
6.2 Ao final da execução: Fração de acertos às referências de memória para cada política.

1. 'Tam. RAM' E 'Tam. Bloco' E 'Tam. CACHE'
2. 'Tam. RAM' E 'Tam. Bloco' E 'Num. Quadros'
3. 'Tam. RAM' E 'Num. Blocos' E 'Tam. CACHE'
4. 'Tam. RAM' E 'Num. Blocos' E 'Num. Quadros'
5. 'Tam. Bloco' E 'Num. Blocos' E 'Tam. CACHE'
6. 'Tam. Bloco' E 'Num. Blocos' E 'Num. Quadros'
Podemos trocar 'Tam. RAM' por 'Tam. End. RAM' (o que aumentaria para 10 combinações, em vez das 6 citadas).
Legenda:
Tam. RAM: Tamanho da memória RAM em Bytes.
Tam. CACHE: Tamanho da memória CACHE em Bytes (desconsiderando os bits dedicados a TAG, que é um componente do quadro).
Tam. Bloco: Tamanho do bloco tanto na memória RAM quanto na CACHE, em Bytes (podendo ser calculado se tivermos o tam. de uma palavra em Bytes e a quant. de palavras/bloco).
Num. Blocos: Quantidade de blocos da memória RAM.
Num. Quadros: Quantidade de quadros (ou linhas) da memória CACHE.
Tam. End. RAM: Número de bits para endereçamento de uma palavra na memória RAM.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list_char {
	char info;
	struct list_char *prox;
} TListChar;

typedef struct list_end_mem {
	unsigned long int info;
	struct list_end_mem *prox;
} TListEndMem;

void executa_programa (unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, unsigned short int map, FILE *fp);
TListEndMem * leitura_arquivo (FILE *fp);
TListChar   * inserir_inicio_list_char   (TListChar   *lista, char info);
TListEndMem * inserir_final_list_end_mem (TListEndMem *lista, unsigned long int info);
void liberar_list_char    (TListChar   *lista);
void liberar_list_end_mem (TListEndMem *lista);

int main (int argc, char* argv[]) {

	if (argc == 6) {
		
		unsigned long int tam_ram   = atoi(argv[1]); // [0, 4.294.967.295]
		unsigned long int tam_block = atoi(argv[2]); // [0, 4.294.967.295]
		unsigned long int num_lines = atoi(argv[3]); // [0, 4.294.967.295] 
		unsigned short int map      = atoi(argv[4]); // [0, 65,535] 
		char *file                  = argv[5];
		
		if (tam_ram == 0) {
			
			printf("Valor do tamanho da RAM inválido: %s\n", argv[1]);
		
		} else if (tam_block == 0) {
			
			printf("Valor do tamanho de cada bloco da RAM (e da cache) inválido: %s\n", argv[2]);
		
		} else if (num_lines == 0) {

			printf("Valor do número total de quadros da cache inválido: %s\n", argv[3]);

		} else if ((map != 1) && (map != 2) && (map != 3)){
			
			printf("Valor de map (esquema de mapeamento) inválido: %s\n", argv[4]);
		
		} else {
		
			FILE *fp = fopen(file, "r");
			if (fp) 				
				executa_programa (tam_ram, tam_block, num_lines, map, fp);
			else
				printf("Valor de file (nome do arquivo) inválido: %s\n", argv[5]);
			fclose(fp);
		}
		
	} else {
		
		printf("Quantidade de parâmetros inválida!\n");
		printf("São necessários 5 parâmetros:\n");
		printf("- tam_ram (valor inteiro): Tamanho da memória RAM em Bytes.\n");
		printf("- tam_block (valor inteiro): Tamanho do bloco tanto na memória RAM quanto na CACHE, em Bytes.\n");
		printf("- num_lines (valor inteiro): Quantidade de quadros (ou linhas) da memória CACHE.\n");
		printf("- map (valor inteiro): Esquema de mapeamento (1 - direto; 2 - associativo; 3 - associativo por conjunto).\n");
		printf("- file (valor cadeia de caracteres): Nome do arquivo de entrada a ser lido pelo programa.\n");
			
	}
}

void executa_programa (unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, unsigned short int map, FILE *fp) {

	TListEndMem *end_mem_list, *end_mem_list_ptr_ini;
	end_mem_list = end_mem_list_ptr_ini = leitura_arquivo(fp);
	
	while (end_mem_list) {
		printf("%lu\n", end_mem_list->info);
		end_mem_list = end_mem_list->prox;
	}
	
	liberar_list_end_mem (end_mem_list_ptr_ini);

}

TListEndMem * leitura_arquivo (FILE *fp) {
	
	TListEndMem * end_mem_list = NULL;
	char *linha = (char *) malloc(12); // "4294967295\n\0" de [0, 4.294.967.295]
		
	while (fgets(linha,12,fp)) {
	
	// Em ASCII: TAB 9, LF ('\n') 10, CR ('\r') 13, Space (' ') 32.
	// Tentativa de pegar ou ou outro erro de formatacao do arquivo de entrada.
		if ((linha[0] == 9) || (linha[0] == 10) || (linha[0] == 13) || (linha[0] == 32))
			break;

	// Com fputs(), o EOF eh na mesma linha do ultimo registro: colocar '\n' para ficar igual aos demais registros.
		if (feof(fp)) {
			int i;
			for (i=0; linha[i] != '\0'; i++);
			linha[i] = '\n';
			linha[i+1] = '\0';
		}		
		
		TListChar *digito_list = NULL;
		char *linha_ptr = linha;
		
		while (1) {
			
			if (*linha_ptr != '\n') {
				
				if ((*linha_ptr >= 48) && (*linha_ptr <= 57)) {// Apenas numeros ('0' a '9')
					digito_list = inserir_inicio_list_char(digito_list, *linha_ptr);
					linha_ptr++;
				} else if (*linha_ptr == 13) { // '/r' ou CR ou CARRIAGE RETURN
					linha_ptr++;
				} else {
					printf("Número %s inválido! São válidos: dígitos numéricos e números de 0 a 4.294.967.295!\n", linha);
					break;
				}
			} else {
				
				unsigned long int end_mem = 0, pot10 = 1;
				while (digito_list) {
					end_mem += pot10*(digito_list->info - '0');
					pot10*=10;
					digito_list = digito_list->prox;
				}
				end_mem_list = inserir_final_list_end_mem(end_mem_list, end_mem);
				liberar_list_char (digito_list);
				break;
			}
		}
	}
	if(linha) free(linha);
	return end_mem_list;
}

TListChar * inserir_inicio_list_char (TListChar *lista, char info) {

	TListChar *novo = (TListChar *) malloc(sizeof(TListChar));
	novo->info = info;
	
	if(lista) 
		novo->prox = lista;
	else
		novo->prox = NULL;
		
	return novo;
}

TListEndMem * inserir_final_list_end_mem (TListEndMem *lista, unsigned long int info) {

	TListEndMem *novo = (TListEndMem *) malloc(sizeof(TListEndMem));
	novo->info = info;
	novo->prox = NULL;

	if (!lista)
		return novo;

	TListEndMem *aux = lista;
	while (aux->prox)
		aux = aux->prox;
	aux->prox = novo;

	return lista;
}

void liberar_list_char (TListChar *lista) {

	if (lista) {
		liberar_list_char (lista->prox);
		free (lista);
	}
}

void liberar_list_end_mem (TListEndMem *lista) {

	if (lista) {
		liberar_list_end_mem (lista->prox);
		free (lista);
	}
}