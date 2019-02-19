/*
 ***************************************************************
 *
 *         FILE NAME      : log.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 
 *
 ***************************************************************
 */

#ifndef LOG_H
#define LOG_H

#define LOG_ENABLE 

#ifdef LOG_ENABLE

#define Log_file Log_file_name
extern FILE* Log_file_id;

#define open_log_file open_log_file_func()
#define close_log_file close_log_file_func()
#define log_print(...) \
	fprintf(Log_file_id, "%s:%s(%d):", __FILE__, __FUNCTION__, __LINE__), \
	fprintf(Log_file_id, __VA_ARGS__); 

/*#define log_print(...) \
	printf("%s:%s(%d):", __FILE__, __FUNCTION__, __LINE__), \
	printf(__VA_ARGS__); */

int open_log_file_func();
int close_log_file_func();

#else

#define open_log_file
#define close_log_file
#define log_print(format, ...) 

#endif

#endif