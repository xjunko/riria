#include <string.h>

char* strcpy(char* strDest, const char* strSrc) {
  char* temp = strDest;
  while ((*strDest++ = *strSrc++) != '\0')
    ;
  return temp;
}