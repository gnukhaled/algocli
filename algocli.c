
#include "algocli-defs.h"
#include "cmds.h"
#include "registry.h"
#include "auth.h"

command_t *ccmd = (command_t *)NULL;
command_t *command = (command_t *)NULL;

char *format_doc(int spcount, const char *text){
  if (spcount < 0) spcount = 0;
  if (!text) text = "";
  size_t textlen = strlen(text);
  size_t doclen  = (size_t)spcount + textlen;
  char *t = calloc(doclen + 1, sizeof(char));
  if (!t) {
      fprintf(stderr, "format_doc: out of memory\n");
      return NULL;
  }
  for (int i = 0; i < spcount; i++)
      t[i] = ' ';
  memcpy(t + spcount, text, textlen);
  t[doclen] = '\0';
  return t;
}

void cmd_ref(void){
  int j = 0;
  int length = 0;
  int longest = 0;
  int reqspc = 0;
  char *line = rl_line_buffer;
  command_t *cmd;
  char *line_copy;

  printf("Possible commands for \"%s\" are:\n\n", line);

  line_copy = strdup(line);
  if (!line_copy)
      return;
  cmd = find_last_command(line_copy);
  if (!cmd || !cmd->subcmd) {
      free(line_copy);
      printf("\n");
      return;
  }

  while (cmd->subcmd[j].name != (char *)NULL){
      length = (int)strlen(cmd->subcmd[j].name);
      if (length > longest)
          longest = length;
      j++;
  }

  for (int i = 0; cmd->subcmd[i].name; i++){
      length = (int)strlen(cmd->subcmd[i].name);
      reqspc = (longest - length) + longest;
      char *doc = format_doc(reqspc, cmd->subcmd[i].doc);
      printf("\t%s %s%s\n", cmd->name, cmd->subcmd[i].name, doc ? doc : "");
      free(doc);
  }

  free(line_copy);
  printf("\n");
}

int algoauth(void){
  pam_handle_t *pamh = NULL;
  struct pam_conv pamc;
  int rc;
  int ok = FALSE;
  const char *user = getenv("USER");

  pamc.conv = &misc_conv;
  pamc.appdata_ptr = NULL;

  if (pam_start("algocli", user, &pamc, &pamh) != PAM_SUCCESS) {
      fprintf(stderr, "PAM initialization failed\n");
      return FALSE;
  }

  printf("\nPlease re-authenticate as user %s\n", user ? user : "(unknown)");

  rc = pam_authenticate(pamh, 0);
  if (rc != PAM_SUCCESS) {
      fprintf(stderr, "User verification failed!\n");
      ok = FALSE;
  } else {
      ok = TRUE;
  }

  pam_end(pamh, rc);
  return ok;
}

char *stripwhite(char *string){
  char *s = string;
  char *t;

  while (whitespace(*s))
      s++;

  if (*s == '\0')
      return s;

  t = s + strlen(s) - 1;
  while (t > s && whitespace(*t))
      t--;
  *++t = '\0';

  return s;
}

/* Maximum number of whitespace-separated tokens we accept on one
 * command line. The original code calloc'd 500 entries on every
 * keypress; in practice the deepest command is `accessctrl ssh set
 * listen-address <addr>` (5 tokens) and the longest argument list
 * is the SSH allowed-host add (1 verb + N hosts). 64 is comfortably
 * above any real input. */
#define MAX_TOKENS 64

int execute_command(char *line){
  char *cmdlist[MAX_TOKENS] = {0};
  char *params[MAX_TOKENS]  = {0};
  int counter = 0;
  int pcounter = 0;
  int index;
  char *tok;
  const char *commstr;

  /* Tokenize. strtok mutates `line` in place; cmdlist holds pointers
   * back into the same buffer. */
  tok = strtok(line, " ");
  while (tok != NULL) {
      if (counter >= MAX_TOKENS) {
          fprintf(stderr, "Too many arguments (limit %d)\n", MAX_TOKENS);
          return -1;
      }
      cmdlist[counter++] = tok;
      tok = strtok(NULL, " ");
  }
  if (counter == 0) return 0;   /* empty line — already filtered upstream */

  command = find_command(cmdlist[0]);

  /* `help <name>` short-circuit: pass just the requested command name. */
  if (command && counter > 1 && strcmp(command->name, "help") == 0) {
      params[0] = cmdlist[1];
      return command->func(params);
  }

  if (!command) {
      func_help(cmdlist);
      return -1;
  }

  /* Walk the command tree. Every token after a leaf becomes a
   * positional argument in params[]. */
  for (int i = 1; i < counter; i++) {
      index = 0;
      bool matched = false;
      while (command->subcmd != NULL &&
             (commstr = command->subcmd[index].name) != NULL) {
          if (strcmp(commstr, cmdlist[i]) == 0) {
              command = &command->subcmd[index];
              matched = true;
              break;
          }
          index++;
      }
      if (!matched && command->subcmd == NULL) {
          /* This token (and every later one) is a positional arg. */
          if (pcounter < MAX_TOKENS - 1) {
              params[pcounter++] = cmdlist[i];
          }
      }
  }

  /* Policy gate: if the resolved command demands authentication, run
   * PAM re-auth here so each func_* doesn't need to repeat the call.
   * Failure produces an audit entry and stops the dispatch. */
  if (command->flags & CMD_AUTH) {
      if (!algoauth()) {
          audit_log_authfail(command->name);
          return -1;
      }
      audit_log_authok(command->name);
  }

  return command->func(params);
}

command_t *find_command(const char *name){
  int i;

  for (i = 0; commands[i].name; i++)
    if (strcmp(name, commands[i].name) == 0)
      return &commands[i];

  return (command_t *)NULL;
}

command_t *find_last_command(char *line){
  char *buf[70] = {0};
  char *tok;
  int counter = 1;

  buf[0] = strtok(line, " ");
  if (!buf[0])
      return NULL;

  while (counter < 70 && (tok = strtok(NULL, " ")) != NULL) {
      buf[counter++] = tok;
  }

  command_t *cmd_ptr = find_command(buf[0]);
  if (!cmd_ptr)
      return NULL;

  for (int i = 1; i < counter; i++) {
      int index = 0;
      const char *commstr;
      while (cmd_ptr->subcmd != NULL &&
             (commstr = cmd_ptr->subcmd[index].name) != NULL) {
          if (strcmp(commstr, buf[i]) == 0) {
              cmd_ptr = &cmd_ptr->subcmd[index];
              break;
          }
          index++;
      }
  }

  return cmd_ptr;
}

void initialize_readline(const char *progname){
  /* readline's rl_readline_name is declared `char *` for backward
   * compatibility, but the API never modifies it. Allocate writable
   * storage so we don't have to cast away const. */
  static char readline_name[64];
  if (progname) {
      snprintf(readline_name, sizeof readline_name, "%s", progname);
      rl_readline_name = readline_name;
  }
  rl_attempted_completion_function = algocli_completion;
  rl_bind_key('\t', rl_complete);
}

char **algocli_completion(const char *text, int start, int end){
   char **matches = NULL;
   (void)end;

   /* Reset between sessions: ccmd is read by algoclisub_generator,
    * which only walks subcommands when ccmd is non-NULL. We must
    * always rewrite ccmd (or set it to NULL) before kicking off a
    * sub-completion, so a stale ccmd from a previous tab can't leak. */
   ccmd = NULL;

   if (start == 0) {
       matches = rl_completion_matches(text, algoclibase_generator);
   } else {
       char *buffer = strdup(rl_line_buffer);
       if (buffer) {
           ccmd = find_last_command(buffer);
           free(buffer);
       }
       matches = rl_completion_matches(text, algoclisub_generator);
   }
   rl_attempted_completion_over = 1;
   return matches;
}

char *algoclibase_generator(const char *text, int state){
  static size_t list_index;
  static size_t len;
  const char *name;

  if (!state) {
      list_index = 0;
      len = text ? strlen(text) : 0;
  }

  while ((name = commands[list_index].name) != NULL) {
      list_index++;
      if (strncmp(name, text ? text : "", len) == 0) {
          return strdup(name);
      }
  }
  return NULL;
}

char *algoclisub_generator(const char *text, int state){
  static size_t list_index;
  static size_t len;
  const char *name;

  if (!state) {
      list_index = 0;
      len = text ? strlen(text) : 0;
  }
  if (!ccmd || ccmd->subcmd == NULL) return NULL;

  while ((name = ccmd->subcmd[list_index].name) != NULL) {
      list_index++;
      if (strncmp(name, text ? text : "", len) == 0) {
          return strdup(name);
      }
  }
  return NULL;
}

int func_exit(char **arg){
  (void)arg;
  exit(0);
}

/* func_nfs_* moved to cmd-nfs.c (Phase 7.2). */

/* func_system_* moved to cmd-system.c (Phase 7.2). */
/* func_accessctrl moved to cmd-accessctrl.c (Phase 7.2). */

/* func_bp moved to cmd-bp.c (Phase 7.2). */

/* Stubs for commands whose underlying cmd-*.c files are still empty.
 * Phase 4 surfaces them honestly: print a clear message and return
 * -ENOSYS so callers/scripts can detect "not implemented". The
 * STUB_NOT_IMPLEMENTED macro lives in algocli.h so the cmd-*.c
 * modules can reuse it for their own per-leaf stubs. */

int func_syshealth(char **arg){ (void)arg; STUB_NOT_IMPLEMENTED("syshealth"); }
int func_config(char **arg)   { (void)arg; STUB_NOT_IMPLEMENTED("config"); }
int func_disk(char **arg)     { (void)arg; STUB_NOT_IMPLEMENTED("disk"); }
int func_repl(char **arg)     { (void)arg; STUB_NOT_IMPLEMENTED("repl"); }
int func_net(char **arg)      { (void)arg; STUB_NOT_IMPLEMENTED("net"); }
int func_cifs(char **arg)     { (void)arg; STUB_NOT_IMPLEMENTED("cifs"); }
int func_snapshot(char **arg) { (void)arg; STUB_NOT_IMPLEMENTED("snapshot"); }
int func_user(char **arg)     { (void)arg; STUB_NOT_IMPLEMENTED("user"); }
int func_vtl(char **arg)      { (void)arg; STUB_NOT_IMPLEMENTED("vtl"); }

/* func_fs / func_nfs / func_system moved to cmd-fs.c / cmd-nfs.c /
 * cmd-system.c (Phase 7.2). */

/* func_log* moved to cmd-log.c (Phase 7.2). */

/* func_bp_* moved to cmd-bp.c (Phase 7.2). */

/* func_fs* moved to cmd-fs.c (Phase 7.2). */

/* func_accessctrl_* moved to cmd-accessctrl.c (Phase 7.2). */

int func_help(char **arg){
  (void)arg;
  int i;
  int j = 0;
  int length = 0;
  int longest = 0;
  int reqspc = 0;
  int printed = 0;

  while (commands[j].name != (char *)NULL){
        length =  strlen(commands[j].name);
        if ( length > longest )
                longest = length;
        j++;
  }

  printf("\n");
  for (i = 0; commands[i].name; i++)
    {
      if (arg[0] == (char *)NULL
	  || (strcmp (arg[0], commands[i].name) == 0)){
	  length =  strlen(commands[i].name);
          reqspc = ((longest - length) + longest);
          char *doc = format_doc(reqspc, commands[i].doc);
          printf("%s%s.\n", commands[i].name, doc ? doc : "");
          free(doc);
          printed++;
        }
    }
  printf("\n");

  if (!printed){
      printf("No such command `%s'. Supported AlgoOS commands are:\n", arg[0]);
      for (i = 0; commands[i].name; i++)
        {
          if (printed == 1)
            {
              printed = 0;
              printf ("\n");
            }

          printf("%s\t", commands[i].name);
          printed++;
        }

      if (printed)
        printf("\n");
    }
  return(0);
}

