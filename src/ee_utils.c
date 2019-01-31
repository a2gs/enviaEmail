/* INCLUDEs DO PROJETO */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>

#include "ee_utils.h"

extern int fd_log_arq;

/* --- Funcoes diversas... -------------------------------------------- */

/* logmsg():
 * Escreve buffer formatado em um arquivo de log
 *
 * PARAMETROS:
 * const char *fmt = String de formatacao
 * ... = variaveis que serao formatadas
 *
 * RETORNO:
 * Tamanho da string formatada
 */

int fd_arq_log = 0;   /* arquivo de log */

int logmsg(const char *fmt, ...)
{
	int ret = 0;
	char msg[BUF_SIZ+1] = {0};
	va_list argumentos;

	va_start(argumentos, fmt);
	ret=vsnprintf(msg, BUF_SIZ, fmt, argumentos);
	msg[BUF_SIZ] = '\0';                 /* garantindo pro strlen()         */
	write(fd_arq_log, msg, strlen(msg)); /* (nao coloquei o 3 elemento como
	                                      * BUF_SIZ pq vsprintf() ja esta
	                                      * cortando ateh o 1o '\0').
	                                      */
	va_end(argumentos);
	return(ret);
}

/* -------------------------------------------------------------------- */

/* nocore():
 * Limita para 0 o tamanho do coredump (impossibilitando eng. revers.)
 *
 * PARAMETROS:
 *
 * RETORNO:
 * 0 Ok. -1 caso de falha (errno sera setado)
 */

int nocore(void)
{
	struct rlimit ncr = {0, 0};
	return(setrlimit(RLIMIT_CORE, (const struct rlimit *) &ncr));
}

/* -------------------------------------------------------------------- */

/* comp_str():
 * Compara 2 strings ate o primeiro '\0'
 * (retorna IDENTICAS se comparacao 'char' por 'char' chegar ate um '/0')
 *
 * PARAMETROS:
 * const char *str1 = Primeira string
 * const char *str2 = Segunda string
 * int n = numero maximo de 'char's' comparados
 *
 * RETORNO:
 *  0 -> Identicas (conforme o blablabla do '\0' jah dito)
 *  1 -> Diferentes ('char' em str2 maior que em str1)
 * -1 -> Diferentes ('char' em str1 maior que em str2)
 */

int comp_str(const char *str1, const char *str2, int n)
{

	register int i = 0;
	int k = n+1;   /* constante: pra apagar o ultimo registro (TAM_EMAIL for
	                * passado como parametro!!!! assim protegemos de nois
	                * proprio!.. heheeh)... pra poder comparar ate o \0 TB!
	                */

	for(i = 0; i < k; i++){
		/* Se foi igual ate o primeiro '\0' de qualquer uma das strings,
		 * retorna STRINGS IGUAIS (strncmp())
		 */
		if(str1[i] == '\0' || str2[i] == '\0')
			return(0);

		if(str1[i] != str2[i]){   /* char[i] diferente */
			if(str1[i] < str2[i]) return(-1);
			else                  return( 1);
		}else
			continue;
	}
	/* NAO DEVE ENTRAR AQUI!! */
	return(0x0d);
}

/* -------------------------------------------------------------------- */

/* dump_area():
 * Imprime no arquivo de log uma regiao de memoria
 *
 * PARAMETROS:
 * char *area = Regiao da memoria a ser impressa
 * size_t tam = Quantidade de bytes impressos
 * char *logo = Descricao da area
 *
 * RETORNO:
 */

void dump_area(char *area, const size_t tam, char *logo)
{
/*
REGUAS:
        1         2         3         4         5         6         7
012345678901234567890123456789012345678901234567890123456789012345678901234
12345678901234567890123456789012345678901234567890123456789012345678901234
MODELO HUMANO:
xx xx xx xx|xx xx xx xx|xx xx xx xx|xx xx xx xx - aaaa|aaaa|aaaa|aaaa

MODELO HACKDUMP:
xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx aaaaaaaaaaaaaaaa

ANTIGO:
xx xx xx xx|xx xx xx xx|xx xx xx xx|xx xx xx xxTaaaa|aaaa|aaaa|aaaa
*/
	unsigned char buff[69+1] = {0};
	register unsigned int pos_buff1 = 0, pos_buff2 = 0, pos_area = 0, i = 1;

	memset(buff, ' ', 69);
	logmsg("+-|%-42.42s [Tot: %uBytes - Inicio: %p]|-+\n", logo, tam, area);

	/* eh mais rapido... e se vc nao gostou, vai se fude! EH ASSIM QUE DEVE SER!! */
	/* (use as reguas no modelo atual)                                            */

	for(pos_buff1 = 0, pos_buff2 = 50, pos_area = 0; pos_area < tam; pos_area++){
		dec2hexFF(area[pos_area], &buff[pos_buff1]);
		pos_buff1 += 3;

		if(isprint(area[pos_area])) buff[pos_buff2] = area[pos_area];
		else                        buff[pos_buff2] = '.';
		
		if(i%4 == 0) pos_buff2 += 2;
		else         pos_buff2++;

		if(i == 16 || pos_area == tam-1){
			buff[11] = '|'; buff[23] = '|';
			buff[35] = '|'; buff[54] = '|';
			buff[59] = '|'; buff[64] = '|';
			buff[47] = ' '; buff[49] = ' ';
			buff[48] = '-'; buff[70] = 0;

			logmsg("| %p %s |\n", &area[pos_area-15], buff);
	
			memset(buff, ' ', 69);
			pos_buff1 = 0;
			pos_buff2 = 50;
			i = 1;
		}else
			i++;
	}
	logmsg("+-|%-60.60s [Fim: %p]|-+\n", logo, &area[pos_area]);
}

/* recebe um dec e 'escreve' o valor na str *her.
ATENCAO:
Essa bosta funciona assim: O MAXIMO VALOR dec PASSADO DEVE SER 0<=dec<=255  porque:
O MAXIMO VALOR ESCRIVO EM *hex SERA 'ff'. SEM '\0'!!! (ESSA FUNCAO NAO ESCREVE NA POSICAO hex[2]!!!)
>> ESSA FUNCAO EH ESPECIFICA PARA dump_area(). SE FOR USAR, CUIDADO! (nao conte com o '\0'!)
*/
void dec2hexFF(int dec, unsigned char *hex)
{
	register unsigned int i = 0;

	for(i = 0, hex[0] = (unsigned char)dec / 16, hex[1] = (unsigned char)dec % 16; i < 2; i++){
		if     (hex[i] == 10) hex[i] = 'a';
		else if(hex[i] == 11) hex[i] = 'b';
		else if(hex[i] == 12) hex[i] = 'c';
		else if(hex[i] == 13) hex[i] = 'd';
		else if(hex[i] == 14) hex[i] = 'e';
		else if(hex[i] == 15) hex[i] = 'f';
		else                  hex[i] = hex[i] + '0';
	}
}

/* -------------------------------------------------------------------- */

/* tempo_seg_mils():
 * Formata uma string com segundo e milessegundo do momento
 *
 * PARAMETROS:
 *
 * RETORNO:
 * String com os dados formatados
 */

char * tempo_seg_mils(void)
{
	static char segmil[TAM_SEGMIL+1] = {0};
	struct timeval st_timeval;
	struct timezone st_timezone;

	memset(segmil, 0, TAM_SEGMIL);

	if(!gettimeofday(&st_timeval, &st_timezone))
		logmsg("ERRO - Tempo de segundo/miliseg nao capturados!\n");

	snprintf(segmil, TAM_SEGMIL-1, "[%ld|%ld]", st_timeval.tv_sec, st_timeval.tv_usec);
	segmil[TAM_SEGMIL] = '\0';

	return(segmil);
}

/* -------------------------------------------------------------------- */
