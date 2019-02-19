#ifndef JVM_H
#define JVM_H

extern char input_file_name[];
extern char output_file_name[];

extern FILE* Input_file_id;
extern FILE* Output_file_id;

extern const char* suffix_input_file_name;
extern const char* suffix_output_file_name;
#ifdef LOG_ENABLE
extern const char* Log_file_name;
#endif

#endif