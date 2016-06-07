#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INPUT_LEN	256
#define MAX_LEN	64

char *p;
int invalid;
int stop = 0;

typedef enum TYPE {
	INT,
	CONS,
	SYMBOL,
} Type;

typedef struct OBJECT {
	Type type;
	struct OBJECT *car;
	struct OBJECT *cdr;
} Object;

Object *new_Object(Type type, Object *car, Object *cdr)
{
	Object *t = (Object *) malloc(sizeof(Object));
	t->type = type;
	t->car = car;
	t->cdr = cdr;
	return t;
}

// function pointer
typedef Object * (*fptr)(Object *);

int count = 0;
Object *symbol_list[100];
Object *ReadMore();

int isNumber(char c)
{
	return (c >= '0' && c <= '9') ? 1 : 0;
}

int isLetter(char c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) ? 1 : 0;
}

void add_symbol(Object *a, Object *b)
{
	symbol_list[count++] = new_Object(SYMBOL, a, b);
}

Object *find_symbol(Object *a)
{
	int i;
	for (i = 0; i < count; i++) {
		if (strcmp((char *) symbol_list[i]->car, (char *) a->car) == 0) {
			return symbol_list[i]->cdr;
		}
	}
	return NULL;
}

char *left;
char buf[MAX_LEN];

char *next_token()
{
	if (left != NULL) {
		char *tmp = left;
		left = NULL;
		return tmp;
	}
	int i = 0;
	memset(buf, 0, MAX_LEN);
	// find the first non-space character
	do {
		if (*p == '\n') {
			return NULL;
		}
	} while (*p++ == ' ');

	//get back to the first character
	p--;
	buf[i++] = *p++;
	if (buf[i - 1] == '(' || buf[i - 1] == ')') {
		return strdup(buf);
	}
	while (isNumber(*p) || isLetter(*p)) {
		buf[i++] = *p++;
	}
	return strdup(buf);
}

Object *Read()
{
	char *t = next_token();
	if (strcmp(t, "(") == 0) {
		// read a list
		return ReadMore();
	}

	int i;
	for (i = 0; i < strlen(t); i++) {
		if (!isNumber(t[i])) {
			break;
		}
	}
	if (i == strlen(t)) {
		// read a number
		int n = atoi(t);
		return new_Object(INT, (Object *) n, NULL);
	}

	//it's a symbol
	return new_Object(SYMBOL, (Object *) t, NULL);
}

Object *ReadMore() {
	char *t = next_token();
	if (strcmp(t, ")") == 0) {
		return NULL;
	}
	left = t;
	return new_Object(CONS, Read(), ReadMore());
}

Object *Evaluate(Object *a);

Object *apply(Object *a, Object *b)
{
	return (*(fptr)a)(b);
}

Object *list(Object *a)
{
	if (a == NULL) {
		return NULL;
	}
	return new_Object(CONS, Evaluate(a->car), list(a->cdr));
}

Object *Evaluate(Object *a)
{
	if (a == NULL) {
		return NULL;
	}

	switch (a->type) {
		case INT:
			return a;
		case CONS:
			return apply(Evaluate(a->car), list(a->cdr));
		case SYMBOL:
			return find_symbol(a);
	}
	
	return a;
}

Object *op_add(Object *a)
{
	int ans = 0;
	while (a != NULL) {
		ans += (int) a->car->car;
		a = a->cdr;
	}
	return new_Object(INT, (Object *) ans, NULL);
}

Object *op_sub(Object *a)
{
	if (a->cdr == NULL) {
		return new_Object(INT, (Object *) (-((int) a->car->car)), NULL);
	}
	int ans = (int) a->car->car;
	a = a->cdr;
	while (a != NULL) {
		ans -= (int) a->car->car;
		a = a->cdr;
	}
	return new_Object(INT, (Object *) ans, NULL);
}

Object *op_mul(Object *a)
{
	int ans = 1;
	while (a != NULL) {
		ans *= (int) a->car->car;
		a = a->cdr;
	}
	return new_Object(INT, (Object *) ans, NULL);
}


Object *op_div(Object *a)
{
	if (a->cdr == NULL) {
		return new_Object(INT, (Object *) (1 / ((int) a->car->car)), NULL);
	}
	int ans = (int) a->car->car;
	a = a->cdr;
	while (a != NULL) {
		ans /= (int) a->car->car;
		a = a->cdr;
	}
	return new_Object(INT, (Object *) ans, NULL);
}

void quit()
{
	stop = 1;
}

void Print(Object *a) {
	if (a == NULL) {
		printf("NIL");
		return;
	}
	if (a->type == INT) {
		printf("%d", (int) a->car);
		return;
	}
	if (a->type == CONS) {
		printf("(");
		Print(a->car);
		printf(" ");
		Print(a->cdr);
		printf(")");
		return;
	}
	if (a->type == SYMBOL) {
		printf("%s", (char *) a->car);
		return;
	}
}
	
int main()
{
	time_t current_time = time(NULL);
	printf("XKCL (Xiankai Common Lisp) Version 1.0 %s",ctime(&current_time));
	add_symbol((Object *) "+", (Object *) op_add);
	add_symbol((Object *) "-", (Object *) op_sub);
	add_symbol((Object *) "*", (Object *) op_mul);
	add_symbol((Object *) "/", (Object *) op_div);
	add_symbol((Object *) "quit", (Object *) quit);
	while (!stop) {
		printf("\n>");
		char buffer[INPUT_LEN] = {0};
		fgets(buffer, sizeof(buffer), stdin);
		p = buffer;

		// first Read the string into cons data structure
		invalid = 0;
		Object *a = Read();

		// then Evaluate data structure
		Object *b = Evaluate(a);

		// print out the value of this data structure or specific value
		Print(b);
	}
	return 0;
}
