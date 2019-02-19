/*
 ***************************************************************
 *
 *         FILE NAME      : hack.h
 *         ORIGINATOR     : ÕıÓ»—Ô(micklongen)
 *         DATE OF ORIGIN : 
 *
 ***************************************************************
 */

#ifndef HACK_H
#define HACK_H

#define START_DATA_ADDR 16
#define START_CODE_ADDR 0

extern FILE* Input_file_id;
extern FILE* Output_file_id;
extern int pass;

extern const char* suffix_input_file_name;
extern const char* suffix_output_file_name;
#ifdef LOG_ENABLE
extern const char* Log_file_name;
#endif

#endif