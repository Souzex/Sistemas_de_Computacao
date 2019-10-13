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
#include <time.h>
#include <math.h>

typedef struct list_char {
	char info;
	struct list_char *prox;
} TListChar;

typedef struct list_long_int {
	unsigned long int info;
	struct list_long_int *prox;
} TListLongInt;

typedef struct node {
    unsigned long int info;
    struct node *prox;
} TNO;

typedef struct fila {
    TNO *ini, *fim;
} TFila;

typedef struct fila TFila;

void executa_programa (unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, unsigned short int map, FILE *arq_ent, FILE *arq_sai);
void executa_map_dir        (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_saida);
void executa_map_assoc      (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_saida);
void executa_map_assoc_conj (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_saida);
void randomico(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_saida);
void fifo(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_saida);
void lfu(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_saida);
void lru(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_saida);
TListLongInt * leitura_arquivo (FILE *arq_entrada);
void grava_saida(const char *mapeamento, const char *politica, unsigned long int endereco, char *miss_hit, unsigned long int num_lines, unsigned long int tam_block, unsigned long int num_block, unsigned long int tag_block, char **cache, FILE *arq_saida);
void grava_estatisticas(const char *mapeamento, const char *politica, unsigned long int acessos, unsigned long int hits, unsigned long int miss, FILE *arq_sai);

TListChar    * inserir_inicio_list_char    (TListChar    *lista, char info);
TListLongInt * inserir_final_list_long_int (TListLongInt *lista, unsigned long int info);
TListLongInt * remover_list_long_int       (TListLongInt *lista, unsigned long int info);
void liberar_list_char     (TListChar   *lista);
void liberar_list_long_int (TListLongInt *lista);

TFila* inicializa_fila (void);
void insere (TFila *f, unsigned long int elem);
unsigned long int retira (TFila *f);
void libera_fila (TFila *f);
int vazia_fila (TFila *f);

int main (int argc, char* argv[]) {

	if (argc == 6) {
		
		unsigned long int tam_ram   = atoi(argv[1]); // [0, 4.294.967.295]
		unsigned long int tam_block = atoi(argv[2]); // [0, 4.294.967.295]
		unsigned long int num_lines = atoi(argv[3]); // [0, 4.294.967.295] 
		unsigned short int map      = atoi(argv[4]); // [0, 65,535] 
		char *nome_arq_ent          = argv[5];
		char nome_arq_sai[200];
		strcpy(nome_arq_sai,"");
		strcat(nome_arq_sai,"SAIDA_");
		strcat(nome_arq_sai,argv[5]);
		strcat(nome_arq_sai,"-");
		strcat(nome_arq_sai,argv[1]);
		strcat(nome_arq_sai,"-");
		strcat(nome_arq_sai,argv[2]);
		strcat(nome_arq_sai,"-");
		strcat(nome_arq_sai,argv[3]);
		strcat(nome_arq_sai,"-");
		strcat(nome_arq_sai,argv[4]);
		strcat(nome_arq_sai,"_");
		
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		char timestamp[16];
		sprintf(timestamp, "%d%d%d_%d%d%d", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		
		strcat(nome_arq_sai,timestamp);
		strcat(nome_arq_sai,".guto");		
		
		if (tam_ram <= 0) {
			
			printf("Valor do tamanho da RAM inválido: %s\n", argv[1]);
		
		} else if (tam_block <= 0) {
			
			printf("Valor do tamanho de cada bloco da RAM (e da cache) inválido: %s\n", argv[2]);
		
		} else if (num_lines <= 0) {

			printf("Valor do número total de quadros da cache inválido: %s\n", argv[3]);

		} else if ((map != 1) && (map != 2) && (map != 3)){
			
			printf("Valor de map (esquema de mapeamento) inválido: %s\n", argv[4]);
		
		} else {
		
			FILE *arq_entrada = fopen(nome_arq_ent, "r");
			FILE *arq_saida   = fopen(nome_arq_sai, "w");
			if (arq_entrada && arq_saida) 				
				executa_programa (tam_ram, tam_block, num_lines, map, arq_entrada, arq_saida);
			else
				printf("Valor de file (nome do arquivo) inválido: %s\n", argv[5]);
			fclose(arq_entrada);
			fclose(arq_saida);
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

void executa_programa (unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, unsigned short int map, FILE *arq_ent, FILE *arq_sai) {
	
	TListLongInt *end_mem_list, *end_mem_list_ptr_ini;
	end_mem_list = end_mem_list_ptr_ini = leitura_arquivo(arq_ent);
	
	if(map == 1)
		executa_map_dir(end_mem_list, tam_ram, tam_block, num_lines, arq_sai);
	else if(map == 2)
		executa_map_assoc(end_mem_list, tam_ram, tam_block, num_lines, arq_sai);
	else if(map == 3)
		executa_map_assoc_conj(end_mem_list, tam_ram, tam_block, num_lines, arq_sai);
		
	liberar_list_long_int(end_mem_list_ptr_ini);

}

TListLongInt * leitura_arquivo (FILE *arq_entrada) {
	
	TListLongInt * end_mem_list = NULL;
	char *linha = (char *) malloc(12); // "4294967295\n\0" de [0, 4.294.967.295]
		
	while (fgets(linha,12,arq_entrada)) {
	
	// Em ASCII: TAB 9, LF ('\n') 10, CR ('\r') 13, Space (' ') 32.
	// Tentativa de pegar ou ou outro erro de formatacao do arquivo de entrada.
		if ((linha[0] == 9) || (linha[0] == 10) || (linha[0] == 13) || (linha[0] == 32))
			break;

	// Com fputs(), o EOF eh na mesma linha do ultimo registro: colocar '\n' para ficar igual aos demais registros.
		if (feof(arq_entrada)) {
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
				end_mem_list = inserir_final_list_long_int(end_mem_list, end_mem);
				liberar_list_char (digito_list);
				break;
			}
		}
	}
	if(linha) free(linha);
	return end_mem_list;
}

void executa_map_dir (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_sai) {

	unsigned long int tam_cache = tam_block * num_lines;
	unsigned long int num_block = tam_ram / tam_block;
	unsigned long int tag_block = tam_ram / tam_cache;
	unsigned long int hits = 0;
	unsigned long int miss = 0;
	unsigned long int acessos = 0;
	
	unsigned long int i, j;
	
	char **cache = (char **) malloc (tag_block*sizeof(char *));
	for(i=0; i<tag_block; i++)
		cache[i] = (char *) malloc (num_lines*sizeof(char));
	
	for(i=0; i<tag_block; i++)
		for(j=0; j<num_lines; j++)
			cache[i][j] = 0;

	while (end_mem_list) {
		
		char miss_hit[5];
		unsigned long int quadro = ((end_mem_list->info)/tam_block) % num_lines;
		unsigned long int tag    = ((end_mem_list->info)/tam_block) / num_lines;
		
		acessos++;
		
		if (cache[tag][quadro] == '1') {// CACHE HIT
			strcpy(miss_hit,"HIT");
			hits++;
		} else {// CACHE MISS
			strcpy(miss_hit,"MISS");
			miss++;
			for(i=0; i<tag_block; i++) cache[i][quadro] = '0';
			cache[tag][quadro] = '1'; 
		}
		
		grava_saida("DIRETO", "NONE", end_mem_list->info, miss_hit, num_lines, tam_block, num_block, tag_block, cache, arq_sai);		
		end_mem_list = end_mem_list->prox;
	} 
	grava_estatisticas("DIRETO", "NONE", acessos, hits, miss, arq_sai);
}

void executa_map_assoc (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_sai) {

	unsigned long int tam_cache = tam_block * num_lines;
	unsigned long int num_block = tam_ram / tam_block;
	
	unsigned long int i, j;
	
	char **cache_rand = (char **) malloc (num_block*sizeof(char *));
	for(i=0; i<num_block; i++)
		cache_rand[i] = (char *) malloc (num_lines*sizeof(char));
	
	char **cache_fifo = (char **) malloc (num_block*sizeof(char *));
	for(i=0; i<num_block; i++)
		cache_fifo[i] = (char *) malloc (num_lines*sizeof(char));
	
	char **cache_lru = (char **) malloc (num_block*sizeof(char *));
	for(i=0; i<num_block; i++)
		cache_lru[i] = (char *) malloc (num_lines*sizeof(char));
	
	char **cache_lfu = (char **) malloc (num_block*sizeof(char *));
	for(i=0; i<num_block; i++)
		cache_lfu[i] = (char *) malloc (num_lines*sizeof(char));
		
	for(i=0; i<num_block; i++)
		for(j=0; j<num_lines; j++) {
			cache_rand[i][j] = 0;
			cache_fifo[i][j] = 0;
			cache_lru[i][j]  = 0;
			cache_lfu[i][j]  = 0;
		}
	
	randomico(end_mem_list, tam_block, num_lines, num_block, cache_rand, arq_sai);
	fifo(end_mem_list, tam_block, num_lines, num_block, cache_fifo, arq_sai);
	lru(end_mem_list, tam_block, num_lines, num_block, cache_lfu, arq_sai);
	lfu(end_mem_list, tam_block, num_lines, num_block, cache_lru, arq_sai);
	
	for(i=0; i<num_block; i++) {
		free(cache_rand[i]);
		free(cache_fifo[i]);
		free(cache_lru[i]);
		free(cache_lfu[i]);
	}
	free(cache_rand);
	free(cache_fifo);
	free(cache_lru);
	free(cache_lfu);
}

void executa_map_assoc_conj (TListLongInt *end_mem_list, unsigned long int tam_ram, unsigned long int tam_block, unsigned long int num_lines, FILE *arq_sai) {

}

void randomico(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_sai) {

	unsigned long int i;

	unsigned long int hits = 0;
	unsigned long int miss = 0;
	unsigned long int acessos = 0;

	while (end_mem_list) {

		char miss_hit[5];
		unsigned long int quadro = 0;
		unsigned long int hits_ant = hits;
		unsigned long int tag = (end_mem_list->info)/tam_block;
	
		acessos++; 
	
		for(i=0; i<num_lines; i++) {
			
			if (cache[tag][i] == '1') {// CACHE HIT
				strcpy(miss_hit,"HIT");
				hits++;
				break;
			}
		}
	
		if (hits_ant == hits) { // CACHE MISS
			strcpy(miss_hit,"MISS");
			miss++;
			if (miss > num_lines) {
				quadro = rand()%num_lines;
				for(i=0; i<num_block; i++) cache[i][quadro] = '0';
				cache[tag][quadro] = '1';
			} else {
				cache[tag][miss-1] = '1';
			}
		}	
		
		grava_saida("ASSOCIATIVO", "RANDOM", end_mem_list->info, miss_hit, num_lines, tam_block, num_block, 0, cache, arq_sai);
		end_mem_list = end_mem_list->prox;
	}
	grava_estatisticas("ASSOCIATIVO", "RANDOM", acessos, hits, miss, arq_sai);
}

void fifo(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_sai) {
	
	TFila *fila = inicializa_fila();
	
	unsigned long int i;
	
	unsigned long int hits = 0;
	unsigned long int miss = 0;
	unsigned long int acessos = 0;

	while (end_mem_list) {
		
		char miss_hit[5];
		unsigned long int quadro = 0;
		unsigned long int hits_ant = hits;
		unsigned long int tag = (end_mem_list->info)/tam_block;
		
		acessos++; 
	
		for(i=0; i<num_lines; i++) {
			
			if (cache[tag][i] == '1') {// CACHE HIT
				strcpy(miss_hit,"HIT");
				hits++;
				break;
			}
		}
		
		if (hits_ant == hits) { // CACHE MISS
			strcpy(miss_hit,"MISS");
			miss++;
			if (miss > num_lines) {
				quadro = retira(fila);
				for(i=0; i<num_block; i++) cache[i][quadro] = '0';
				cache[tag][quadro] = '1';
				insere(fila, quadro);
			} else {
				cache[tag][miss-1] = '1';
				insere(fila, miss-1);
			}
		}
		
		grava_saida("ASSOCIATIVO", "FIFO", end_mem_list->info, miss_hit, num_lines, tam_block, num_block, 0, cache, arq_sai);
		end_mem_list = end_mem_list->prox;
	}
	libera_fila(fila);
	grava_estatisticas("ASSOCIATIVO", "FIFO", acessos, hits, miss, arq_sai);
}

void lru(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_sai) {
	
	TListLongInt *lru_list = NULL;
	
	unsigned long int i;
	
	unsigned long int hits = 0;
	unsigned long int miss = 0;
	unsigned long int acessos = 0;

	while (end_mem_list) {
		
		char miss_hit[5];
		unsigned long int quadro = 0;
		unsigned long int hits_ant = hits;
		unsigned long int tag = (end_mem_list->info)/tam_block;
		
		acessos++; 
	
		for(i=0; i<num_lines; i++) {
			
			if (cache[tag][i] == '1') {// CACHE HIT
				strcpy(miss_hit,"HIT");
				hits++;
				lru_list = remover_list_long_int(lru_list,i);
				lru_list = inserir_final_list_long_int(lru_list,i);
				break;
			}
		}
		
		if (hits_ant == hits) { // CACHE MISS
			strcpy(miss_hit,"MISS");
			miss++;
			if (miss > num_lines) {				
				quadro = lru_list->info;
				lru_list = remover_list_long_int(lru_list,lru_list->info);
				lru_list = inserir_final_list_long_int(lru_list,lru_list->info);
				for(i=0; i<num_block; i++) cache[i][quadro] = '0';
				cache[tag][quadro] = '1';
			} else {
				lru_list = inserir_final_list_long_int(lru_list,miss-1);
				cache[tag][miss-1] = '1';
			}
		}
		
		grava_saida("ASSOCIATIVO", "LRU", end_mem_list->info, miss_hit, num_lines, tam_block, num_block, 0, cache, arq_sai);
		end_mem_list = end_mem_list->prox;
	}
	liberar_list_long_int(lru_list);
	grava_estatisticas("ASSOCIATIVO", "LRU", acessos, hits, miss, arq_sai);	
}

void lfu(TListLongInt *end_mem_list, unsigned long int tam_block, unsigned long int num_lines, unsigned long int num_block, char **cache, FILE *arq_sai) {
	
	unsigned long int i;
	
	unsigned long int contador[num_lines];
	for(i=0; i<num_lines; i++) contador[i] = 0;
		
	unsigned long int hits = 0;
	unsigned long int miss = 0;
	unsigned long int acessos = 0;
	
	while (end_mem_list) {
		
		char miss_hit[5];
		unsigned long int quadro = 0;
		unsigned long int hits_ant = hits;
		unsigned long int tag = (end_mem_list->info)/tam_block;
		
		acessos++; 
	
		for(i=0; i<num_lines; i++) {
			
			if (cache[tag][i] == '1') {// CACHE HIT
				strcpy(miss_hit,"HIT");
				hits++;
				contador[i]++;
				break;
			}
		}
	
		if (hits_ant == hits) { // CACHE MISS
			strcpy(miss_hit,"MISS");
			miss++;
			if (miss > num_lines) {				
				unsigned long int max=0;
				for(i=0; i<num_lines; i++)
					if(contador[i] > max) {
						max = contador[i];
						quadro = i;
					}
				contador[quadro] = 0;
				for(i=0; i<num_block; i++) cache[i][quadro] = '0';
				cache[tag][quadro] = '1';
			} else {
				contador[miss-1]++;;
				cache[tag][miss-1] = '1';
			}
		}	
		
		grava_saida("ASSOCIATIVO", "LFU", end_mem_list->info, miss_hit, num_lines, tam_block, num_block, 0, cache, arq_sai);
		end_mem_list = end_mem_list->prox;
	}
	grava_estatisticas("ASSOCIATIVO", "LFU", acessos, hits, miss, arq_sai);
}

void grava_saida(const char *mapeamento, const char *politica, unsigned long int endereco, char *miss_hit, unsigned long int num_lines, unsigned long int tam_block, unsigned long int num_block, unsigned long int tag_block, char **cache, FILE *arq_saida) {
	
	unsigned long int i, j;
	char linha[200];
	strcpy(linha,"");
	
	//int tamanho = (int)((ceil(log10(endereco+1))+1)*sizeof(char));
	char str_end[11]; // [0, 4.294.967.295]
	sprintf(str_end, "%lu", endereco);
	
	strcat(linha, mapeamento);
	strcat(linha, "_");
	strcat(linha, politica);
	strcat(linha, "_");
	strcat(linha, miss_hit);
	strcat(linha, "_ENDER_");
	strcat(linha, str_end);
	strcat(linha, "\n");
		
	fputs(linha, arq_saida);
	strcpy(linha, "");
	
	for(j=0; j<num_lines; j++) {
		
		strcat(linha, "{QUADRO_");
		//tamanho = (int)((ceil(log10(j+1))+1)*sizeof(char));
		char str_qdr[11]; // [0, 4.294.967.295]
		sprintf(str_qdr, "%lu", j);
		strcat(linha, str_qdr);
		strcat(linha, "}");
	
		if(!strcmp(mapeamento,"DIRETO")) { // DIRETO_NONE_
		
			for(i=0; i<tag_block; i++) {
				if (cache[i][j] == '1') {
					unsigned long int end = i*tag_block + j*tam_block;
					//tamanho = (int)((ceil(log10(end+tam_block+1))+1)*sizeof(char));
					char str_end1[11], str_end2[11]; // [0, 4.294.967.295]
					sprintf(str_end1, "%lu", end);
					sprintf(str_end2, "%lu", end+tam_block-1);
					strcat(linha, "[");
					strcat(linha, str_end1);
					strcat(linha, "-");
					strcat(linha, str_end2);
					strcat(linha, "]");
				}
			}
	
		} else if (!strcmp(mapeamento,"ASSOCIATIVO")) {
		
			for(i=0; i<num_block; i++) {
				if (cache[i][j] == '1') {
					unsigned long int end = i*tam_block;
					//tamanho = (int)((ceil(log10(end+tam_block+1))+1)*sizeof(char));
					char str_end1[11], str_end2[11]; // [0, 4.294.967.295]
					sprintf(str_end1, "%lu", end);
					sprintf(str_end2, "%lu", end+tam_block-1);
					strcat(linha, "[");
					strcat(linha, str_end1);
					strcat(linha, "-");
					strcat(linha, str_end2);
					strcat(linha, "]");
				}
			}
		
		} else if (!strcmp(mapeamento,"ASSOCI_CONJ")) {

		}
		strcat(linha, "\n");
		fputs(linha, arq_saida);
		strcpy(linha, "");
	}
}

void grava_estatisticas(const char *mapeamento, const char *politica, unsigned long int acessos, unsigned long int hits, unsigned long int miss, FILE *arq_sai) {
	
	float taxa, acc, hit, mis;
	char str_num[350]; // ‘sprintf’ output between 33 and 342 bytes into a destination of size 30 - float representation
	char linha[1000];
	
	acc = acessos;
	hit = hits;
	mis = miss;
	
	strcpy(linha,"");
	strcpy(str_num,"");
	strcat(linha, "\n");
	strcat(linha, "ESTATÍSTICAS:\n");
	strcat(linha, "\n");
	strcat(linha, mapeamento);
	strcat(linha, "_");
	strcat(linha, politica);
	strcat(linha, "\n");
	sprintf(str_num, "ACESSOS: %lu\n", acessos);
	strcat(linha,str_num);
	sprintf(str_num, "HITS: %lu\n", hits);
	strcat(linha,str_num);
	sprintf(str_num, "MISS: %lu\n", miss);
	strcat(linha,str_num);
	taxa = hit/acc;
	sprintf(str_num, "RELAÇÃO HITS/ACESSOS: %.5f\n", taxa);
	strcat(linha,str_num);
	taxa = mis/acc;
	sprintf(str_num, "RELAÇÃO MISS/ACESSOS: %.5f\n", taxa);
	strcat(linha,str_num);
	taxa = hit/mis;
	sprintf(str_num, "RELAÇÃO HITS/MISS: %.5f\n", taxa);
	strcat(linha,str_num);
	strcat(linha, "\n");
	
	fputs(linha, arq_sai);
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

TListLongInt * inserir_final_list_long_int (TListLongInt *lista, unsigned long int info) {

	TListLongInt *novo = (TListLongInt *) malloc(sizeof(TListLongInt));
	novo->info = info;
	novo->prox = NULL;

	if (!lista)
		return novo;

	TListLongInt *aux = lista;
	while (aux->prox)
		aux = aux->prox;
	aux->prox = novo;

	return lista;
}

TListLongInt *remover_list_long_int (TListLongInt *lista, unsigned long int info) {

	if(!lista) return NULL;
	
	TListLongInt *head = lista;
	TListLongInt *ant  = NULL;
	
	while(lista) {
		if(info == lista->info) {
			if(ant) {
				ant->prox = lista->prox;
				free(lista);
			} else {
				head = lista->prox;
				free(lista);
			}
			break;
		}
		ant = lista;
		lista = lista->prox;
	}
	
	return head;
}

void liberar_list_char (TListChar *lista) {

	if (lista) {
		liberar_list_char (lista->prox);
		free (lista);
	}
}

void liberar_list_long_int (TListLongInt *lista) {

	if (lista) {
		liberar_list_long_int (lista->prox);
		free (lista);
	}
}

TFila* inicializa_fila (void) { 

    TFila *f = (TFila *) malloc(sizeof(TFila));
    f->ini = f->fim = NULL;
}

void insere (TFila *f, unsigned long int elem) {

    TNO *novo = (TNO *) malloc (sizeof(TNO));
    novo->info = elem;
    novo->prox = NULL;
    if (vazia_fila(f))
        f->ini = f->fim = novo;
    else {
        f->fim->prox = novo;
        f->fim = novo;
    }
}

unsigned long int retira (TFila *f) {

    if (vazia_fila(f)) exit(1);
    int resp = f->ini->info;
    TNO *q = f->ini;
    f->ini = f->ini->prox;
    if(!f->ini) f->fim = NULL;
    free (q);
    return resp;
}

void libera_fila (TFila *f) { 

    TNO *q = f->ini, *t;
    while (q) {
        t = q;
        q = q->prox;
        free (t);
    }
    free (f);    
}

int vazia_fila (TFila *f) { 

    return f->ini == NULL;
}