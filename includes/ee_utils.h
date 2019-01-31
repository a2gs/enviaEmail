#ifndef H_EE_UTILS
#define H_EE_UTILS

	#define BUF_SIZ 2500
	#define TAM_SEGMIL (sizeof(long)*2+10)

	int logmsg(const char *, ...);
	int nocore(void);
	int comp_str(const char *, const char *, int );
	void dec2hexFF(int , unsigned char *);
	void dump_area(char *, size_t , char *);
	char * tempo_seg_mils(void);

#endif
