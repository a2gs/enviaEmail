#ifndef H_ENVIA_EMAIL
#define H_ENVIA_EMAIL

	/* EXPORTing VARs GLOBAIS - inicio */
	/* EXPORTing VARs GLOBAIS - fim */

	#define VERNUM "0.1a"
	#define VERSAO "envia_email() v" ## VERNUM ## " - A2GS 23Nov2002  - SP / Brasil"

	/* DEFINE's & INCLUDE's - inicio */
		#define ERRO -1
		#define OK    1
		#define CR    '\r'
		#define LF    '\n'

		#define AQUI "[%s#%ld]", __FILE__, __LINE__
		#define say_error(errmsg) printf(AQUI); \
		                          printf(errmsg);

		#define SMTP_HOST "127.0.0.1"   /* smtp server       */
		#define SMTP_PORTA 25           /* default smtp port */

		/* RFC821 - inicio */
			#define sCMDSMTP_HELO "HELO %s"        /* HELO <itself-domain>  */
			#define sCMDSMTP_QUIT "QUIT"           /* QUIT                  */
			#define sCMDSTMP_MAIL "MAIL FROM:<%s>" /* MAIL FROM:<I>         */
			#define sCMDSMTP_RCPT "RCPT TO:<%s>"   /* RCPT TO:<destination> */

			#define rREAD_FOR_MAIL              "220"
			#define rCMDSMTP_QUIT_OK            "221"
			#define rCMDSMTP_OK                 "250"
			#define rCMDSMTP_USR_NOT_LOCAL_FRW  "251"
			#define rCMDSMTP_DATA_OK            "354"
			#define rCMDSMTP_USR_DONT_MATCH     "550"
			#define rCMDSMTP_USR_NOT_LOCAL_TRY  "551"
			#define rCMDSMTP_USR_AMBIGUOUS      "553"
		/* RFC821 - fim */

		/* DIGEST - Inicio */
			#define MSGDIGEST_Off	0
			#define MSGDIGEST_On    1
			#define MSGDIGEST_Sts   2
			#define TAM_MSGDIGEST	EVP_MAX_MD_SIZE
			#define DG_MD2		1
			#define DG_MD5		2
			#define DG_SHA		3
			#define DG_SHA1		4
			#define DG_RIPEMD160	5
			#define DG_NULL		0
		/* DIGEST - Fim */

		/* TAMANHOS - inicio */
			#define TAM_NOME	50
			#define TAM_EMAIL	50
			#define TAM_MSG		2000   /* tamanho da msg definida pelo usuario */
			#define TAM_SERVER_SEND	256
			#define TAM_SERVER_RESP	256
			#define TAM_IP		15     /* "aaa.bbb.ccc.ddd" */
		/* TAMANHOS - fim */

/* DEFINEs & INCLUDEs - fim */


/* ESTRUTURAS - inicio */
	typedef struct destinatario{
		char   nome[TAM_NOME+1];
		char   email[TAM_EMAIL+1];
		int    achado;    /* indica se esse email esta no smtpserver 0/1 */
		struct destinatario *prox;
	}St_destinatario;  /* 4 */

	typedef struct link_list{
		St_destinatario *inicio, *fim;
		int              total_list;
		int              total_nao_achados;
	}St_LinkList;      /* 2 */

	typedef struct SMTP_Config{
		int  porta;
		char ip[TAM_IP+1];
	}St_SMTP_Config;   /* 1 */

	typedef struct sckt{
		int                sockfd;
		struct sockaddr_in serv_addr;
	}St_Socket;        /* 3 */

	typedef struct thread_info{
		int a;
	}St_Thread_info;   /* 5 */

	typedef struct mensage{
		char          msg[TAM_MSG+1];          /* nao preciso nem dizer, ne ?      */
		unsigned int  tam_msg;                 /* strlen(St_msg.msg);              */
		unsigned char md[TAM_MSGDIGEST+1];     /* message digest                   */
		unsigned int  tam_md;                  /* Definido em EVP_DigestFinal_ex() */
		int           onmd;                    /* on 1/off 0 MD5                   */
	}St_msg;           /* 6 */

	typedef struct email{
		St_LinkList    EmailList;      /* 2 */
		St_SMTP_Config SMTP_Conf;      /* 1 */
		St_Socket      Socket;         /* 3 */
		St_Thread_info Thread_Info;    /* 5 */
		St_msg         Msg;            /* 6 */
	}St_email;
/* ESTRUTURAS - fim */


/* VARIAVEIS GLOBAIS - inicio */
	signed int glob_envmail_ERRO;	/* variavel global de erro da API envia_email.
                                         * Essa variavel eh setada quando eu quero dizer pro
                                         * programador-usuario q houve ERRO e TERA uma tabela
                                         * de defines de erros!!! falei.
                                         */
/* VARIAVEIS GLOBAIS - fim */


/* PROTOTIPOS - inicio */
	int conecta_tcpip(St_email *);
	int define_msg_email(St_email *, int , char *, ...);
	int adiciona_destinatario_email(St_email *, char *, char *);
	int remove_destinatario_email(St_email *, char *);
	int status_msg_digest(St_email *, int );
	int calcula_msg_digest(St_email *, int );
	St_email * inicia_envia_email(void);
	int fim(St_email *, int );
	int envia_email(St_email *);
/* PROTOTIPOS - fim */

#endif
