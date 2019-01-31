/*******************************************************
 * Andre Augusto Giannotti Scota (a2gs)
 * andre.scota@gmail.com
 *
 * API envia_email				 
 *
 * Conjunto de funcoes para envio de email em C POSIX
 *
 * CODIGO LIVRE (Free Code)
 *
 *
 *
 * Fontes:
 * RFC821
 * Unix Network Programming - R. Stevens
 * code.box.sk
 *						 
 *
 *******************************************************/

/*
 * Autor: | Data:     | ID:  | Cagada:            | Versao:
 * Andre  | 23Nov2002 | A2GS | Criacao            | 0.1a
 *        |           |      |                    |
 *        |           |      |                    |
 *
 */

/* FALTA:
 * --- IMPLANTAR VARIAVEL: int glob_envmail_ERRO;
 *
 * - aplicar privilation separation pra cada chamada de funcao (baixar
 *      e voltar o user level SOMENTE nas funcoes que o usuario ira chamar! o resto deixa...)
 *
 * - os offsets mostrados por dump_area nao tao corretos
 *
 *
 *
 *
 */
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
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/evp.h>

#include "envia_email.h"
#include "ee_utils.h"

/********************* MAIN() **********************/
int main(int argc, char **argv, char **env)
{
	int ret = 0;
	char x[] = "andre_augusto_giannotti_scota_test_1 abcde fgh ijlmno pqrst uvxz 0123456789";
	register int i = 0;
	St_email *email_test = NULL;
	St_destinatario *dest_aux = NULL;
	int asd = 0;

	/* Inicializa */
	email_test = inicia_envia_email();
	/* --- */

	/* inclui/exclui destinatarios ... */
	adiciona_destinatario_email(email_test, "Andre Augusto Giannotti Scota", "void@infernno.inc");
	adiciona_destinatario_email(email_test, "a2gs", "a2gs@lab.org");
	adiciona_destinatario_email(email_test, "1234567890", "xyz@lab.org");
	adiciona_destinatario_email(email_test, "Poco_Neuvos", "poco_neuvos@drug.org");
	adiciona_destinatario_email(email_test, "test", "test@wohow.com");
	adiciona_destinatario_email(email_test, "abcdefghj", "abcdefghj@abc.org");

	remove_destinatario_email(email_test, "abcdefghj@abc.org");
	remove_destinatario_email(email_test, "argh.com");
	remove_destinatario_email(email_test, "xyz@lab.org");
	remove_destinatario_email(email_test, "test@wohow.com");
	
	logmsg("Lista de destinatarios (TOTAL: %d):\n", email_test->EmailList.total_list);
	for(i = 1, dest_aux = email_test->EmailList.inicio; i <= email_test->EmailList.total_list; i++, dest_aux = dest_aux->prox){
		logmsg("LOGINUTIL - Nome%d: [%s] email%d: [%s]\n", i, dest_aux->nome, i, dest_aux->email);
	}
	/* --- */

	/* define msg */

	asd=define_msg_email(email_test, DG_MD5, x);
	logmsg("Mensagem: [%s]\n", email_test->Msg.msg);
	logmsg("Mensagem tamanho: [%d-%d]\n", email_test->Msg.tam_msg, asd);

	logmsg("Digest: [");
	for(asd=0; asd < email_test->Msg.tam_md; asd++)
		logmsg("%02x", email_test->Msg.md[asd]);
	logmsg("]\n");
	/* --- */

	dump_area(email_test->Msg.msg, email_test->Msg.tam_msg, "dump da mensagem");

	/* envia_email() */
	ret = envia_email(email_test);
	printf("\n\nRetorno de envia_email(): [%d]\n\n", ret);
	/* --- */

	return(0);
}
/********************* MAIN() **********************/


/************************ ENVIA EMAIL ************************/

/* -------------------------------------------------------------------- */

/* conecta_tcpip():
 * Faz conexao TCPIP com servidor SMTP
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 *
 * RETORNO:
 * OK   -> Conexao Ok (estabelecida)
 * ERRO -> Erro (conexao nao estabelecida)
 */

int conecta_tcpip(St_email *email)
{
	bzero((char *) &email->Socket.serv_addr, sizeof(email->Socket.serv_addr));

	email->Socket.serv_addr.sin_family      = AF_INET;
	email->Socket.serv_addr.sin_addr.s_addr = inet_addr(email->SMTP_Conf.ip);
	email->Socket.serv_addr.sin_port        = htons(email->SMTP_Conf.porta);

	if((email->Socket.sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		logmsg("ATENCAO - ERROR conecta_tcpip(): socket() < 0. Retornar corretamente e deixar errno setado!\n");
		fim(email, ERRO);
	}

	if(connect(email->Socket.sockfd,
	   (struct sockaddr *) &email->Socket.serv_addr,
	   sizeof(email->Socket.serv_addr)) < 0){
		logmsg("ATENCAO - ERROR conecta_tcpip(): connect() < 0. Retornar corretamente e deixar errno setado!\n");
		fim(email, ERRO);
	}
	return(OK);
}

/* -------------------------------------------------------------------- */

/* define_msg_email():
 * Define a msg que o usuario quer enviar
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * int tipo_digest = tipo do algoritmo de message digest (ver calcula_msg_digest())
 * char *msg_fmt = buffer contendo a msg que formatarah a mensagem de envio
 * ... = ...
 *
 * RETORNO:
 * Retorna o tamanho da msg que sera enviada
 */

int define_msg_email(St_email *email, int tipo_digest, char *msg_fmt, ...)
{
	int ret = 0;
	va_list argumentos;	

	va_start(argumentos, msg_fmt);

	ret=vsnprintf(email->Msg.msg, TAM_MSG+1, msg_fmt, argumentos);

	if(ret<=TAM_MSG) email->Msg.tam_msg = ret;
	else             email->Msg.tam_msg = TAM_MSG;

	email->Msg.msg[email->Msg.tam_msg] = '\0';

	if(tipo_digest == DG_NULL){
		status_msg_digest(email, MSGDIGEST_Off);
		memset(email->Msg.md, '\0', TAM_MSGDIGEST);
	}else{
		status_msg_digest(email, MSGDIGEST_On);
		if(calcula_msg_digest(email, tipo_digest) == ERRO) logmsg("Erro em calcular digest!!\n");
	}

	return(email->Msg.tam_msg);
}

/* -------------------------------------------------------------------- */

/* adiciona_destinatario_email():
 * Adiciona uma nova pessoa a lista de destinatarios
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * char *nome = buffer contendo o nome do destinatario
 * char *email = buffer contendo o email do destinatario
 *
 * RETORNO:
 * Retorna OK se adicionado. ERRO se nao foi possivel.
 */

int adiciona_destinatario_email(St_email *email, char *dest_nome, char *dest_email)
{
	St_destinatario *dest_aux = NULL;

	/* Registro do novo destinatario */
	if((dest_aux = ((St_destinatario *)malloc(sizeof(St_destinatario)))) == NULL){
		logmsg("LOGINUTIL - ERRO adiciona_destinatario: impossivel alocar memo. Setar errno e documentar!!\n");
		return(ERRO);
	}else{
		/* carregando dados novos no novo registro */
		strncpy(dest_aux->nome, dest_nome, TAM_NOME);
		dest_aux->nome[TAM_NOME]  ='\0';
		strncpy(dest_aux->email, dest_email, TAM_EMAIL);
		dest_aux->email[TAM_EMAIL] = '\0';

		/* se nao ha ninguem na lista... */
		if(email->EmailList.total_list == 0){
			/* eh  primeiro da lista... adicionar corretamente */
			dest_aux->prox          = (St_destinatario *)NULL;
			email->EmailList.inicio = dest_aux;
			email->EmailList.fim    = dest_aux;
		}else{
			/* adiciona no fim */
			dest_aux->prox             = (St_destinatario *)NULL;
			email->EmailList.fim->prox = dest_aux;
			email->EmailList.fim       = dest_aux;
		}
		email->EmailList.total_list++;   /* total de destin. */
		return(OK);
	}
}

/* -------------------------------------------------------------------- */

/* remove_destinatario_email():
 * Remove uma nova pessoa a lista de destinatarios
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * char *email = buffer contendo o email do destinatario
 *
 * RETORNO:
 * Retorna OK se removido. ERRO se nao foi possivel (localizado na lista).
 */

int remove_destinatario_email(St_email *email, char *dest_email)
{
	register int i = 0;
	int comp_ret = 0;
	/* *dest_aux1 = registro a ser exluido
	 * *dest_aux2 = registro anterior da aux1
	 */
	register St_destinatario *dest_aux1 = NULL, *dest_aux2 = NULL;

	for(dest_aux1 = email->EmailList.inicio,
	    dest_aux2 = email->EmailList.inicio,
	    i = 1                               ;
	    i <= email->EmailList.total_list    ;
	    i++                                  ){

		/* Essa funcao compara strings (emails).
		 * Nao usei str#cmp() pq quando chega no '\0', essas funcs retornam
		 * != '\0'. No caso, se for igual ate o '\0' DE QUALQUER UMAS DAS
		 * STRINGS, as strings (emails) SERAO CONSIDERADAS IGUAS!!!
		 */
		comp_ret = comp_str(dest_email, dest_aux1->email, TAM_EMAIL);

		/* o email passado pelo programador-usuario eh igual...eh esse q sera excluido */
		if(comp_ret == 0){

			/* desreferenciando node... */
			if(dest_aux1 == email->EmailList.inicio)   /* se for o primeiro */
				/* dest_aux1 VC ESTAH FORA!! */
				email->EmailList.inicio = dest_aux1->prox;

			else /* nao eh o primeiro */
				/* dest_aux1 VC ESTAH FORA!! */
				dest_aux2->prox = dest_aux1->prox;

			free(dest_aux1);   /* free... */
			email->EmailList.total_list--;   /* total de destin. */

			return(OK);   /* localizou e removeu */

		}else /* 1 ou -1 */{
		/* o email passado pelo programador-usuario eh diferente... nao eh esse q sera
		 * excluido. Iremos avancar aux2 e achar o proximo node para
		 * aux1 apontar para proxima comparacao.
		 */
			dest_aux2 = dest_aux1;
			dest_aux1 = dest_aux1->prox;
		}
	}
	return(ERRO);   /* nao localizou registro */
}

/* -------------------------------------------------------------------- */

/* status_msg_digest():
 * Altera/retorna se o Message Digest esta ativo ou nao.
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * i = Tipo da operacao:
 *     MSGDIGEST_On  -> Liga msg digest (Retorna MSGDIGEST_On)
 *     MSGDIGEST_Off -> Desliga msg digest (Retorna MSGDIGEST_Off)
 *     MSGDIGEST_Sts -> Retorna o status do msg digest
 *
 * RETORNO:
 * MSGDIGEST_Off ou MSGDIGEST_On, conforme operacao.
 */

int status_msg_digest(St_email *email, int i)
{
	register int ret = 0;

	if(i == MSGDIGEST_Off){
		email->Msg.onmd = MSGDIGEST_Off;
		ret = MSGDIGEST_Off;
	}else if(i == MSGDIGEST_On){
		email->Msg.onmd = MSGDIGEST_On;
		ret = MSGDIGEST_On;
	}else if(i == MSGDIGEST_Sts)
		ret = email->Msg.onmd;

	return(ret);
}

/* -------------------------------------------------------------------- */

/* calcula_msg_digest():
 * Calcula Message Digest (utilizando biblioteca OpenSSL) se ligado
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * int tipo_digest = Tipo do message digest:
 *                   DG_MD2       -> MD2
 *                   DG_MD5       -> MD5
 *                   DG_SHA       -> SHA
 *                   DG_SHA1      -> SHA1
 *                   DG_RIPEMD160 -> RIPEMD160
 *                   DG_NULL      -> Nao calcula digest
 *
 * RETORNO:
 * Tamanho do digest calculado (byte) ou ERRO em caso de falha.
 */

int calcula_msg_digest(St_email *email, int tipo_digest)
{
	EVP_MD_CTX *evp_ctx_st;
	const EVP_MD * (*digest_aplicado)(void);

	evp_ctx_st = EVP_MD_CTX_create();
	if(evp_ctx_st == NULL)
		return(ERRO);

	/* Indico que o message digest esta ativo */
	if(email->Msg.onmd == MSGDIGEST_On){

		/* Adiciono todos algoritmos de msg digest na tabela interna OpenSSL */
		OpenSSL_add_all_digests();

		/* Inicializo evp_ctx_st */
		EVP_MD_CTX_init(evp_ctx_st);

		/* Seleciono o tipo do algoritmo msg digest */
		switch(tipo_digest){
			/*case DG_MD2:       -* EVP_md2(3) *-
				digest_aplicado = EVP_md2;
				break;*/
			case DG_MD5:       /* EVP_md5(3)       */
				digest_aplicado = EVP_md5;
				break;
			/* case DG_SHA:       -* EVP_sha(3) *-
				digest_aplicado = EVP_sha;
				break;*/
			case DG_SHA1:      /* EVP_sha1(3)      */
				digest_aplicado = EVP_sha1;
				break;
			case DG_RIPEMD160: /* EVP_ripemd160(3) */
				digest_aplicado = EVP_ripemd160;
				break;
			default:           /* EVP_md_null(3)   */
				digest_aplicado = EVP_md_null;
				break;
		}

		/* Indica para usar o msgdigest evp_md_st no contexto evp_ctx_st */
		if(!(EVP_DigestInit_ex(evp_ctx_st, digest_aplicado(), NULL)))
			return(ERRO);

		/* Calcula digest do buffer Msg.email usando o contexto evp_ctx_st */
		if(!(EVP_DigestUpdate(evp_ctx_st, email->Msg.msg, email->Msg.tam_msg)))
			return(ERRO);

		/* Armazena o calculo do digest no buffer Msg.md5, email->Msg.tam_md recebera o tamanho do digest */
		if(!(EVP_DigestFinal_ex(evp_ctx_st, email->Msg.md, &email->Msg.tam_md)))
			return(ERRO);

		/* Limpa contexto evp_ctx_st */
		EVP_MD_CTX_destroy(evp_ctx_st);

		return(email->Msg.tam_md);
	}else{
		memcpy("MSG DIGEST OFF", email->Msg.md, TAM_MSGDIGEST);
		email->Msg.md[TAM_MSGDIGEST] = '\0';
		return(ERRO);
	}
}

/* -------------------------------------------------------------------- */

/* inicia_envia_email()
 * Declara struct de controle de email.
 * Essa struct contem a lista de destinatario, configuracao da mensage, etc.
 * Varias structs email podem ser declaradas. Cada uma tera comportamento proprio.
 *
 * PARAMETROS:
 *
 * RETORNO:
 * Endereco para struct email ou ERRO em caso de falha.
 */

extern int  fd_arq_log;   /* arquivo de log. TODO: Isso deve ficar em ee_utils.c */

St_email * inicia_envia_email(void)
{
	St_email *ret = (St_email *)NULL;

	/* Abrir/criar arquivo de log */
	fd_arq_log = open("./envia_email.log", O_WRONLY|O_CREAT|O_APPEND, S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	if(fd_arq_log == -1){
		printf("ERRO inicia_envia_email(): Nao pode abrir arquivo de log. errno=[%s]\nARQUIVO DE LOG REDIRECIONADO PARA STDERR_FILENO\n", strerror(errno));
		fd_arq_log = STDERR_FILENO;
	}

	/* Aloca memo. para struct principal: St_email */
	if((ret = (St_email *)malloc((size_t)sizeof(St_email))) == (St_email *)NULL)
		fim(ret, ERRO);

	memset(ret, 0, sizeof(St_email));   /* 00000... 0 */

	/* Iremos setar algumas valores iniciais: tamanho da lista (0),
	 * conf. SMTP, etc
	 */
	ret->SMTP_Conf.porta = SMTP_PORTA;
	strncpy(ret->SMTP_Conf.ip, SMTP_HOST, TAM_IP);
	ret->SMTP_Conf.ip[TAM_IP] = '\0';

	ret->EmailList.inicio            = (St_destinatario *)NULL;
	ret->EmailList.fim               = (St_destinatario *)NULL;
	ret->EmailList.total_list        = 0;
	ret->EmailList.total_nao_achados = 0;
	ret->Msg.onmd                    = MSGDIGEST_On;

	/* Limita CoreDump pra 0 (hackers experientes sabem eng. rev.) */
	if(nocore() != 0)
		logmsg("ERRO inicia_envia_email(): nocore()!=0. continuando...\n");

	return(ret);
}

/* -------------------------------------------------------------------- */

/* fim():
 * Finaliza thread com algum retorno
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 * int erro = Motivo da saida
 *
 * RETORNO:
 */

int fim(St_email *email, int erro)
{
	/* ESSA FUNCAO DEVE FINALIZAR O THREAD ABERTO POR envia_email()
	ESSA FUNCAO SO DEVE SER CHAMADA NAS FUNCOES:
	conecta()
	*/

	logmsg("ATENCAO - Saindo com erro: exit(%d)\n(ATENCAO: QUANDO FOR PTHREADs NAO PODEREI FAZER exit()!!!)", erro);
	logmsg("Valor de errno = [%d - %s]\n", errno, strerror(errno));

	exit(erro);
}

/* -------------------------------------------------------------------- */

/* envia_email()
 * Envia email com os dados contidos em *email (destinatarios, msg, etc).
 *
 * PARAMETROS:
 * St_email *email = ponteiro para strutc de controle de email
 *
 * RETORNO:
 *
 */

int envia_email(St_email *email)
{   /* i lah vamos nois! */

	/*
		- checa a estrutura *email: verifica se todos dados estao ok. se nao: return(ERRO)
		e seta erro glob_envia_email
	
		- DISPARA THREAD
	*/

	conecta_tcpip(email);

	return(OK);
}

/************************ ENVIA EMAIL ************************/
