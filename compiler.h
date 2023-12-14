typedef enum token_type {
	eqlsym = 1, constsym, neqsym, varsym, lsssym, procsym, leqsym,
	callsym, gtrsym, ifsym, geqsym, thensym, multsym, elsesym,
	divsym, whilesym, plussym, dosym, minussym, beginsym, lparensym,
	endsym, rparensym, readsym, commasym, writesym, periodsym, identsym,
	semicolonsym, numbersym, assignsym, andsym, orsym, notsym
} token_type;

typedef struct lexeme {
	char name[12];
	int value;
	token_type type;
} lexeme;

typedef struct instruction {
	int opcode;
	int l;
	int m;
} instruction;

typedef struct symbol {
	int kind;
	char name[12];
	int val;
	int level;
	int addr;
	int mark;
} symbol;

lexeme *lexanalyzer(char *input, int printFlag);
instruction *parse(lexeme *list, int printTable, int printCode);
void execute_program(instruction *code, int printFlag);