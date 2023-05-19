#include <string.h>
#include "stdio.h"
#include <ctype.h>
#include <stdlib.h>

#define MAX_IDENT_LEN 30
#define MAX_INT_LEN 10

FILE* fp;
FILE* fwp;

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

int isOperator (char ch) {
  const char* operators[] = {"+", "-", "*", ":", "="};
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

void raiseError(char* message) {
  printf("Lexical ERR!  %s\n", message);
  fprintf(fwp, "Lexical ERR! %s\n", message);
  printf("Detailed error information can be found in the output file code.lex\n");
  exit(1);
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

Token getNextToken(){
  Token token;

  // Skip white spaces
  char ch = fgetc(fp);
  while (isspace(ch)) {
    ch = fgetc(fp);
  }

  // Check EOF
  if(ch == EOF){
    token.type = NO_TYPE;
    return token;
  }

  // Skip comments
  if (ch == '/') {
    char c = fgetc(fp);
    char nextc;
    if (c == '*') {
      do {
        if (nextc == EOF) {
          raiseError("Comment cannot terminated!");
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
        //TODO:
        raiseError("Identifiers must be smaller or equal than 30 characters!");
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
  int operatorStatus = isOperator(ch);
  if (operatorStatus != 0) {
    if(operatorStatus == 1){
      if (ch == ':') {
        raiseError("Invalid operator, assignment operator must be used like ':='");
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
        //TODO:
        raiseError("Integers must be smaller or equal than 10 digits!");
      }
      ch = fgetc(fp);
    }
    if (isalpha(ch) || ch == '_') {
      raiseError("Identifiers can't start with numbers!");
    }
    ungetc(ch,fp);
    token.lexeme[j] = '\0';
    token.type = INT_CONST;
    return token;
  }

  if(ch == '"') {
    ch = fgetc(fp);
    fprintf(fwp, "StringConst(\"");
    while(ch != '"') {
      if(ch == EOF){
        raiseError("String must be terminated before file ends!");
        }
      fprintf(fwp, "%c", ch);
      ch = fgetc(fp);
    }
    fprintf(fwp, "\")\n");
    token.type = STRING_CONST;
    return token;
  }

  token.type = NO_TYPE;
  return token;
}



int main (int argc, char *argv[]) {
  char* file = "code.psi";
  if(argc > 1) {
    file = argv[1];
  }
  
  fp = fopen(file, "r");

  if(fp == NULL) {
    printf("Cannot open file: %s\n", argv[1]);
    return 1;
  }

  fwp = fopen("code.lex", "w");

  Token token;
  char c = fgetc(fp);
  while (c != EOF){
    ungetc(c, fp);
    token = getNextToken();
    switch (token.type) {
      case IDENTIFIER:
        fprintf(fwp, "Identifier(%s)\n", token.lexeme);
        break;
      case INT_CONST:
        fprintf(fwp, "IntConst(%s)\n", token.lexeme);
        break;
      case OPERATOR:
        fprintf(fwp, "Operator(%s)\n", token.lexeme);
        break;
      case LEFT_PAR:
        fprintf(fwp, "LeftPar\n");
        break;
      case RIGHT_PAR:
        fprintf(fwp, "RightPar\n");
        break;
      case LEFT_SQUARE_BRACKET:
        fprintf(fwp, "LeftSquareBracket\n");
        break;
      case RIGHT_SQUARE_BRACKET:
        fprintf(fwp, "RightSquareBracket\n");
        break;
      case LEFT_CURLY_BRACKET:
        fprintf(fwp, "LeftCurlyBracket\n");
        break;
      case RIGHT_CURLY_BRACKET:
        fprintf(fwp, "RightCurlyBracket\n");
        break;
      case STRING_CONST:
        break;
      case KEYWORD:
        fprintf(fwp, "Keyword(%s)\n", token.lexeme);
        break;
      case ENDOFLINE:
        fprintf(fwp, "EndOfLine\n");
        break;
    }
    token.type = NO_TYPE;
    c = fgetc(fp);
  }
  fclose(fwp);
  fclose(fp);
  printf("Lexical analysis completed! output: code.lex\n");
  return 0;
}