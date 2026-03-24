#include "console.h"

// ===== strcmp =====
int strcmp(const char* a, const char* b){
    while(*a && *b){
        if(*a != *b) return 1;
        a++; b++;
    }
    return *a != *b;
}

// ===== shell =====
void shell_run(char* input){

    // ===== color =====
    if(strcmp(input, "color red") == 0){
        console_set_color(0x00FF0000);
        console_write("\ncolor = red\n");
        return;
    }

    if(strcmp(input, "color green") == 0){
        console_set_color(0x0000FF00);
        console_write("\ncolor = green\n");
        return;
    }

    if(strcmp(input, "color blue") == 0){
        console_set_color(0x000000FF);
        console_write("\ncolor = blue\n");
        return;
    }

    if(strcmp(input, "color white") == 0){
        console_set_color(0x00FFFFFF);
        console_write("\ncolor = white\n");
        return;
    }

    if(strcmp(input, "color yellow") == 0){
        console_set_color(0x00FFFF00);
        console_write("\ncolor = yellow\n");
        return;
    }

    // ===== clear =====
    if(strcmp(input, "clear") == 0){
        console_clear();
        return;
    }

    // ===== help =====
    if(strcmp(input, "help") == 0){
        console_write("\nCommands:\n");
        console_write("color red/green/blue/white/yellow\n");
        console_write("clear\n");
        console_write("help\n");
        return;
    }

    // ===== fallback =====
    console_write("\nUnknown command\n");
}