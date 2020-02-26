#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step one instruction exactly", cmd_si },
  { "info", "Display informations about the program being debugged", cmd_info },
  { "x", "Examine memory", cmd_x }

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  uint32_t n = 0;

  if (arg == NULL) {
    /* no argument given */
    cpu_exec(1); // 1 is the default
  }
  else {
    char *temp = strtok(NULL, " ");
    if (temp == NULL) {
      sscanf(arg, "%u", &n);
      cpu_exec(n); // Execute n times
    }
    else {
      printf("Syntax error: Too much arguments");
      return -1;
    }
  }
  return 0;
}

static int cmd_info(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if (arg == NULL) {
    /* no argument given, show rules */
    printf("\"info\" must be followed by the name of an info command.\n");
    printf("List of info subcommands:\n\n");
    printf("info r -- List of integer registers and their contents\n");
    printf("info w -- Status of specified watchpoints (all watchpoints if no argument)\n");
  }
  else if (strcmp(arg, "r")) {
    printf("eax\t0x%x\t%d\n", reg_l(R_EAX), reg_l(R_EAX));
    printf("ecx\t0x%x\t%d\n", reg_l(R_ECX), reg_l(R_ECX));
    printf("edx\t0x%x\t%d\n", reg_l(R_EDX), reg_l(R_EDX));
    printf("ebx\t0x%x\t%d\n", reg_l(R_EBX), reg_l(R_EBX));
    printf("esp\t0x%x\t%d\n", reg_l(R_ESP), reg_l(R_ESP));
    printf("ebp\t0x%x\t%d\n", reg_l(R_EBP), reg_l(R_EBP));
    printf("esi\t0x%x\t%d\n", reg_l(R_ESI), reg_l(R_ESI));
    printf("edi\t0x%x\t%d\n", reg_l(R_EDI), reg_l(R_EDI));
    printf("eip\t0x%x\t%d\n", cpu.eip, cpu.eip);
    printf("ax\t0x%x\t%d\n", reg_w(R_AX), reg_w(R_AX));
    printf("cx\t0x%x\t%d\n", reg_w(R_CX), reg_w(R_CX));
    printf("dx\t0x%x\t%d\n", reg_w(R_DX), reg_w(R_DX));
    printf("bx\t0x%x\t%d\n", reg_w(R_BX), reg_w(R_BX));
    printf("sp\t0x%x\t%d\n", reg_w(R_SP), reg_w(R_SP));
    printf("bp\t0x%x\t%d\n", reg_w(R_BP), reg_w(R_BP));
    printf("si\t0x%x\t%d\n", reg_w(R_SI), reg_w(R_SI));
    printf("di\t0x%x\t%d\n", reg_w(R_DI), reg_w(R_DI));
    printf("al\t0x%x\t%d\n", reg_b(R_AL), reg_b(R_AL));
    printf("cl\t0x%x\t%d\n", reg_b(R_CL), reg_b(R_CL));
    printf("dl\t0x%x\t%d\n", reg_b(R_DL), reg_b(R_DL));
    printf("bl\t0x%x\t%d\n", reg_b(R_BL), reg_b(R_BL));
    printf("ah\t0x%x\t%d\n", reg_b(R_AH), reg_b(R_AH));
    printf("ch\t0x%x\t%d\n", reg_b(R_CH), reg_b(R_CH));
    printf("dh\t0x%x\t%d\n", reg_b(R_DH), reg_b(R_DH));
    printf("bh\t0x%x\t%d\n", reg_b(R_BH), reg_b(R_BH));
  }
  else if (strcmp(arg, "w")) {
    printf("To be implemented...");
  }
  else {
  	printf("Undefined info command: \"%s\".  Try \"help info\".\n", arg);
  }
  return 0;
}

static int cmd_x(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  uint32_t n = 0;

  if (arg == NULL) {
    /* no argument given */
    printf("Argument required (starting display address).\n");
  }
  else {
    char *temp1 = strtok(NULL, " ");
    char *temp2 = strtok(NULL, " ");
    uint32_t start_addr = 0;
    uint32_t mem_data = 0;
    if (temp2 == NULL) {
      sscanf(arg, "%u", &n);
      sscanf(temp1, "%x", &start_addr);
      uint32_t i;
      for (i = 0; i < n ; i++) {
      	printf("0x%x:\t", start_addr);
      	mem_data = vaddr_read(start_addr, 4);
      	uint32_t d;
      	for (d = 0; d < 4; d++) {
      		printf("%02x ", (mem_data & 0x000000ff));
      		mem_data >>= 8;
      	}
      	printf("\n");
      	start_addr += 4;
      }
    }
    else {
      printf("Syntax error: Too much arguments");
      return -1;
    }
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
