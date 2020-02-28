#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <assert.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DECINT, TK_HEXINT, TK_PLUS, TK_MINUS, 
  TK_TIMES, TK_DIV, TK_LPAREN, TK_RPAREN, TK_REG, TK_NEQ, TK_LOGAND,
  TK_LOGOR, TK_LOGNOT, TK_NEG, TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\$[a-z]{2,3}", TK_REG},         // register
  {"0x[0-9a-fA-F]+", TK_HEXINT},     // hexadecimal integer
  {"0|[1-9][0-9]*", TK_DECINT},      // decimal integer
  {"==", TK_EQ},                     // equal
  {"!=", TK_NEQ},                    // not equal
  {"&&", TK_LOGAND},                 // logical and
  {"\\|\\|", TK_LOGOR},              // logical or
  {"!", TK_LOGNOT},                  // not
  {"\\+", TK_PLUS},                  // plus
  {"\\-", TK_MINUS},                 // minus
  {"\\*", TK_TIMES},                 // times
  {"\\/", TK_DIV},                   // division
  {"\\(", TK_LPAREN},                // left parenthesis
  {"\\)", TK_RPAREN},                // right parenthesis
  {" +", TK_NOTYPE}                  // spaces
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        if (rules[i].token_type != TK_NOTYPE) {
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int check_parentheses(int p, int q) {
	assert(p < q);
	int estack[32] = {0};
	int i, etop = -1;
	int lastmatchlp = -1, lastmatchrp = -1;
	for (i = p; i <= q; i++) {
		if (tokens[i].type == TK_LPAREN) {
			estack[++etop] = i;
		}
		if (tokens[i].type == TK_RPAREN) {
			if (etop == -1) {
				return -2; // bad expression, the rightmost ')' is not matched
			}
			lastmatchlp = estack[etop--];
			lastmatchrp = i;
		}
	}
	if (etop != -1) {
		return -2; // bad expression, the leftmost '(' is not matched
	}
	if (lastmatchlp != p || lastmatchrp != q) {
		if (tokens[p].type == TK_LPAREN && tokens[q].type == TK_LPAREN) {
			return -3; // the leftmost '(' and the rightmost ')' are not matched
		}
		return -1; // the whole expression is not surrounded by a matched pair of parentheses
	}
	return 1;
}

static bool is_op(int i) {
	if (i != TK_DECINT && i != TK_HEXINT && i != TK_REG && i != TK_NOTYPE) 
		return true;
	return false;
}

static int dominant_operator(int p, int q) {
	int estack[32] = {0};
	int i, etop = -1;
	for (i = p; i <= q; i++) {
		if (!is_op(tokens[i].type)) continue;
		if (etop == -1) estack[++etop] = i;
		else if (tokens[estack[etop]].type == TK_LPAREN) estack[++etop] = i;
		else if (tokens[i].type == TK_LPAREN) estack[++etop] = i;
		else if (tokens[i].type == TK_RPAREN) {
			while (tokens[estack[etop]].type != TK_LPAREN) etop--;
			etop--;
		}
		else if (tokens[i].type == TK_LOGOR) estack[etop] = i;
		else if (tokens[i].type == TK_LOGAND) {
			if (tokens[estack[etop]].type == TK_LOGOR) estack[++etop] = i;
			else estack[etop] = i;
		}
		else if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) {
			if (tokens[estack[etop]].type == TK_LOGAND || tokens[estack[etop]].type == TK_LOGOR)
				estack[++etop] = i;
			else estack[etop] = i;
		}
		else if (tokens[i].type == TK_PLUS || tokens[i].type == TK_MINUS) {
			if (tokens[estack[etop]].type == TK_LOGAND || tokens[estack[etop]].type == TK_LOGOR
				|| tokens[estack[etop]].type == TK_EQ || tokens[estack[etop]].type == TK_NEQ)
				estack[++etop] = i;
			else estack[etop] = i;
		}
		else if (tokens[i].type == TK_TIMES || tokens[i].type == TK_DIV) {
			if (tokens[estack[etop]].type == TK_TIMES || tokens[estack[etop]].type == TK_DIV
				|| tokens[estack[etop]].type == TK_LOGNOT || tokens[estack[etop]].type == TK_DEREF
				|| tokens[estack[etop]].type == TK_NEG)
				estack[etop] = i;
			else estack[++etop] = i;
		}
		else if (tokens[i].type == TK_DEREF || tokens[i].type == TK_NEG || tokens[i].type == TK_LOGNOT) {
			estack[++etop] = i;
		}
	}
	return estack[0];
}

static uint32_t eval(int p, int q) {
	if (p > q) {
		return -1;
	}
	if (p == q) {
		uint32_t n = -1;
		if (tokens[p].type == TK_DECINT) {
			sscanf(tokens[p].str, "%u", &n);
		}
		else if (tokens[p].type == TK_HEXINT) {
			sscanf(tokens[p].str, "%x", &n);
		}
		else if (tokens[p].type == TK_REG) {
			uint32_t i;
			char *str = tokens[p].str + 1;
    		for (i = 0; i < 8; i++) {
    			if (strcmp(reg_name(i, 4), str) == 0) {
    				n = reg_l(i);
    				return n;
    			}
    		}
    		if (strcmp("eip", str) == 0) {
    			n = cpu.eip;
    			return n;
    		}
		}
		return n;
	}
	else if (check_parentheses(p, q) == 1) {
		return eval(p - 1, q - 1);
	}
	else {
		int op = dominant_operator(p, q);
		printf("%s\n", tokens[op].str);
		uint32_t val1 = eval(p, op - 1);
		uint32_t val2 = eval(op + 1, q);
		switch (tokens[op].type) {
		  case TK_PLUS: return val1 + val2;
		  case TK_MINUS: return val1 - val2;
		  case TK_TIMES: return val1 * val2;
		  case TK_DIV: return val1 / val2;
		  case TK_LOGAND: return val1 && val2;
		  case TK_LOGOR: return val1 || val2;
		  case TK_EQ: return val1 == val2;
		  case TK_NEQ: return val1 != val2;
		  case TK_NEG: return -val2;
		  case TK_DEREF: return vaddr_read(val2, 4);
		  case TK_LOGNOT: return !val2;
		  default: assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  if (check_parentheses(0, nr_token - 1) == -2) {
  	*success = false;
  	return 0;
  }
  uint32_t i;
  for (i = 0; i < nr_token; i++) {
  	if (tokens[i].type == TK_MINUS && (i == 0 || is_op(tokens[i - 1].type))) {
  		tokens[i].type = TK_NEG;
  		continue;
  	}
  	if (tokens[i].type == TK_TIMES && (i == 0 || is_op(tokens[i - 1].type))) {
  		tokens[i].type = TK_DEREF;
  		continue;
  	}
  }
  *success = true;
  return eval(0, nr_token - 1);
}
