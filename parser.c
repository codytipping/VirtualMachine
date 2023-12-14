#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 150
#define MAX_SYMBOL_COUNT 50
#define MAX_REG_HEIGHT 10

int lexIdx;
int cIndex;
int tIndex;
int level;
int stackSize;
instruction * code;
symbol * table;

void program (lexeme * list);
void block (lexeme * list);
void const_declaration (lexeme * list);
int var_declaration (lexeme * list);
void procedure_declaration (lexeme * list);
void statement (lexeme * list);
void logic (lexeme * list);
void condition (lexeme * list);
int rel_op (lexeme * list);
void expression (lexeme * list);
void term (lexeme * list);
void factor (lexeme * list);
void emit (int opname, int level, int mvalue);
void addToSymbolTable (int k, char n[], int v, int l, int a, int m);
void mark (void);
int multipledeclarationcheck (char name[]);
int findsymbol (char name[], int kind);
void printparseerror (int err_code);
void printsymboltable (void);
void printassemblycode (void);

instruction * parse (lexeme * list, int printTable, int printCode) {
    code = malloc (sizeof (instruction) * MAX_CODE_LENGTH);
    table = malloc (sizeof (symbol) * MAX_SYMBOL_COUNT);
    stackSize = 0;
    tIndex = 0;
    cIndex = 0;
    lexIdx = -1;
    level = -1;
    program (list);
    if (printTable) printsymboltable();
    if (printCode) printassemblycode();
    code[cIndex].opcode = -1;
    return code;
}
void program (lexeme * list) {
    emit (7, 0, 0);
    addToSymbolTable (3, "main", 0, 0, 0, 0);
    block (list);
    lexIdx++;
    if (list[lexIdx].type == periodsym) emit (9, 0, 3);
    else {
        printparseerror (1);
        exit (1);
    }
    code[0].m = table[0].addr;
    for (int c = 1; c < cIndex; c++) {
        if (code[c].opcode == 5) code[c].m = table[code[c].m].addr;
    }
    return;
}
void block (lexeme * list) {
    level++;
    int procIndex = tIndex - 1;
    const_declaration (list);
    int x = var_declaration (list);
    procedure_declaration (list);
    table[procIndex].addr = cIndex;
    emit (6, 0, x + 3);
    statement (list);
    mark();
    level--;
    return;
}
void const_declaration (lexeme * list) {
    lexIdx++;
    if (list[lexIdx].type == constsym) {
        do {
            lexIdx++;
            if (list[lexIdx].type == identsym) {
                if (multipledeclarationcheck (list[lexIdx].name) != -1) {
                    printparseerror (18);
                    exit (1);
                }
                addToSymbolTable (1, list[lexIdx].name, 0, level, 0, 0);
            }
            else {
                printparseerror (2);
                exit (1);
            }
            lexIdx++;
            if (list[lexIdx].type != assignsym) {
                printparseerror (2);
                exit (1);
            }
            lexIdx++;
            if (list[lexIdx].type == numbersym) table[tIndex - 1].val = list[lexIdx].value;
            else {
                printparseerror (2);
                exit (1);
            }
            lexIdx++;
        } while (list[lexIdx].type == commasym);
        if (list[lexIdx].type == semicolonsym) return;
        else if (list[lexIdx].type == identsym) {
            printparseerror (13);
            exit (1);
        }
    }
    else lexIdx--;
    return;
}
int var_declaration (lexeme * list) {
    int numVar = 0;
    lexIdx++;
    if (list[lexIdx].type == varsym) {
        do {
            lexIdx++;
            if (list[lexIdx].type == identsym) {
                if (multipledeclarationcheck (list[lexIdx].name) != -1) {
                    printparseerror (18);
                    exit (1);
                }
                addToSymbolTable (2, list[lexIdx].name, 0, level, numVar + 3, 0);
                numVar++;
            }
            else {
                printparseerror (3);
                exit (1);
            }
            lexIdx++;
        } while (list[lexIdx].type == commasym);
        if (list[lexIdx].type == semicolonsym) return numVar;
        else if (list[lexIdx].type == assignsym) {
            printparseerror (13);
            exit (1);
        }
        else {
            printparseerror (14);
            exit (1);
        }
    }
    else lexIdx--;
    return numVar;
}
void procedure_declaration (lexeme * list) {
    lexIdx++;
    while (list[lexIdx].type == procsym) {
        lexIdx++;
        if (list[lexIdx].type == identsym) {
            if (multipledeclarationcheck (list[lexIdx].name) != -1) {
                printparseerror (18);
                exit (1);
            }
            addToSymbolTable (3, list[lexIdx].name, 0, level, 0, 0);
        }
        else {
            printparseerror (14);
            exit (1);
        }
        lexIdx++;
        if (list[lexIdx].type != semicolonsym) {
            printparseerror (4);
            exit (1);
        }
        block (list);
        lexIdx++;
        if (list[lexIdx].type == semicolonsym) emit (2, 0, 0);
        else {
            printparseerror (14);
            exit (1);
        }
        lexIdx++;
    }
    lexIdx--;
    return;
}
void statement (lexeme * list) {
    int symIdx, jpcIdx, jmpIdx, loopIdx;
    lexIdx++;
    if (list[lexIdx].type == identsym) {
        symIdx = findsymbol (list[lexIdx].name, 2);
        if (symIdx == -1) {
            if ((findsymbol (list[lexIdx].name, 1) != -1) || (findsymbol (list[lexIdx].name, 3) != -1)) {
                printparseerror (6);
                exit (1);
            }
            else {
                printparseerror (19);
                exit (1);
            }
        }
        lexIdx++;
        if (list[lexIdx].type != assignsym) {
            printparseerror (5);
            exit (1);
        }
        expression (list);
        emit (4, level - table[symIdx].level, table[symIdx].addr);
        stackSize--;
    }
    else if (list[lexIdx].type == callsym) {
        lexIdx++;
        if (list[lexIdx].type == identsym) {
            symIdx = findsymbol (list[lexIdx].name, 3);
            if (symIdx == -1) {
                if ((findsymbol (list[lexIdx].name, 1) != -1) || (findsymbol (list[lexIdx].name, 2) != -1)) {
                    printparseerror (7);
                    exit (1);
                }
                else {
                    printparseerror (19);
                    exit (1);
                }
            }
        }
        else {
            printparseerror (7);
            exit (1);
        }
        emit (5, level - table[symIdx].level, symIdx);
    }
    else if (list[lexIdx].type == beginsym) {
        do {
            statement (list);
            lexIdx++;
        } while (list[lexIdx].type == semicolonsym);
        if (list[lexIdx].type == identsym ||
            list[lexIdx].type == readsym ||
            list[lexIdx].type == writesym ||
            list[lexIdx].type == beginsym ||
            list[lexIdx].type == callsym ||
            list[lexIdx].type == ifsym ||
            list[lexIdx].type == whilesym) {
            printparseerror (15);
            exit (1);
        }
        else if (!(list[lexIdx].type == endsym ||
            list[lexIdx].type == identsym ||
            list[lexIdx].type == readsym ||
            list[lexIdx].type == writesym ||
            list[lexIdx].type == beginsym ||
            list[lexIdx].type == callsym ||
            list[lexIdx].type == ifsym ||
            list[lexIdx].type == whilesym)) {
            printparseerror (16);
            exit (1);
        }
    }
    else if (list[lexIdx].type == ifsym) {
        logic (list);
        jpcIdx = cIndex;
        emit (8, 0, 0);
        stackSize--;
        lexIdx++;
        if (list[lexIdx].type == thensym) statement (list);
        else {
            printparseerror (8);
            exit (1);
        }
        lexIdx++;
        if (list[lexIdx].type == elsesym) {
            jmpIdx = cIndex;
            emit (7, 0, 0);
            code[jpcIdx].m = cIndex;
            statement (list);
            code[jmpIdx].m = cIndex;
        }
        else {
            code[jpcIdx].m = cIndex;
            lexIdx--;
        }
    }
    else if (list[lexIdx].type == whilesym) {
        loopIdx = cIndex;
        logic (list);
        jpcIdx = cIndex;
        emit (8, 0, 0);
        stackSize--;
        lexIdx++;
        if (list[lexIdx].type == dosym) statement (list);
        else {
            printparseerror (9);
            exit (1);
        }
        emit (7, 0, loopIdx);
        code[jpcIdx].m = cIndex;
    }
    else if (list[lexIdx].type == readsym) {
        lexIdx++;
        if (list[lexIdx].type == identsym) {
            symIdx = findsymbol (list[lexIdx].name, 2);
            if (symIdx == -1) {
                if ((findsymbol (list[lexIdx].name, 1) != -1) || (findsymbol (list[lexIdx].name, 3) != -1)) {
                    printparseerror (6);
                    exit (1);
                }
                else {
                    printparseerror (19);
                    exit (1);
                }
            }
            emit (9, 0, 2);
            stackSize++;
            if (stackSize > MAX_REG_HEIGHT) {
                printparseerror (20);
                exit (1);
            }
            emit (4, level - table[symIdx].level, table[symIdx].addr);
            stackSize--;
        }
        else {
            printparseerror (6);
            exit (1);
        }
    }
    else if (list[lexIdx].type == writesym) {
        expression (list);
        emit (9, 0, 1);
    }
    else lexIdx--;
    return;
}
void logic (lexeme * list) {
    lexIdx++;
    if (list[lexIdx].type == notsym) {
        condition (list);
        emit (2, 0, 14);
        stackSize--;
        return;
    }
    else {
        lexIdx--;
        condition (list);
        lexIdx++;
        while (list[lexIdx].type == andsym || list[lexIdx].type == orsym) {
            if (list[lexIdx].type == andsym) {
                condition (list);
                emit (2, 0, 12);
                stackSize--;
            }
            else if (list[lexIdx].type == orsym) {
                condition (list);
                emit (2, 0, 13);
                stackSize--;
            }
            lexIdx++;
        }
    }
    lexIdx--;
    return;
}
void condition (lexeme * list) {
    lexIdx++;
    if (list[lexIdx].type == lparensym) {
        logic (list);
        lexIdx++;
        if (list[lexIdx].type != rparensym) {
            printparseerror (12);
            exit (1);
        }
    }
    else {
        lexIdx--;
        expression (list);
        int relop = rel_op (list);
        expression (list);
        emit (2, 0, relop);
        stackSize--;
    }
    return;
}
int rel_op (lexeme * list) {
    int relop;
    lexIdx++;
    if (list[lexIdx].type == eqlsym) relop = 6;
    else if (list[lexIdx].type == neqsym) relop = 7;
    else if (list[lexIdx].type == lsssym) relop = 8;
    else if (list[lexIdx].type == leqsym) relop = 9;
    else if (list[lexIdx].type == gtrsym) relop = 10;
    else if (list[lexIdx].type == geqsym) relop = 11;
    else {
        printparseerror (10);
        exit (1);
    }
    return relop;
}
void expression (lexeme * list) {
    lexIdx++;
    if (list[lexIdx].type == minussym) {
        term (list);
        emit (2, 0, 1);
    }
    else if (list[lexIdx].type == plussym) term (list);
    else {
        lexIdx--;
        term (list);
    }
    lexIdx++;
    while (list[lexIdx].type == plussym || list[lexIdx].type == minussym) {
        if (list[lexIdx].type == plussym) {
            term (list);
            emit (2, 0, 2);
            stackSize--;
        }
        else if (list[lexIdx].type == minussym) {
            term (list);
            emit (2, 0, 3);
            stackSize--;
        }
        else lexIdx--;
        lexIdx++;
    }
    lexIdx--;
    return;
}
void term (lexeme * list) {
    factor (list);
    lexIdx++;
    while (list[lexIdx].type == multsym || list[lexIdx].type == divsym) {
        if (list[lexIdx].type == multsym) {
            factor (list);
            emit (2, 0, 4);
            stackSize--;
        }
        else if (list[lexIdx].type == divsym) {
            factor (list);
            emit (2, 0, 5);
            stackSize--;
        }
        lexIdx++;
    }
    lexIdx--;
    return;
}
void factor (lexeme * list) {
    lexIdx++;
    if (list[lexIdx].type == identsym) {
        if (findsymbol (list[lexIdx].name, 3) != -1) {
            printparseerror (11);
            exit (1);
        }
        int symCon = findsymbol (list[lexIdx].name, 1);
        int symVar = findsymbol (list[lexIdx].name, 2);
        if ((symCon == -1) && (symVar == -1)) {
            printparseerror (19);
            exit (1);
        }
        else if (symCon > symVar) {
            emit (1, 0, table[symCon].val);
            stackSize++;
            if (stackSize > MAX_REG_HEIGHT) {
                printparseerror (20);
                exit (1);
            }
        }
        else if (symVar > symCon) {
            emit (3, level - table[symVar].level, table[symVar].addr);
            stackSize++;
            if (stackSize > MAX_REG_HEIGHT) {
                printparseerror (20);
                exit (1);
            }
        }
    }
    else if (list[lexIdx].type == numbersym) {
        emit (1, 0, list[lexIdx].value);
        stackSize++;
        if (stackSize > MAX_REG_HEIGHT) {
            printparseerror (20);
            exit (1);
        }
    }
    else if (list[lexIdx].type == lparensym) {
        expression (list);
        lexIdx++;
        if (list[lexIdx].type != rparensym) {
            printparseerror (12);
            exit (1);
        }
    }
    else {
        printparseerror (11);
        exit (1);
    }
    return;
}
void emit (int opname, int level, int mvalue) {
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;
	cIndex++;
}
void addToSymbolTable (int k, char n[], int v, int l, int a, int m) {
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].val = v;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;
	tIndex++;
}
void mark (void) {
	int i;
	for (i = tIndex - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}
int multipledeclarationcheck(char name[]) {
	int i;
	for (i = 0; i < tIndex; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}
int findsymbol(char name[], int kind) {
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < tIndex; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}
void printparseerror(int err_code) {
	switch (err_code)
	{
		case 1:
			printf("Parser Error: Program must be closed by a period\n");
			break;
		case 2:
			printf("Parser Error: Constant declarations should follow the pattern 'ident := number {, ident := number}'\n");
			break;
		case 3:
			printf("Parser Error: Variable declarations should follow the pattern 'ident {, ident}'\n");
			break;
		case 4:
			printf("Parser Error: Procedure declarations should follow the pattern 'ident ;'\n");
			break;
		case 5:
			printf("Parser Error: Variables must be assigned using :=\n");
			break;
		case 6:
			printf("Parser Error: Only variables may be assigned to or read\n");
			break;
		case 7:
			printf("Parser Error: call must be followed by a procedure identifier\n");
			break;
		case 8:
			printf("Parser Error: if must be followed by then\n");
			break;
		case 9:
			printf("Parser Error: while must be followed by do\n");
			break;
		case 10:
			printf("Parser Error: Relational operator missing from condition\n");
			break;
		case 11:
			printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, constants, and variables\n");
			break;
		case 12:
			printf("Parser Error: ( must be followed by )\n");
			break;
		case 13:
			printf("Parser Error: Multiple symbols in variable and constant declarations must be separated by commas\n");
			break;
		case 14:
			printf("Parser Error: Symbol declarations should close with a semicolon\n");
			break;
		case 15:
			printf("Parser Error: Statements within begin-end must be separated by a semicolon\n");
			break;
		case 16:
			printf("Parser Error: begin must be followed by end\n");
			break;
		case 17:
			printf("Parser Error: Bad arithmetic\n");
			break;
		case 18:
			printf("Parser Error: Confliciting symbol declarations\n");
			break;
		case 19:
			printf("Parser Error: Undeclared identifier\n");
			break;
		case 20:
			printf("Parser Error: Register Overflow Error\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");
			break;
	}
	
	free(code);
	free(table);
}
void printsymboltable() {
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr, table[i].mark);
	free(table);
	table = NULL;
}
void printassemblycode() {
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < cIndex; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RET\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("EQL\t");
						break;
					case 7:
						printf("NEQ\t");
						break;
					case 8:
						printf("LSS\t");
						break;
					case 9:
						printf("LEQ\t");
						break;
					case 10:
						printf("GTR\t");
						break;
					case 11:
						printf("GEQ\t");
						break;
                    case 12:
                        printf("AND\t");
                        break;
                    case 13:
                        printf("ORR\t");
                        break;
                    case 14:
                        printf("NOT\t");
                        break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	if (table != NULL)
		free(table);
}
