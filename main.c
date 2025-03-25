/*
    This is a very small implementation of a lambda calculus interpreter.
*/

#include "ctype.h"
#include "stdlib.h"
#include "assert.h"
#include <stdio.h>
#include <string.h>

#define LAMBDA "λ"

struct lambda {

    union {
        struct {
            struct lambda* left; 
            struct lambda* right;
        }; // Application
        int tag; // variable tag
        struct {
            struct lambda* body;
            int arg;
        };
    };
    

    enum {
        VAR,
        APP,
        DEF
    } t;
};




int sp(char* lstr, int n, int r) {
    int d = 0;
    int i;
    if (r) i = n-1;
    else i = 0;
    do { 
        switch (lstr[i]) {
            case '(': d += 1; break;
            case ')': d -= 1; break;
        }
        if (r) i-=1;
        else i+=1;
    } while (d != 0 && i <= n);

    if (d != 0) return -1;

    if (r) return n-i-1;
    else return i-1;
}

int rsp(char* lstr, int n) {
    return sp(lstr,n,1);
}

int lsp(char* lstr, int n) {
    return sp(lstr,n,0);
}

int isl(char* lstr) {
    return !strncmp(lstr,LAMBDA,strlen(LAMBDA));
}

// first we need to parse the lambda string into a structured form
struct lambda* parse(char* lstr, int n) {

    printf("%.*s\n",n,lstr);

    int p = lsp(lstr,n);
    if (p + 1 == n && p != 0 ) {
        return parse(lstr+1,n-2);
    } 

    // 3 cases 
    // p == n -> (expr)
    // p < n  -> (expr) expr ou (expr) expr expr
    // p == 0 -> expr (?expr)
    struct lambda* ex = calloc(1, sizeof(struct lambda));

    if (isl(lstr)) {
        ex->t = DEF;
        ex->arg = *(lstr+strlen(LAMBDA));
        assert(*(lstr+strlen(LAMBDA)+1) == '.');
        ex->body = parse(lstr+strlen(LAMBDA)+2,n-strlen(LAMBDA)-2);
        goto return_free;
    }

    if (n == 1) {
        assert(isalnum(*lstr));
        ex->t = VAR;
        ex->tag = *(lstr);
        goto return_free;
    } 

    int rp = 0;
    printf("right\n");
    if (lstr[n-1] == ')') {
        rp = rsp(lstr,n);
        ex->right = parse((lstr+n)-rp,rp);
    } else {
        ex->right = parse(lstr+n-1,1);
        rp += 1;
    }
    printf("left\n");
    // left should be the (expr) + any other expressions before the last one
    ex->left = parse(lstr,n-rp);
    ex->t = APP;


return_free:
    return ex;
    
}

// build a lambda expression from a structured form
char* build(struct lambda* l) {
    int a = 1024;
    char* s = calloc(a, 1);
    if (l->t == APP) {
        char* sl = build(l->left);
        char* sr = build(l->right);
        if (strlen(sl) + strlen(sr) > a) 
            s = realloc(s, strlen(sl) + strlen(sr) + a);
        strcat(s, "(");
        strcat(s, sl);
        strcat(s, ")(");
        strcat(s, sr);
        strcat(s, ")");
        
        free(sl);
        free(sr);
    } else if (l->t == DEF) {
        char* b = build(l->body);
        if (strlen(b)> a) 
            s = realloc(s, strlen(b) + a);

        strcat(s, LAMBDA);
        strcat(s, (char*)&l->arg);
        strcat(s,".");
        strcat(s,b);
        free(b);
    } else {
        strcat(s,(char*)&l->tag);
    }


    return s;
}

// visualize the lambda expression as a tree
void visualize(struct lambda* l,int d){
    for (int i = 0; i < d; i++) printf(" ");
    switch (l->t) {
        case VAR:
            printf("VAR: '%c'\n",l->tag);
            break;
        case DEF:
            printf("DEF ('%c'):\n",l->arg);
            visualize(l->body, d+1);
            break;
        case APP:
            printf("APP: \n");
            visualize(l->left, d+1);
            visualize(l->right, d+1);
            break;
    }

}

int* tags(struct lambda* l) {
    char* str = build(l);
    int* t = calloc(26, sizeof(int));
    for (int i = 0; i < strlen(str); i++) {
        if (isalpha(str[i]) && str[i+1]== '.' ) t[str[i]-'a'] = 1;
    }
    return t;
}

// rename a variable in a lambda expression
int convert(struct lambda* a,struct lambda* b) {

    int* ta = tags(a);
    int* tb = tags(b);

    int* c = calloc(26, sizeof(int));
    int* d = calloc(26, sizeof(int));
    int s = 0;
    for (int i = 0; i < 26; i++) {
        int p = ta[i] & tb[i];
        if (p) s++;
        if (p) printf("'%c' == '%c'\n",i+'a',i+'a');
        c[i] = p;
        int o = ta[i] | tb[i];
        d[i] = o;
    }

    if (s == 0) return 0;

    // find s zeros in d
    int j = 0;
    int* z = calloc(26, sizeof(int));
    for (int i = 0; i < 26; i++) {
        if (d[i] == 0) {
            z[j] = i;
            j++;
        }
    }

    printf("s: %d\n",s);

    int k = 0;
    // replace all instances of a variable in a with the corresponding z
    for (int i = 0; i < 26; i++) {
        if (c[i]) {
            char* astr = build(a);
            // replace all instances of i in a with z[k]
            for (int j = 0; j < strlen(astr); j++) {
                if (astr[j] == i+'a') astr[j] = z[k]+'a';
            }
            struct lambda* na = parse(astr, strlen(astr));
            *a = *na;
            free(na);
            k++;
        }
    }

    free(c);
    free(d);
    free(z);
    free(ta);
    free(tb);
    return 0;
}


// replace all instances of a variable with a lambda expression
int replace(struct lambda* l, struct lambda* x, int d) {
    switch (l->t) {
        case VAR:
            //printf("'%c' == '%c'\n",l->tag,d);
            if (l->tag == d) *l = *x;
            return 0;
        case APP:
            replace(l->left, x,d);
            replace(l->right, x,d);
            return 1;
        case DEF:
            // TODO: handle collisions
            replace(l->body, x,d);
            return 1;
    }


}

// implement subsitution for a function in a lambda expression
struct lambda* subst(struct lambda* l, struct lambda* x) {
    if (l->t != DEF) return NULL;
    //printf("Subst %s into %s\n",build(x),build(l));
    convert(l,x);

    // variable to replace
    int r = l->arg;
    struct lambda* b = l->body;

    // replace all instances of r in x with b
    replace(b, x, r);

    return b;
}

// beta-reduce a lambda expression
int reduce(struct lambda* l) {
    if (l->t == DEF) return reduce(l->body);
    if (l->t == VAR)  return 0;
    //printf("%s\n",build(l));

    struct lambda* f = l->left;
    if (f->t == APP) reduce(f);

    struct lambda* x = l->right;
    if (x->t == APP) reduce(x);



    struct lambda* b = subst(f, x);
    if (b == NULL) return 0;
    //printf("%s\n",build(b));
    reduce(b);
    *l = *b;


    return 0;
}


#define ZERO "(λa.λb.b)"
#define ONE "(λa.λb.ab)"
#define TWO "(λa.λb.a(ab))"
#define THREE "(λa.λb.a(a(ab)))"
#define FOUR "(λa.λb.a(a(a(ab))))"
#define FIVE "(λa.λb.a(a(a(a(ab)))))"
#define SIX "(λa.λb.a(a(a(a(a(ab))))))"
#define SEVEN "(λa.λb.a(a(a(a(a(a(ab)))))))"
#define EIGHT "(λa.λb.a(a(a(a(a(a(a(ab))))))))"


#define SUCC "(λn.λg.λt.g(ngt))"
#define ADD "(λm.λn.λf.λx.mf(nfx))"



int main() {
    //char* expr = calloc(1024,1);
    //scanf("%s",expr);
    char* expr = ADD THREE THREE;
    printf("expr: %s\n",expr);
    struct lambda* l = parse(expr,strlen(expr));
    char* out = build(l);
    printf("out: %s\n",out);
    visualize(l, 0);
    reduce(l);
    visualize(l, 0);
    char* new = build(l);
    printf("reduced: %s\n",new);

    return 0;
}