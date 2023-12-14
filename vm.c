#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#define MAX_REG_LENGTH 10
#define MAX_DATA_LENGTH 50
#define MAX_PROGRAM_LENGTH 150

int data_stack[MAX_DATA_LENGTH], register_stack[MAX_REG_LENGTH], BP, SP, PC, RP;
char ISA_list[][4] = {"LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO"};
char OPR_list[][4] = {"RET", "NEG", "ADD", "SUB", "MUL", "DIV", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ", "AND", "ORR", "NOT"};
char SIO_list[][4] = {"WRT", "RED", "HAL"};

void print_execution (int line, char * opname, instruction IR, int PC, int BP, int SP, int RP, int * data_stack, int * register_stack) {
    int i = 0;
    printf ("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t\t", line, opname, IR.l, IR.m, PC, BP, SP, RP);
    for (i = MAX_REG_LENGTH - 1; i >= RP; i--) printf ("%d ", register_stack[i]);
    printf ("\n");
    printf ("\tdata stack : ");
    for (i = 0; i <= SP; i++) printf ("%d ", data_stack[i]);
    printf ("\n");
}
int base (int L, int BP, int * data_stack) {
    int ctr = L, rtn = BP;
    while (ctr > 0) {
        rtn = data_stack[rtn];
        ctr--;
    }
    return rtn;
}
void execute_program (instruction * code, int printFlag) {
    int haltFlag = 0, cIndex = 0;
    instruction IR;
    BP = 0;
    SP = -1;
    PC = 0;
    RP = MAX_REG_LENGTH;
    printf ("\t\t\t\tPC\tBP\tSP\tRP\n");
    printf ("Initial values:\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, RP);
    while (!haltFlag) {
        IR.opcode = code[PC].opcode;
        IR.l = code[PC].l;
        IR.m = code[PC].m;
        cIndex = PC;
        PC++;
        if (IR.opcode != 2 && IR.opcode != 9) {
            switch (IR.opcode) {
                case 1:
                    --RP;
                    register_stack[RP] = IR.m;
                    break;
                case 3:
                    --RP;
                    register_stack[RP] = data_stack[base(IR.l, BP, data_stack) + IR.m];
                    break;
                case 4:
                    data_stack[base(IR.l, BP, data_stack) + IR.m] = register_stack[RP];
                    ++RP;
                    break;
                case 5:
                    data_stack[SP + 1] = base (IR.l, BP, data_stack);
                    data_stack[SP + 2] = BP;
                    data_stack[SP + 3] = PC;
                    BP = SP + 1;
                    PC = IR.m;
                    break;
                case 6:
                    SP += IR.m;
                    break;
                case 7:
                    PC = IR.m;
                    break;
                default:
                    if (!register_stack[RP]) PC = IR.m;
                    ++RP;
                    break;
            }
        }
        else if (IR.opcode == 2) {
            switch (IR.m) {
                case 0:
                    SP = BP - 1;
                    BP = data_stack[SP + 2];
                    PC = data_stack[SP + 3];
                    break;
                case 1:
                    register_stack[RP] = -register_stack[RP];
                    break;
                case 2:
                    ++RP;
                    register_stack[RP] += register_stack[RP - 1];
                    break;
                case 3:
                    ++RP;
                    register_stack[RP] -= register_stack[RP - 1];
                    break;
                case 4:
                    ++RP;
                    register_stack[RP] *= register_stack[RP - 1];
                    break;
                case 5:
                    ++RP;
                    register_stack[RP] /= register_stack[RP - 1];
                    break;
                case 6:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] == register_stack[RP - 1]);
                    break;
                case 7:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] != register_stack[RP - 1]);
                    break;
                case 8:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] < register_stack[RP - 1]);
                    break;
                case 9:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] <= register_stack[RP - 1]);
                    break;
                case 10:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] > register_stack[RP - 1]);
                    break;
                case 11:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] >= register_stack[RP - 1]);
                    break;
                case 12:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] && register_stack[RP - 1]);
                    break;
                case 13:
                    ++RP;
                    register_stack[RP] = (register_stack[RP] || register_stack[RP - 1]);
                    break;
                default:
                    register_stack[RP] = !(register_stack[RP]);
                    break;
            }
        }
        else if (IR.opcode == 9) {
            switch (IR.m) {
                case 1:
                    printf ("Top of Stack Value: %d\n", register_stack[RP]);
                    ++RP;
                    break;
                case 2:
                    --RP;
                    printf ("Please Enter an Integer: \n");
                    scanf ("%d", &register_stack[RP]);
                    break;
                default:
                    haltFlag = 1;
                    break;
            }
        }
        if (IR.opcode == 2)
            print_execution (cIndex, OPR_list[IR.m], IR, PC, BP, SP, RP, data_stack, register_stack);
        else if (IR.opcode == 9) print_execution (cIndex, SIO_list[IR.m - 1], IR, PC, BP, SP, RP, data_stack, register_stack);        else print_execution (cIndex, ISA_list[IR.opcode - 1], IR, PC, BP, SP, RP, data_stack, register_stack);
    }
    return;
}
