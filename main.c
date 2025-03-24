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
    } while (d > 0 && i < n);

    if (d != 0) return -1;

    return i-1;
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

    printf("parse: %.*s\n",n,lstr);

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
        assert(isalpha(*lstr));
        ex->t = VAR;
        ex->tag = *(lstr);
        goto return_free;
    } 

    if (p > 0) {
        // right should be the rightmost expression
        printf("right\n");
        int rp = 0;
        if (lstr[n] == ')') {
            rp = rsp(lstr,n);
            ex->right = parse(lstr+rp+1,n-rp-1);
        } else {
            ex->right = parse(lstr+n-1,1);
            rp += 1;
        }
        printf("left\n");
        // left should be the (expr) + any other expressions before the last one
        ex->left = parse(lstr,n-rp);
        ex->t = APP;
        goto return_free;
    }
   
    ex->t = APP;
    ex->left = parse(lstr,1);
    ex->right = parse(lstr+1,n-1);


return_free:
    return ex;
    
}



char* build(struct lambda* l) {
    int a = 1024;
    char* s = calloc(a, 1);
    s[0] = '(';
    if (l->t == APP) {
        char* sl = build(l->left);
        char* sr = build(l->right);
        if (strlen(sl) + strlen(sr) > a) 
            s = realloc(s, strlen(sl) + strlen(sr) + a);
        strcat(s, sl);
        strcat(s, sr);
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

    strcat(s,")");

    return s;
}

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
            visualize(l->right, d+1);
            visualize(l->left, d+1);
            break;
    }

}

int replace(struct lambda* l, struct lambda* x, int d) {
    visualize(l, 0);
    switch (l->t) {
        case VAR:
            printf("'%c' == '%c'\n",l->tag,d);
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
int subst(struct lambda* l, struct lambda* x) {
    if (l->t != DEF) return 1;

    // variable to replace
    int r = l->arg;
    struct lambda* b = l->body;

    // replace all instances of r in x with b
    return replace(b, x, r);
}

int reduce(struct lambda* l) {
    if (l->t != APP) return 1;

    struct lambda* f = l->left;
    if (f->t != DEF) reduce(f);

    struct lambda* x = l->right;

    subst(f, x);

    *l = *f;

    return 0;
}

#define TRUE "(λt.λf.t)"
#define FALSE "(λt.λf.f)"
#define AND "(λp.λq.pqp)"
#define OR "(λp.λq.ppq)"
#define NOT "(λp.λa.λb.pba)"



int main() {
    //char* expr = calloc(1024,1);
    //scanf("%s",expr);
    char* expr = TRUE "ab";
    printf("expr: %s\n",expr);
    struct lambda* l = parse(expr,strlen(expr));
    char* out = build(l);
    printf("out: %s\n",out);
    visualize(l, 0);
    printf("reduce: %d\n",reduce(l));  
    //visualize(l, 0);
    char* new = build(l);
    printf("%s\n",new);

    return 0;
}