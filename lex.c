#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "compiler.h"

#define MAX_NUMBER_TOKENS 1000
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5
#define MAX_NUMBER_RESWORDS 13
#define MAX_RESWORD_LEN 10
#define MAX_SPECIAL_LEN 2
#define MAX_NUMBER_SPECIAL 19

void printlexerror (int type);
void printtokens (void);
void skip_comment_section (char * input);
int pass_reserved_name (char * input);
void pass_identifier_name (char * input);
void pass_numerical_name (char * input);
void pass_special_name (char * input);

lexeme * list;
int lex_index;
int file_index;

char word[][MAX_RESWORD_LEN] = {"begin", "call", "const", "do", "else", "end", "if", "procedure", "read", "then", "var", "while", "write"};
char special[][MAX_SPECIAL_LEN] = {"==", "!", "!=", "<", "<=", ">", ">=", "*", "/", "+", "-", "(", ")", ",", ".", ";", ":=", "&&", "||"};
int wsym[] = {beginsym, callsym, constsym, dosym, elsesym, endsym, ifsym, procsym, readsym, thensym, varsym, whilesym, writesym};
int ssym[] = {eqlsym, notsym, neqsym, lsssym, leqsym, gtrsym, geqsym, multsym, divsym, plussym, minussym, lparensym, rparensym, commasym, periodsym, semicolonsym, assignsym, andsym, orsym};

lexeme * lexanalyzer (char * input, int printFlag) {
    list = malloc (sizeof (lexeme) * MAX_NUMBER_TOKENS);
    lex_index = 0;
    file_index = 0;
    int res_word = 0;
    while (input[file_index] != '\0') {
        if (iscntrl (input[file_index]) || isspace (input[file_index])) {
            file_index++;
            continue;
        }
        if (input[file_index] == '/' && input[file_index + 1] == '*') {
            file_index += 2;
            skip_comment_section (input);
            continue;
        }
        if (isalpha (input[file_index])) {
            res_word = pass_reserved_name (input);
            if (!res_word) pass_identifier_name (input);
            continue;
        }
        if (isdigit (input[file_index])) {
            pass_numerical_name (input);
            continue;
        }
        if (!isalnum (input[file_index])) {
            pass_special_name (input);
            continue;
        }
    }
    if (printFlag) printtokens();
    list[lex_index].type = -1;
    return list;
}
void skip_comment_section (char * input) {
    while (input[file_index] != '\0') {
        if (input[file_index] == '*' && input[file_index + 1] == '/') {
            file_index += 2;
            return;
        }
        file_index++;
    }
    if (input[file_index] == '\0') {
        printlexerror (5);
        exit (1);
    }
}
int pass_reserved_name (char * input) {
    int i = 0;
    int j = 0;
    while (i < MAX_NUMBER_RESWORDS) {
        if (input[file_index] == word[i][j]) {
            j++;
            while (j < strlen (word[i])) {
                if (!isalpha (input[file_index + j])) return 0;
                if (input[file_index + j] != word[i][j]) break;
                j++;
            }
        }
        if (j == strlen (word[i])) {
            if (isspace (input[file_index + j]) || !isalnum (input[file_index + j])) {
                strcpy (list[lex_index].name, word[i]);
                list[lex_index].value = wsym[i];
                list[lex_index].type = wsym[i];
                lex_index++;
                file_index += strlen (word[i]);
                return 1;
            }
        }
        j = 0;
        i++;
    }
    return 0;
}
void pass_identifier_name (char * input) {
    int i = 0;
    list[lex_index].value = identsym;
    list[lex_index].type = identsym;
    while (i < MAX_IDENT_LEN) {
        if (isspace (input[file_index]) || iscntrl (input[file_index]) || !isalnum (input[file_index])) break;
        list[lex_index].name[i] = input[file_index];
        file_index++;
        i++;
    }
    if (i == MAX_IDENT_LEN) {
        printlexerror (3);
        exit (1);
    }
    list[lex_index].name[i] = '\0';
    lex_index++;
}
void pass_numerical_name (char * input) {
    int i = 0;
    list[lex_index].type = numbersym;
    while (i < MAX_NUMBER_LEN) {
        if (isalpha (input[file_index])) {
            printlexerror (1);
            exit (1);
        }
        if (isspace (input[file_index]) || iscntrl (input[file_index]) || !isdigit (input[file_index])) break;
        list[lex_index].name[i] = input[file_index];
        file_index++;
        i++;
    }
    list[lex_index].name[i] = '\0';
    if (i == MAX_NUMBER_LEN) {
        printlexerror (2);
        exit (1);
    }
    file_index -= i;
    while (i > 0) {
        list[lex_index].value += (input[file_index] - '0') * pow (10.0, (double) (i - 1));
        file_index++;
        i--;
    }
    lex_index++;
}
void pass_special_name (char * input) {
    int i = 0, error = 0, incFile = 1;
    while (i < MAX_NUMBER_SPECIAL) {
        if (input[file_index] == special[i][0]) {
            if (ssym[i] == eqlsym) {
                if (input[file_index + 1] != '=') error = 1;
                else incFile = 2;
                break;
            }
            if (ssym[i] == notsym) {
                if (input[file_index + 1] == '=') {
                    i++;
                    incFile = 2;
                }
                break;
            }
            if (ssym[i] == lsssym || ssym[i] == gtrsym) {
                if (input[file_index + 1] == '=') {
                    i++;
                    incFile = 2;
                }
                break;
            }
            if (ssym[i] == assignsym) {
                if (input[file_index + 1] != '=') error = 1;
                else incFile = 2;
                break;
            }
            if (ssym[i] == andsym) {
                if (input[file_index + 1] != '&') error= 1;
                else incFile = 2;
                break;
            }
            if (ssym[i] == orsym) {
                if (input[file_index + 1] != '|') error= 1;
                else incFile = 2;
                break;
            }
            break;
        }
        i++;
    }
    if (error || i == MAX_NUMBER_SPECIAL) {
        printlexerror (4);
        exit (1);
    }
    strcpy (list[lex_index].name, special[i]);
    list[lex_index].value = ssym[i];
    list[lex_index].type = ssym[i];
    lex_index++;
    file_index += incFile;
}
void printtokens () {
    int i;
    printf("Lexeme Table:\n");
    printf("lexeme\t\ttoken type\n");
    for (i = 0; i < lex_index; i++)
    {
        switch (list[i].type)
        {
            case eqlsym:
                printf("%11s\t%d", "==", eqlsym);
                break;
            case neqsym:
                printf("%11s\t%d", "!=", neqsym);
                break;
            case lsssym:
                printf("%11s\t%d", "<", lsssym);
                break;
            case leqsym:
                printf("%11s\t%d", "<=", leqsym);
                break;
            case gtrsym:
                printf("%11s\t%d", ">", gtrsym);
                break;
            case geqsym:
                printf("%11s\t%d", ">=", geqsym);
                break;
            case multsym:
                printf("%11s\t%d", "*", multsym);
                break;
            case divsym:
                printf("%11s\t%d", "/", divsym);
                break;
            case plussym:
                printf("%11s\t%d", "+", plussym);
                break;
            case minussym:
                printf("%11s\t%d", "-", minussym);
                break;
            case lparensym:
                printf("%11s\t%d", "(", lparensym);
                break;
            case rparensym:
                printf("%11s\t%d", ")", rparensym);
                break;
            case commasym:
                printf("%11s\t%d", ",", commasym);
                break;
            case periodsym:
                printf("%11s\t%d", ".", periodsym);
                break;
            case semicolonsym:
                printf("%11s\t%d", ";", semicolonsym);
                break;
            case assignsym:
                printf("%11s\t%d", ":=", assignsym);
                break;
            case beginsym:
                printf("%11s\t%d", "begin", beginsym);
                break;
            case endsym:
                printf("%11s\t%d", "end", endsym);
                break;
            case ifsym:
                printf("%11s\t%d", "if", ifsym);
                break;
            case thensym:
                printf("%11s\t%d", "then", thensym);
                break;
            case elsesym:
                printf("%11s\t%d", "else", elsesym);
                break;
            case whilesym:
                printf("%11s\t%d", "while", whilesym);
                break;
            case dosym:
                printf("%11s\t%d", "do", dosym);
                break;
            case callsym:
                printf("%11s\t%d", "call", callsym);
                break;
            case writesym:
                printf("%11s\t%d", "write", writesym);
                break;
            case readsym:
                printf("%11s\t%d", "read", readsym);
                break;
            case constsym:
                printf("%11s\t%d", "const", constsym);
                break;
            case varsym:
                printf("%11s\t%d", "var", varsym);
                break;
            case procsym:
                printf("%11s\t%d", "procedure", procsym);
                break;
            case identsym:
                printf("%11s\t%d", list[i].name, identsym);
                break;
            case numbersym:
                printf("%11d\t%d", list[i].value, numbersym);
                break;
            case andsym:
                printf("%11s\t%d", "&&", andsym);
                break;
            case orsym:
                printf("%11s\t%d", "||", orsym);
                break;
            case notsym:
                printf("%11s\t%d", "!", notsym);
                break;
        }
        printf("\n");
    }
    printf("\n");
    printf("Token List:\n");
    for (i = 0; i < lex_index; i++)
    {
        if (list[i].type == numbersym)
            printf("%d %d ", numbersym, list[i].value);
        else if (list[i].type == identsym)
            printf("%d %s ", identsym, list[i].name);
        else
            printf("%d ", list[i].type);
    }
    printf("\n");
}
void printlexerror(int type) {
    if (type == 1)
        printf("Lexical Analyzer Error: Invalid Identifier\n");
    else if (type == 2)
        printf("Lexical Analyzer Error: Number Length\n");
    else if (type == 3)
        printf("Lexical Analyzer Error: Identifier Length\n");
    else if (type == 4)
        printf("Lexical Analyzer Error: Invalid Symbol\n");
    else if (type == 5)
        printf("Lexical Analyzer Error: Never-ending comment\n");
    else
        printf("Implementation Error: Unrecognized Error Type\n");
    free(list);
    return;
}
