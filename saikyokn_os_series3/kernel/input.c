#define INPUT_BUF 256

volatile char input_buf[INPUT_BUF];
volatile int input_head = 0;
volatile int input_tail = 0;

void input_push(char c){
    input_buf[input_head] = c;
    input_head = (input_head + 1) % INPUT_BUF;
}

int input_pop(){
    if(input_head == input_tail) return -1;

    char c = input_buf[input_tail];
    input_tail = (input_tail + 1) % INPUT_BUF;
    return c;
}