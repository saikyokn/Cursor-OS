#include "console.h"

int strcmp(const char* a, const char* b){
    while(*a && *b){
        if(*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

int strncmp(const char* a, const char* b, int n){
    for(int i=0;i<n;i++){
        if(a[i] != b[i]) return 0;
        if(a[i] == 0) return 1;
    }
    return 1;
}

// ===== コマンド =====
void cmd_help(){
    console_write("\nCommands:\nhelp clear echo\n");
}

void cmd_clear(){
    console_clear();
}

void cmd_echo(char* text){
    console_write("\n");
    console_write(text);
    console_write("\n");
}

// ===== シェル本体 =====
void shell_run(char* input){

    if(strcmp(input, "help")){
        cmd_help();
        return;
    }

    if(strcmp(input, "clear")){
        cmd_clear();
        return;
    }

    if(strncmp(input, "echo ", 5)){
        cmd_echo(input + 5);
        return;
    }

    console_write("\nUnknown command\n");
}