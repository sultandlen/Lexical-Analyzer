#include <string.h>
#include "stdio.h"
#include <ctype.h>

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
    ENDOFLINE
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[32];
} Token;

int isOperator (char ch, FILE *fp) {
  const char* operators[] = {"+", "-", "*", "/", ":"};
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

Token getNextToken(FILE* fp){
  Token token;

  //skip white spaces
  char ch = fgetc(fp);
  while (isspace(ch)) {
    ch = fgetc(fp);
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
      ch = fgetc(fp);
    }
    ungetc(ch, fp);
    token.lexeme[j] = '\0'; //null terminator, marks the end of a string
    token.type = IDENTIFIER;
    return token;
  }



  // Check for operators
  int operatorStatus = isOperator(ch, fp);
  if (operatorStatus != 0) {
    if(operatorStatus == 1){
      strcpy(token.lexeme, &ch);
    } else if(operatorStatus == 2) {
      token.lexeme[0] = ch;
      token.lexeme[1] = fgetc(fp);
    }
    token.type = OPERATOR;
    return token;
  }

  // Check for integer constants
  if (isdigit(ch)) {
    int j = 0;
    while (isdigit(ch)) {
      token.lexeme[j++] = ch;
      ch = fgetc(fp);
    }
    ungetc(ch,fp);
    token.lexeme[j] = '\0';
    token.type = INT_CONST;
    return token;
  }
}



int main (int argc, char *argv[]) {
  if(argc != 2) {
    printf("Please specify a file: %s <filename>\n",argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "r");

  if(fp == NULL) {
    printf("Cannot open file: %s\n", argv[1]);
    return 1;
  }

  Token token;
  char c = fgetc(fp);
  while (c != EOF){
    ungetc(c, fp);
    token = getNextToken(fp);
    switch (token.type) {
      case IDENTIFIER:
        printf("Identifier(%s)\n", token.lexeme);
        break;
      case INT_CONST:
        printf("IntConst(%s)\n", token.lexeme);
        break;
      case OPERATOR:
        printf("Operator(%s)\n", token.lexeme);
        break;
      case LEFT_PAR:
        printf("LeftPar\n");
        break;
      case RIGHT_PAR:
        printf("RightPar\n");
        break;
      case LEFT_SQUARE_BRACKET:
        printf("LeftSquareBracket\n");
        break;
      case RIGHT_SQUARE_BRACKET:
        printf("RightSquareBracket\n");
        break;
      case LEFT_CURLY_BRACKET:
        printf("LeftCurlyBracket\n");
        break;
      case RIGHT_CURLY_BRACKET:
        printf("RightCurlyBracket\n");
        break;
      case STRING_CONST:
        printf("StringConst(\"%s\")\n", token.lexeme);
        break;
      case KEYWORD:
        printf("Keyword(%s)\n", token.lexeme);
        break;
      case ENDOFLINE:
        printf("EndOfLine\n");
        break;
    }
    c = fgetc(fp);
  }
  return 0;
}