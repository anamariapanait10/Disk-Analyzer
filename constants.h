#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#define HELP "0"
#define ADD "1"
#define SUSPEND "2"
#define RESUME "3"
#define REMOVE "4"
#define INFO "5"
#define LIST "6"
#define PRINT "7"

#define INSTR_LENGTH 100

const char *daemon_pid_file_path = "/tmp/disk-analyzer/daemon.pid";
const char *da_pid_file_path = "/tmp/disk-analyzer/da.pid";
const char *instruction_file_path = "/tmp/disk-analyzer/instruction.txt";
const char *output_file_path_prefix = "/tmp/disk-analyzer/output";
const char *output_file_path = "/tmp/disk-analyzer/output.txt";
const char *log_file_path = "/tmp/disk-analyzer/log.txt";

#endif
