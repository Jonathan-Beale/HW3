// Jonathan Beale
// 7/3/2024 HW3

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// given
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, fisym, eqlsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym, elsesym
} token_type;

#define MAX_IDENT_LEN 11
#define MAX_NUM_LEN 5

// Structure to hold a token
typedef struct token {
    token_type type;
    char lexeme[MAX_IDENT_LEN + 1];
    struct token *next;
    int index;
} token;
token *current_token = NULL;


// given
typedef struct {
  int kind;
  char name[10];
  int val;
  int level;
  int addr;
  int mark;
} symbol;

// symbol table and a counter for it
symbol *symbol_table[500];
int symbol_count = 0;

// Function declarations
token *tokenize(char *input); // Now returns a token pointer to the head of the list
token *createToken(const char *lexeme, token_type type);
void appendToken(token **head, token *newToken);
void freeTokenList(token *head);
int reserved(token **head, const char *str); // now treats call as an Identifier
void printTokenList(token *head); // no longer needs the input


// New function declarations
int typeCheck(token_type checktype);
int findSymbol();
int addSymbol(int kind, char *name, int value, int mark, int address);
void printSymTable();
void block();
void program();
void constDeclaration();
int varDeclaration();
void expression();
void factor();
void term();
void condition();
void statement();

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Expected: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // getting the contents of the file
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    
    // closing the file
    fclose(file);

    printf("Source Program:\n");
    printf("%s\n\n", buffer);

    printf("Lexeme Table:\n");
    printf("lexeme\t\t\ttoken type\n");

    // this is the important part
    token *head = tokenize(buffer);
    
    printTokenList(head);

    current_token = head;
    program();

    printSymTable();

    freeTokenList(head);

    free(buffer);
    return 0;
}

// creates a token with the given lexeme and type
token *createToken(const char *lexeme, token_type type) {
    token *newToken = (token *)malloc(sizeof(token));
    if (newToken == NULL) {
        fprintf(stderr, "Memory allocation failure.");
        exit(1);
    }
    strncpy(newToken->lexeme, lexeme, sizeof(newToken->lexeme) - 1);
    newToken->lexeme[sizeof(newToken->lexeme) - 1] = '\0';
    newToken->type = type;
    newToken->next = NULL;
    return newToken;
}

// appends tokens to the end of the token list
void appendToken(token **head, token *newToken) {
    if (*head == NULL) {
        newToken->index = 0;
        *head = newToken;
    } else {
        token *temp = *head;
        int i = 0;
        while (temp->next != NULL) {
            temp = temp->next;
            i++;
        }
        newToken->index = i;
        temp->next = newToken;
    }
    
    printf("%-16s\t%d\n", newToken->lexeme, newToken->type);
}

// self explanatory really
void freeTokenList(token *head) {
    while (head != NULL) {
        token *temp = head;
        head = head->next;
        free(temp);
    }
}

// appends a new token for reserved words, returns 0 otherwise
// commented out else, procedure and call so that they are treated as Identifiers
int reserved(token **head, const char *str) {
    if(!strcmp(str, "odd")) {
        appendToken(head, createToken(str, oddsym));
        return 1;
    } else if(!strcmp(str, "fi")) {
        appendToken(head, createToken(str, fisym));
        return 8;
    } else if(!strcmp(str, "begin")) {
        appendToken(head, createToken(str, beginsym));
        return 21;
    } else if(!strcmp(str, "end")) {
        appendToken(head, createToken(str, endsym));
        return 22;
    } else if(!strcmp(str, "if")) {
        appendToken(head, createToken(str, ifsym));
        return 23;
    } else if(!strcmp(str, "then")) {
        appendToken(head, createToken(str, thensym));
        return 24;
    } else if(!strcmp(str, "while")) {
        appendToken(head, createToken(str, whilesym));
        return 25;
    } else if(!strcmp(str, "do")) {
        appendToken(head, createToken(str, dosym));
        return 26;
    } /* else if(!strcmp(str, "call")) {
        appendToken(head, createToken(str, callsym));
        return 27;
    } */ else if(!strcmp(str, "const")) {
        appendToken(head, createToken(str, constsym));
        return 28;
    } else if(!strcmp(str, "var")) {
        appendToken(head, createToken(str, varsym));
        return 29;
    } /* else if(!strcmp(str, "procedure")) {
        appendToken(head, createToken(str, procsym));
        return 30;
    } */ else if(!strcmp(str, "write")) {
        appendToken(head, createToken(str, writesym));
        return 31;
    } else if(!strcmp(str, "read")) {
        appendToken(head, createToken(str, readsym));
        return 32;
    } /* else if(!strcmp(str, "else")) {
        appendToken(head, createToken(str, elsesym));
        return 33;
    } */ else return 0;
}

// prints the token list
void printTokenList(token *head) {
    token *temp = head;
    
    printf("\nToken List:\n");
    temp = head;
    while (temp != NULL) {
        printf("%d ", temp->type);
        if (temp->type == identsym || temp->type == numbersym) {
            printf("%s ", temp->lexeme);
        }
        temp = temp->next;
    }
    printf("\n\n");
}

// tokenizes the input and prints the contents of the lexeme table and token list
// now returns the head of the token list
// no longer frees and prints the token list
token *tokenize(char *input) {
    int i = 0;
    int length = strlen(input);
    token *head = NULL;
    while (i < length) {
        if (isspace(input[i])) {
            // Skip whitespace
            i++;
        } else if (isalpha(input[i])) {
            // must be an identifier
            int start = i;
            while (isalnum(input[i])) i++;
            int len = i - start;
            if (len > MAX_IDENT_LEN) { // identifier is too long
                printf("%-16.*s\tError: name too long\n", len, &input[start]);
            } else {
                char ident[MAX_IDENT_LEN + 1];
                strncpy(ident, &input[start], len);
                ident[len] = '\0';
                if (!reserved(&head, ident)) {
                    appendToken(&head, createToken(ident, identsym));
                }
            }
        } else if (isdigit(input[i])) {
            // must be a number
            int start = i;
            while (isdigit(input[i])) i++;
            int len = i - start;
            if (len > MAX_NUM_LEN) { // number is too long
                printf("%-16.*s\tError: Number too long\n", len, &input[start]);
            } else {
                char number[MAX_NUM_LEN + 1];
                strncpy(number, &input[start], len);
                number[len] = '\0';
                appendToken(&head, createToken(number, numbersym));
            }
        } else {
            // special symbols
            switch (input[i]) {
                case '+':
                    appendToken(&head, createToken("+", plussym));
                    break;
                case '-':
                    appendToken(&head, createToken("-", minussym));
                    break;
                case '*':
                    appendToken(&head, createToken("*", multsym));
                    break;
                case '/':
                    if (input[i + 1] == '*') { // comments
                        i += 2;
                        // skip over content
                        while (input[i] != '*' || input[i + 1] != '/') i++;
                        i += 2;
                    } else {
                        appendToken(&head, createToken("/", slashsym));
                    }
                    break;
                case '=':
                    appendToken(&head, createToken("=", eqlsym));
                    break;
                case '<':
                    if (input[i + 1] == '>') {
                        appendToken(&head, createToken("<>", neqsym));
                        i++;
                    } else if (input[i + 1] == '=') {
                        appendToken(&head, createToken("<=", leqsym));
                        i++;
                    } else {
                        appendToken(&head, createToken("<", lessym));
                    }
                    break;
                case '>':
                    if (input[i + 1] == '=') {
                        appendToken(&head, createToken(">=", geqsym));
                        i++;
                    } else {
                        appendToken(&head, createToken(">", gtrsym));
                    }
                    break;
                case '(':
                    appendToken(&head, createToken("(", lparentsym));
                    break;
                case ')':
                    appendToken(&head, createToken(")", rparentsym));
                    break;
                case ',':
                    appendToken(&head, createToken(",", commasym));
                    break;
                case ';':
                    appendToken(&head, createToken(";", semicolonsym));
                    break;
                case '.':
                    appendToken(&head, createToken(".", periodsym));
                    break;
                case ':':
                    if (input[i + 1] == '=') {
                        appendToken(&head, createToken(":=", becomessym));
                        i++;
                    } else { // ':' is not in the language
                        printf("%-16c\tError: Invalid symbol\n", input[i]);
                    }
                    break;
                default: // Unknown Symbol
                    printf("%-16c\tError: Invalid symbol\n", input[i]);
            }
            i++;
        }
    }
    // Moved the printing and freeing of the list to the main function
    return head;
}

// prints sym table
void printSymTable() {
    printf("\nSymbol Table:\n");
    printf("Kind\t| Name\t| Value\t| Level\t| Address\t| Mark\t\n");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < symbol_count; i++) {
    symbol_table[i]->mark = 1;
    printf("%d\t\t\t| %s\t\t\t| %d\t\t\t| %d\t\t\t| %d\t\t\t\t| %d\n",
           symbol_table[i]->kind, symbol_table[i]->name, symbol_table[i]->val,
           symbol_table[i]->level, symbol_table[i]->addr,
           symbol_table[i]->mark);
    }
}

// function adds symbols to sym table
int addSymbol(int kind, char *name, int value, int mark, int address) {
    symbol *newSym = (symbol *)malloc(sizeof(symbol));
    if (newSym == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        printf("Memory allocation failed\n");
        printSymTable();
        exit(1);
    }

    // set given parameters
    newSym->kind = kind;
    strncpy(newSym->name, name, sizeof(newSym->name) - 1);
    newSym->name[sizeof(newSym->name) - 1] = '\0';
    newSym->val = value;
    newSym->level = 0;
    newSym->addr = address;
    newSym->mark = mark;

    // add sym to table list
    symbol_table[symbol_count++] = newSym;
    return symbol_count;
}

// returns index of symbol if found, -1 otherwise
// same function as SYMBOLTABLECHECK in psuedo code
int findSymbol() {
    char name[12];
    strncpy(name, current_token->lexeme, sizeof(current_token->lexeme));
    name[sizeof(current_token->lexeme) - 1] = '\0';

    for (int i = symbol_count - 1; i > -1; i--) {
        if (strcmp(symbol_table[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Returns 1 if current_token's type matches check_type
// otherwise returns 0
int typeCheck(token_type checktype) {
    if (current_token->type == checktype) {
        return 1;
    } else {
        return 0;
    }
}

// program calls block and ensures following '.'
// prints beginning and end of the Assembly code
void program() {

    printf("Assembly Code:\n");
    printf("JMP\t0\t3\n");

    block();

    if(current_token != NULL) {
        // program must end in a period
        if (!typeCheck(periodsym)) {
            printf("Error: program should end with a '.' Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
    } else {
        printf("Error: program should end with a '.'\n");
        printSymTable();
        exit(1);
    }

    printf("SYS\t0\t3\n\n");
}

// Detects and sets constants
// looks for 'const x = 7;' or 'const x = 7, ..., y = 8;'
void constDeclaration() {
  if (typeCheck(constsym)) {

    do { // keeps adding constants as long as a comma follows the last const declaration
        current_token = current_token->next;

        if (!typeCheck(identsym)) {
            printf("Error: Identifier expected after 'const'. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }

        if (findSymbol() != -1) {
            printf("Error: Duplicate identifier '%s'.\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        token *identToken = current_token;
        current_token = current_token->next;

        if (!typeCheck(eqlsym)) {
            printf("Error: '=' expected after 'const' Identifier. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;

        if (!typeCheck(numbersym)) {
            printf("Error: Number expected after 'const' Ident '='. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }

        addSymbol(1, identToken->lexeme, atoi(current_token->lexeme), 0, 0);
        current_token = current_token->next;
    } while(typeCheck(commasym));

    // declaration must end in a semicolon
    if (!typeCheck(semicolonsym)) {
      printf("Error: ';' expected after declaration. Got "
             "'%s'\n",
             current_token->lexeme);
      printSymTable();
      exit(1);
    }
    current_token = current_token->next;
    
  }
}

// returns the number of variables set when the function is called
// detects and sets variables in a similar manner to the constDeclaration function
int varDeclaration() {
    int numVars = 0;
    if (typeCheck(varsym)) {
        do { // runs as long as a comma follows the last var declaration
            numVars++;
            current_token = current_token->next;
            
            if (!typeCheck(identsym)) {
                printf("Error: Identifier expected in var declaration. Got "
                    "'%s'\n",
                    current_token->lexeme);
                printSymTable();
                exit(1);
            }
            if (findSymbol() != -1) {
                printf("Error: Duplicate identifier '%s'.\n", current_token->lexeme);
                printSymTable();
                exit(1);
            }

            addSymbol(2, current_token->lexeme, 0, 0, numVars + 2);
            current_token = current_token->next;
        } while (typeCheck(commasym));

        // declaration must end in a semicolon
        if (!typeCheck(semicolonsym)) {
            printf("Error: Expected ';' after var declaration. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;

    }
    return numVars;
}

// checks for an Identifier, Number or (Expression)
void factor() {
    int tempIndex = 0;

    if (typeCheck(identsym)) { // Identifier
        int sym_index = findSymbol();
        if (sym_index == -1) {
            printf("Error: Undeclared identifier '%s'.\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        if (symbol_table[sym_index]->kind == 1) { // Constant
            printf("LIT\t0\t%d\n", symbol_table[sym_index]->val);

        } else { // Variable
            printf("LOD\t0\t%d\n", symbol_table[sym_index]->addr);
        }
        current_token = current_token->next;

    } else if (typeCheck(numbersym)) { // Number
        printf("LIT\t0\t%s\n", current_token->lexeme);
        current_token = current_token->next;


    } else if (typeCheck(lparentsym)) { // (Expression)
        current_token = current_token->next;
        if(typeCheck(rparentsym)) {
            printf("Error: Expected '(Expression)'. Got '()'");
        }

        expression();

        if (!typeCheck(rparentsym)) {
            printf("Error: ')' expected after '(' {expression}. Got '%s'\n",
                    current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;

    } else {
        if (typeCheck(rparentsym)) {
            printf("Error: expression needed '(' {expression} ')'\n");
            printSymTable();
            exit(1);
        }
        printf("Error: Invalid factor, must start with an Identifier, Number or '('. Got '%s'\n", current_token->lexeme);
        printSymTable();
        exit(1);
    }
}

// looks for factor {("*"|"/") factor}
void term() {
    factor();

    // repeats as long as the last factor is followed by a multsym or slashsym
    while (typeCheck(multsym) || typeCheck(slashsym)) {
        if (typeCheck(multsym)) {
            current_token = current_token->next;
            
            factor();
            printf("MUL\t0\t3\n");
        } else {
            current_token = current_token->next;
            
            factor();
            printf("DIV\t0\t4\n");
        }
    }
}

// looks for term {("+"|"-") factor}
void expression() {
    term();

    // repeats as long as the last term is followed by a plussym or minussym
    while (typeCheck(plussym) || typeCheck(minussym)) {
        if (typeCheck(plussym)) {
            current_token = current_token->next;
        
            term();
            printf("ADD\t0\t1\n");
        } else {
            current_token = current_token->next;
            
            term();
            printf("SUB\t0\t2\n");
        }
    }
}

// looks for "odd" expression | expression rel-op expression
void condition() {
    if (typeCheck(oddsym)) {
        current_token = current_token->next;
        expression();
        printf("ODD\t0\t11\n");
    } else {
        expression();
        if (typeCheck(eqlsym)) {
            current_token = current_token->next;
            expression();
            printf("EQL\t0\t5\n");
        } else if (typeCheck(neqsym)) {
            current_token = current_token->next;
            expression();
            printf("NEQ\t0\t6\n");
        } else if (typeCheck(lessym)) {
            current_token = current_token->next;
            expression();
            printf("LSS\t0\t7\n");
        } else if (typeCheck(leqsym)) {
            current_token = current_token->next;
            expression();
            printf("LEQ\t0\t8\n");
        } else if (typeCheck(gtrsym)) {
            current_token = current_token->next;
            expression();
            printf("GTR\t0\t9\n");
        } else if (typeCheck(geqsym)) {
            current_token = current_token->next;
            expression();
            printf("GEQ\t0\t10\n");
        } else {
            printf("Error: Expected an operator. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
    }
}

// checks for an optional statement
void statement() {
    int jpc_index = 0;

    if (typeCheck(identsym)) { // looks for ident ":=" expression
        int sym_index = findSymbol();
        if (sym_index == -1) {
            printf("Error: Undeclared identifier '%s'.\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        if (symbol_table[sym_index]->kind != 2) {
            printf("Error: only variables can be changed by assignment.\n");
            printSymTable();
            exit(1);
        }

        current_token = current_token->next;

        if (!typeCheck(becomessym)) {
            printf("Error: Expected ':=' after identifier. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }

        current_token = current_token->next;

        expression();
        printf("STO\t0\t%d\n", symbol_table[sym_index]->addr);
        return;
    }

    if (typeCheck(beginsym)) { // looks for "begin" statement {";" statement} "end"
        do {
            current_token = current_token->next;
            
            statement();
        } while (typeCheck(semicolonsym));

        if (!typeCheck(endsym)) {
            printf("Error: Expected 'end' after 'begin' statement. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;
        return;
    }

    if (typeCheck(ifsym)) { // looks for "if" condition "then" statement "fi"
        current_token = current_token->next;

        condition();
        jpc_index = current_token->index;
        printf("JPC\t0\t%d\n", jpc_index);
        if (!typeCheck(thensym)) {
            printf("Error: expected 'if' condition 'then'. Got 'if' condition '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;

        statement();

        if (!typeCheck(fisym)) {
            printf("Error: Expected 'if' condition 'then' statement 'fi'. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;
        return;
    }

    if (typeCheck(whilesym)) { // looks for "while" condition "do" statement
        current_token = current_token->next;

        int loop_index = current_token->index;
        condition();
        if (!typeCheck(dosym)) {
            printf("Error: Expected 'do' after 'while'. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;

        jpc_index = current_token->index;
        printf("JPC\t0\t%d\n", jpc_index);

        statement();

        printf("JMP\t0\t%d\n", loop_index);
        return;
    }

    if (typeCheck(readsym)) { // looks for "read" ident
        current_token = current_token->next;

        if (!typeCheck(identsym)) {
            printf("Error: Expected an identifier after 'read'. Got '%s'\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        int sym_index = findSymbol();
        if (sym_index == -1) {
            printf("Error: Undeclared identifier '%s'.\n", current_token->lexeme);
            printSymTable();
            exit(1);
        }
        if (symbol_table[sym_index]->kind != 2) {
            printf("Error: only variables can be read.\n");
            printSymTable();
            exit(1);
        }
        current_token = current_token->next;


        printf("SYS\t0\t3\n");
        printf("STO\t0\t%d\n", symbol_table[sym_index]->addr);
        return;
    }

    if (typeCheck(writesym)) { // looks for "write" expression
        current_token = current_token->next;
    
        expression();
        current_token = current_token->next;

        printf("SYS\t0\t2\n");
        return;
    }
}

// checks for constant declarations, then var declarations, then statements
void block() {
    constDeclaration();
    int numVars = varDeclaration();
    printf("INC\t0\t%d\n", numVars + 3);
    statement();
}