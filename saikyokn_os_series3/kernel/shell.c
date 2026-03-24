#include "console.h"

int strcmp(const char* a, const char* b){
    while(*a && *b){
        if(*a != *b) return 1;
        a++; b++;
    }
    return *a != *b;
}

void shell_run(char* input){

    // ===== clear =====
    if(strcmp(input, "clear") == 0){
        console_clear();
        console_write("> ");
        return;
    }

    // ===== UI RENDER =====
    if(strcmp(input, "/UIR") == 0){
        console_ui_render();
        return;
    }

    // ===== color =====
    if(strcmp(input, "color red") == 0){
        console_set_color(0x00FF0000);
        return;
    }

    if(strcmp(input, "color green") == 0){
        console_set_color(0x0000FF00);
        return;
    }

    if(strcmp(input, "color blue") == 0){
        console_set_color(0x000000FF);
        return;
    }

    if(strcmp(input, "color white") == 0){
        console_set_color(0x00FFFFFF);
        return;
    }

    // ===== help =====
    if(strcmp(input, "help") == 0){
        console_write("\nCommands:\n");
        console_write("clear\n");
        console_write("/UIR\n");
        console_write("color red/green/blue/white\n");
        return;
    }

    console_write("\nUnknown command\n");
}