#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------------------------
#define MAX_PROGRAM_SIZE 1000
#define STACK_SIZE 256
#define MAX_LABELS 100
//--------------------------------------------------------------------------------------------
typedef struct {
    int buf[STACK_SIZE];
    int sp;
} Stack;

void push(Stack* stack, int number) {
    stack->sp++;
    stack->buf[stack->sp] = number;
}

int pop(Stack* stack) {
    int number = stack->buf[stack->sp];
    stack->sp--;
    return number;
}

int top(Stack* stack) {
    return stack->buf[stack->sp];
}

//---------------------------------------------------------------------------------------------

int get_label_address(char* labels[], int label_addresses[], int label_count, char* label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i], label) == 0) {
            return label_addresses[i];
        }
    }
    return -1;
}

//----------------------------------------------------------------------------------------------

void interpret(char* program[], int program_size, char* labels[], int label_addresses[], int label_count) {
    int pc = 0;
    Stack stack = {.sp = -1};

    while (pc < program_size && strcmp(program[pc], "HALT") != 0) {
        char* opcode = program[pc++];

        if (strcmp(opcode, "PUSH") == 0) {
            int number = atoi(program[pc++]);
            push(&stack, number);

        } else if (strcmp(opcode, "POP") == 0) {
            pop(&stack);

        } else if (strcmp(opcode, "SWAP") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            push(&stack, a);
            push(&stack, b);

        } else if (strcmp(opcode, "ADD") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            push(&stack, a + b);

        } else if (strcmp(opcode, "MUL") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            push(&stack, a * b);

        } else if (strcmp(opcode, "SUB") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            push(&stack, b - a);

        } else if (strcmp(opcode, "DIV") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            if (a != 0 )
                push(&stack, b / a);
            else
                printf("divide by 0 error\n");

        } else if (strcmp(opcode, "MOD") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            if (a != 0 )
                push(&stack, b % a);
            else
                printf("divide by 0 error\n");

        } else if (strcmp(opcode, "DUP") == 0) {
            int a = top(&stack);
            push(&stack, a);

        } else if (strcmp(opcode, "DUP2") == 0) {
            int a = pop(&stack);
            int b = pop(&stack);
            push(&stack, b);
            push(&stack, a);
            push(&stack, b);
            push(&stack, a);
            
        } else if (strcmp(opcode, "PRINT") == 0) {
            char* str = program[pc++];
            for (char* p = str; *p != '\0'; p++) {
                if (*p == '\\' && *(p + 1) == 'n') {
                    putchar('\n');
                    p++;
                } else if (*p == '\\' && *(p + 1) == 't') {
                    putchar('\t');
                    p++;
                } else {
                    putchar(*p);
                }
            }
        } else if (strcmp(opcode, "TOP") == 0) {
            printf("%d", top(&stack));

        } else if (strcmp(opcode, "READ") == 0) {
            int number;
            scanf("%d", &number);
            push(&stack, number);

        } else if (strcmp(opcode, "JUMP.EQ.0") == 0) {
            char* label = program[pc++];
            int number = top(&stack);
            if (number == 0) {
                pc = get_label_address(labels, label_addresses, label_count, label);
            }

        } else if (strcmp(opcode, "JUMP.GT.0") == 0) {
            char* label = program[pc++];
            int number = top(&stack);
            if (number > 0) {
                pc = get_label_address(labels, label_addresses, label_count, label);
            }

        } else if (strcmp(opcode, "JUMP.LS.0") == 0) {
            char* label = program[pc++];
            int number = top(&stack);
            if (number < 0) {
                pc = get_label_address(labels, label_addresses, label_count, label);
            }

        } else if (strcmp(opcode, "JUMP") == 0) {
            char* label = program[pc++];
            pc = get_label_address(labels, label_addresses, label_count, label);

        } else {
            fprintf(stderr, "Unexpected opcode received: %s\n", opcode);
            exit(1);
        }
    }
}

//--------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Could not open file");
        return 1;
    }

    char* program[MAX_PROGRAM_SIZE];
    char* labels[MAX_LABELS];
    int label_addresses[MAX_LABELS];
    int program_size = 0;
    int label_count = 0;
    char line[256];
    int token_counter = 0;

    while (fgets(line, sizeof(line), file)) {
        char* opcode = strtok(line, " \t\n");

        if (opcode == NULL) {
            continue;
        }

        if (opcode[strlen(opcode) - 1] == ':') {
            opcode[strlen(opcode) - 1] = '\0';  
            labels[label_count] = strdup(opcode);
            label_addresses[label_count] = token_counter;
            label_count++;
            continue;
        }

        program[token_counter++] = strdup(opcode);

        if (strcmp(opcode, "PUSH") == 0) {
            char* number_str = strtok(NULL, " \t\n");
            program[token_counter++] = strdup(number_str);

        } else if (strcmp(opcode, "PRINT") == 0) {
            char* string_literal = strtok(NULL, "\"");
            program[token_counter++] = strdup(string_literal);

        } else if (strcmp(opcode, "JUMP.EQ.0") == 0 || strcmp(opcode, "JUMP.GT.0") == 0 || strcmp(opcode, "JUMP.LS.0") == 0 || strcmp(opcode, "JUMP") == 0) {
            char* label = strtok(NULL, " \t\n");
            program[token_counter++] = strdup(label);
        }
    }
    fclose(file);

    interpret(program, token_counter, labels, label_addresses, label_count);

    for (int i = 0; i < token_counter; i++) {
        free(program[i]);
    }
    for (int i = 0; i < label_count; i++) {
        free(labels[i]);
    }

    return 0;
}