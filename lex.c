#include <string.h>
#include "stdio.h"
#include <ctype.h>
#include <stdlib.h>

#define MAX_IDENT_LEN 30
#define MAX_INT_LEN 10

typedef enum {
    IDENTIFIER,
    INT_CONST,
    OPERATOR,
    LEFT_PAR,
    RIGHT_PAR,
    LEFT_SQUARE_BRACKET,
    RIGHT_SQUARE_BRACKET,
    LEFT_CURLY_BRACKET,
    RIGHT_CURLY_BRACKET,
    STRING_CONST,
    KEYWORD,
    ENDOFLINE,
    NO_TYPE
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[32];
} Token;

int isOperator (char ch, FILE *fp) {
  const char* operators[] = {"+", "-", "*", ":"};
  int numOperators = sizeof(operators) / sizeof(operators[0]);
  for (int i = 0; i < numOperators; i++) {
    if (ch == *operators[i]) {
      char nextCh = fgetc(fp);
      if((ch == '+' && nextCh == '+') ||
         (ch == '-' && nextCh == '-') ||
         (ch == ':' && nextCh == '=')) {
        ungetc(nextCh, fp);
        return 2;
      }
      ungetc(nextCh, fp);
      return 1;
    }
  }
  return 0;
}

void isKeyword (Token* token) {
  const char* keywords[] = {"break", "case", "char", "const", "continue", "do", "else", "enum", "float",
                          "for", "goto", "if", "int", "long", "record", "return", "static", "while"};
  for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    if (strcasecmp( keywords[i], (*token).lexeme) == 0) {
      (*token).type = KEYWORD;
      return;
    }
  }
}

Token getNextToken(FILE* fp, FILE* fwp){
  Token token;

  // Skip white spaces
  char ch = fgetc(fp);
  while (isspace(ch)) {
    ch = fgetc(fp);
  }

  // Check EOF
  if(ch == EOF){
    exit(0);
  }

  // Skip comments
  if (ch == '/') {
    char c = fgetc(fp);
    char nextc;
    if (c == '*') {
      do {
        if (nextc == EOF) {
          printf("Lexical Error: Comment cannot terminated!");
          break;
        }
        c = nextc;
        nextc = fgetc(fp);
      } while (!(c == '*' && nextc == '/'));
        token.type = NO_TYPE;
        return token;
    } else {
      ungetc(c, fp);
      token.type = OPERATOR;
      token.lexeme[0] = '/';
      token.lexeme[1] = '\0';
      return token;
    }
  }

  //check for end of line
  if (ch == ';') {
    token.type = ENDOFLINE;
    strcpy(token.lexeme, "EndOfLine"); //can't assign str to char[] directly!!
    ch = fgetc(fp);
    return token;
  }

  // Check for braces
  if (ch == '(') {
    token.type = LEFT_PAR;
    return token;
  }
  if (ch == ')') {
    token.type = RIGHT_PAR;
    return token;
  }
  if (ch == '[') {
    token.type = LEFT_SQUARE_BRACKET;
    return token;
  }
  if (ch == ']') {
    token.type = RIGHT_SQUARE_BRACKET;
    return token;
  }
  if (ch == '{') {
    token.type = LEFT_CURLY_BRACKET;
    return token;
  }
  if (ch == '}') {
    token.type = RIGHT_CURLY_BRACKET;
    return token;
  }


  // Check for identifier
  if (isalpha(ch)) { // Starts with letter
    int j = 0;
    while ((isalnum(ch) || ch == '_')) {
      token.lexeme[j++] = ch;
      if(j > MAX_IDENT_LEN) {
        printf("Identifiers must be smaller or equal than %d characters\n",MAX_IDENT_LEN);
        exit(1);
      }
      ch = fgetc(fp);
    }
    ungetc(ch, fp);
    token.lexeme[j] = '\0'; //null terminator, marks the end of a string
    token.type = IDENTIFIER;
    isKeyword(&token);
    return token;
  }

  // Check for operators
  int operatorStatus = isOperator(ch, fp);
  if (operatorStatus != 0) {
    if(operatorStatus == 1){
      if (ch == ':') {
        printf("Invalid operator, assignment operator must be used like ':='\n");
        exit(1);
      }
      token.lexeme[0] = ch;
      token.lexeme[1] = '\0';
    } else {
      token.lexeme[0] = ch;
      token.lexeme[1] = fgetc(fp);
      token.lexeme[2] = '\0';
    }
    token.type = OPERATOR;
    return token;
  }

  // Check for integer constants
  if (isdigit(ch)) {
    int j = 0;
    while (isdigit(ch)) {
      token.lexeme[j++] = ch;
      if(j > MAX_INT_LEN) {
        printf("Integers must be smaller or equal than %d digits\n",MAX_INT_LEN);
        exit(1);
      }
      ch = fgetc(fp);
    }
    ungetc(ch,fp);
    token.lexeme[j] = '\0';
    token.type = INT_CONST;
    return token;
  }

  if(ch == '"') {
    ch = fgetc(fp);
    printf("StringConst(\"");
    fprintf(fwp, "StringConst(\"");
    while(ch != '"') {
      if(ch == EOF){
        printf("String must be terminated before file ends\n");
        exit(1);
        }
      printf("%c", ch);
      fprintf(fwp, "%c", ch);
      ch = fgetc(fp);
    }
    printf("\")\n");
    fprintf(fwp, "\")\n");
    token.type = STRING_CONST;
    return token;
  }
}



int main (int argc, char *argv[]) {
  char* file = "code.psi";
  if(argc > 1) {
    file = argv[1];
  }
  
  FILE *fp = fopen(file, "r");

  if(fp == NULL) {
    printf("Cannot open file: %s\n", argv[1]);
    return 1;
  }

  FILE * fwp = fopen("code.lex", "w");

  Token token;
  char c = fgetc(fp);
  while (c != EOF){
    ungetc(c, fp);
    token = getNextToken(fp, fwp);
    switch (token.type) {
      case IDENTIFIER:
        printf("Identifier(%s)\n", token.lexeme);
        fprintf(fwp, "Identifier(%s)\n", token.lexeme);
        break;
      case INT_CONST:
        printf("IntConst(%s)\n", token.lexeme);
        fprintf(fwp, "IntConst(%s)\n", token.lexeme);
        break;
      case OPERATOR:
        printf("Operator(%s)\n", token.lexeme);
        fprintf(fwp, "Operator(%s)\n", token.lexeme);
        break;
      case LEFT_PAR:
        printf("LeftPar\n");
        fprintf(fwp, "LeftPar\n");
        break;
      case RIGHT_PAR:
        printf("RightPar\n");
        fprintf(fwp, "RightPar\n");
        break;
      case LEFT_SQUARE_BRACKET:
        printf("LeftSquareBracket\n");
        fprintf(fwp, "LeftSquareBracket\n");
        break;
      case RIGHT_SQUARE_BRACKET:
        printf("RightSquareBracket\n");
        fprintf(fwp, "RightSquareBracket\n");
        break;
      case LEFT_CURLY_BRACKET:
        printf("LeftCurlyBracket\n");
        fprintf(fwp, "LeftCurlyBracket\n");
        break;
      case RIGHT_CURLY_BRACKET:
        printf("RightCurlyBracket\n");
        fprintf(fwp, "RightCurlyBracket\n");
        break;
      case STRING_CONST:
        break;
      case KEYWORD:
        printf("Keyword(%s)\n", token.lexeme);
        fprintf(fwp, "Keyword(%s)\n", token.lexeme);
        break;
      case ENDOFLINE:
        printf("EndOfLine\n");
        fprintf(fwp, "EndOfLine\n");
        break;
    }
    token.type = NO_TYPE;
    c = fgetc(fp);
  }
  fclose(fwp);
  fclose(fp);
  printf("Lexical analysis completed!\n");
  return 0;
}