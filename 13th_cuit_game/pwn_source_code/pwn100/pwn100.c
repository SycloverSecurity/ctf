#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void init_work(void) {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
}

int welcome(void) {
  // welcome info
  FILE *fp = fopen("./info.txt", "r");
  if (!fp) {
    printf("open error,plz contect us!\n");
    return -1;
  }
  char *tmp = (char *)malloc(0x100 * sizeof(char));
  while (fgets(tmp, 0x100, fp) != NULL) {
    printf("%s", tmp);
  }
  printf("\n\t\t\t\t\t\t\t\tI Will make Lemon 2 Lemon water for you!\n");
  fclose(fp);
}

int make_lemon_2_lemonwater(void) {
  char buffer[100] = {0};
  write(1, "[*]Give me your lemon:", strlen("[*]Give me your lemon:"));
  read(0, buffer, 100);
  write(1, "\t\t\tProccessing...\n", strlen("\t\t\tProccessing...\n"));
  write(1, "[*]Ok...Here you are : ", strlen("[*]Ok...Here you are : "));
  printf(buffer);
}

int main(void) {

  init_work();
  welcome();
  while (1) {
    make_lemon_2_lemonwater();
  }
  return 0;
}