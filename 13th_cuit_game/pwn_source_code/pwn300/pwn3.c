#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_NEW 0x100
#define CMD_EDIT 0x101
#define CMD_DELETE 0x102
#define CMD_SHOW 0x103
#define CMD_EXIT 0x104
#define CMD_MARK 0x105
#define CMD_SHOW_MARK 0x106
#define CMD_DEL_MARK 0x107
#define CMD_EDIT_MARK 0x108

#define CMD_LEN 0x10
#define CONTENT_LEN 0x100

void init_work(void) {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  alarm(60);
}

// 48bytes
struct NOTE {
  int index;
  int size;
  char name[32];
  char *content;
};

// 24bytes
struct MARK {
  int index; // index of markdown_list
  int id;    // note id
  char *info;
  // func ptrs;
  int (*show_info)(char *);
};

int note_count = 0;
int mark_count = 0;
int flag = 2;
struct NOTE *note_list[10]; // 10 notes max.
struct MARK *mark_list[10]; // 10 markdown max

int show_info(char *content) {
  // show
  puts(content);
}

void welcome(void) {
  printf("\t=== Welcome to My NOTEPAD ===\n");
  printf("\t\t>It's not a safe NOTEPAD...Is't it?\n");
  printf("\t\t>Try to exploit it!\n");
}
int saferead(char *buffer, int count, char spl) {
  int i = 0;
  char ch;
  while (i < count) {
    ch = getchar();
    if (ch == spl) {
      ch = '\0';
      buffer[i] = ch;
      break;
    }
    buffer[i] = ch;
    i++;
  }
  return i;
}
void mysplit(char *str) {
  int i;
  for (i = 0; i < strlen(str); i++) {
    if (str[i] == ' ' || str[i] == '\n') {
      str[i] = '\0';
      break;
    }
  }
}

int getcmd(char *cmd) {
  int index = 0;
  char cmdlist[9][16] = {"new",  "edit",      "delete",      "show",     "exit",
                         "mark", "show_mark", "delete_mark", "edit_mark"};
  for (index = 0; index < 9; index++) {
    if (!strcmp(cmd, cmdlist[index])) {
      return index + 0x100;
    }
  }
  return -1;
}

int readint() {
  char buf[32] = {0};
  read(0, buf, 32);
  return atoi(buf);
}
int new (void) {
  int i;
  int size;
  char *ptr;
  if (note_count < 10) {
    struct NOTE *note = (struct NOTE *)malloc(sizeof(struct NOTE));
    write(1, "$ note size:", strlen("$ note size:"));
    size = readint();
    if (size > 4096)
      size = 4096;
    note->size = size;
    ptr = (char *)malloc(size);
    note->content = ptr;
    note->index = note_count;
    write(1, "$ note name:", strlen("$ note name:"));
    saferead(note->name, 32, '\n');
    write(1, "$ note content:", strlen("$ note content:"));
    saferead(note->content, note->size, '\n');
    for (i = 0; i < 10; i++) {
      if (note_list[i] == NULL) {
        note_list[i] = note;
        break;
      }
    }
    note_count++;
  } else {
    write(1, ":( too much note...\n", strlen(":( too much note...\n"));
    return 0;
  }
}
int edit(void) {
  int index;
  write(1, "$ note index:", strlen("$ note index:"));
  index = readint();
  while (index < 0 || index > 10) {
    write(1, "Error index!\n", strlen("Error index!\n"));
    index = readint();
  }
  // edit name && content
  write(1, "$ note name:", strlen("$ note name:"));
  saferead(((struct NOTE *)(note_list[index]))->name, 32, '\n');
  write(1, "$ note content:", strlen("$ note content:"));
  saferead(((struct NOTE *)(note_list[index]))->content,
           ((struct NOTE *)(note_list[index]))->size, '\n');
}
int delete (void) {
  int index;
  write(1, "$ note index:", strlen("$ note index:"));
  index = readint();
  while (index < 0 || index > 10) {
    write(1, "Error index!\n", strlen("Error index!\n"));
    index = readint();
  }
  if (note_list[index] != NULL) {
    // free note
    free(note_list[index]);
    note_list[index] = NULL;
    note_count--;
  }
}
int show(void) {
  int index;
  write(1, "$ note index:", strlen("$ note index:"));
  index = readint();
  while (index < 0 || index > 10) {
    write(1, "Error index!\n", strlen("Error index!\n"));
    index = readint();
  }
  if (note_list[index] != NULL) {
    printf("\tid:%d\n", ((struct NOTE *)(note_list[index]))->index);
    printf("\tsize:%d\n", ((struct NOTE *)(note_list[index]))->size);
    printf("\tname:%s\n", ((struct NOTE *)(note_list[index]))->name);
    // for info leak
    printf("\tcontent:");
    write(1, ((struct NOTE *)(note_list[index]))->content,
          ((struct NOTE *)(note_list[index]))->size);
    write(1, "\n", 1);
    // printf("\tcontent:%s\n", ((struct NOTE *)(note_list[index]))->content);
  } else {
    write(1, "Emmu...empty note.\n", strlen("Emmu...empty note.\n"));
  }
}

int mark(void) {
  int id;
  char *ptr;
  if (mark_count < 10) {
    struct MARK *mark = (struct MARK *)malloc(sizeof(struct MARK));
    write(1, "$ index of note you want to mark:",
          strlen("$ index of note you want to mark:"));
    id = readint();
    while (id < 0 || id > 10) {
      write(1, "Error index!\n", strlen("Error index!\n"));
      write(1, "$ index of note you want to mark:",
            strlen("$ index of note you want to mark:"));
      id = readint();
    }
    if (note_list[id] == NULL) {
      write(1, "Emmu...empty note.\n", strlen("Emmu...empty note.\n"));
      return;
    }
    mark->id = id;
    mark->index = mark_count;
    write(1, "$ mark info:", strlen("$ mark info:"));
    ptr = (char *)malloc(32);
    saferead(ptr, 32, '\n');
    mark->info = ptr;
    mark->show_info = show_info;
    mark_list[mark_count] = mark;
    mark_count++;
  } else {
    write(1, "Emmu...too much mark!\n", strlen("Emmu...too much mark!\n"));
  }
}

int show_mark(void) {
  int index;
  write(1, "$ mark index:", strlen("$ mark index:"));
  index = readint();
  while (index < 0 || index > 10) {
    write(1, "Error index!\n", strlen("Error index!\n"));
    index = readint();
  }
  // call func without check
  mark_list[index]->show_info(mark_list[index]->info);
  /*
  if (mark_list[index] != NULL) {
    mark_list[index]->show_info(mark_list[index]->info);
  } else {
    write(1, "Emmu...Empty\n", strlen("Emmu...Empty\n"));
    return;
  }
  */
}
int delete_mark(void) {
  int index;
  if (flag > 0) {
    write(1, "$ mark index:", strlen("$ mark index:"));
    index = readint();
    while (index < 0 || index > 10) {
      write(1, "Error index!\n", strlen("Error index!\n"));
      index = readint();
    }
    if (mark_list[index] != NULL) {
      // free note
      free(mark_list[index]);
      mark_count--;
      flag--;
    } else {
      write(1, "exo me?\n", strlen("exo me?\n"));
    }
  } else {
    write(1, "just 2 times!\n", strlen("just 2 times!\n"));
    return;
  }
}

// heap overflow
int edit_mark(void) {
  int index;
  write(1, "$ mark index:", strlen("$ mark index:"));
  index = readint();
  while (index < 0 || index > 10) {
    write(1, "Error index!\n", strlen("Error index!\n"));
    index = readint();
  }
  if (note_list[index] == NULL) {
    write(1, "exo me?\n", strlen("exo me?\n"));
    return;
  }
  write(1, "$ mark content:", strlen("$ mark content:"));
  // overflow
  saferead(mark_list[index]->info, 64, '\n');
}

int help(void) {
  write(1, "CMD List:\n", strlen("CMD List:\n"));
  write(1, "\tadd(add your note)\n", strlen("\tadd(add your note)\n"));
  write(1, "\tedit(edit your note)\n", strlen("\tedit(edit your note)\n"));
  write(1, "\tdelete(delete your note)\n",
        strlen("\tdelete(delete your note)\n"));
  write(1, "\tshow(print your note)\n", strlen("\tshow(print your note)\n"));
  write(1, "\tmark(mark your note)\n", strlen("\tmark(mark your note)\n"));
  write(1, "\tshow_mark(show your mark info)\n",
        strlen("\tshow_mark(show your mark info)\n"));
  write(1, "\tdelete_mark(delete your mark)\n",
        strlen("\tdelete_mark(delete your mark)\n"));
  write(1, "\tedit_mark(edit your mark)\n",
        strlen("\tedit_mark(edit your mark)\n"));
}

int main(int argc, char *argv[]) {
  init_work();
  welcome();
  /*
  printf("%d\n",sizeof(struct NOTE));
  printf("%d\n",sizeof(struct MARK));
  */
  char cmd[CMD_LEN] = {0};
  while (1) {
    write(1, "$ ", strlen("$ "));
    saferead(cmd, CMD_LEN, '\n');
    mysplit(cmd);
    switch (getcmd(cmd)) {
    case CMD_NEW:
      new ();
      break;
    case CMD_EDIT:
      edit();
      break;
    case CMD_DELETE:
      delete ();
      break;
    case CMD_SHOW:
      show();
      break;
    case CMD_MARK:
      mark();
      break;
    case CMD_SHOW_MARK:
      show_mark();
      break;
    case CMD_DEL_MARK:
      delete_mark();
      break;
    case CMD_EDIT_MARK:
      edit_mark();
      break;
    case CMD_EXIT:
      exit(1);
    default:
      help();
      break;
    }
    memset(cmd, 0, CMD_LEN);
  }
  return 0;
}
