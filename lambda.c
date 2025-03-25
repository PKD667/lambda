/*
    This is a very small implementation of a lambda calculus interpreter.
*/

#include "main.h"



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

    //printf("%.*s\n",n,lstr);

    int p = lsp(lstr,n);
    if (p + 1 == n && p != 0 ) {
        return parse(lstr+1,n-2);
    } 

    // 3 cases 
    // p == n -> (expr)
    // p < n  -> (expr) expr ou (expr) expr expr
    // p == 0 -> expr (?expr)
    struct lambda* ex = calloc(1, sizeof(struct lambda));
    // give it a unique id
    ex->uid = rand() / 10000000;


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
    //printf("right\n");
    if (lstr[n-1] == ')') {
        rp = rsp(lstr,n);
        ex->right = parse((lstr+n)-rp,rp);
    } else {
        ex->right = parse(lstr+n-1,1);
        rp += 1;
    }
    //printf("left\n");
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
            if (l->arg == d) l->arg = x->tag;
            replace(l->body, x,d);
            return 1;
    }


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
            struct lambda* na = calloc(1,sizeof(struct lambda));
            na->t = VAR;
            na->tag = z[k]+'a';
            replace(a,na,i+'a');
            free(na);
            k++;
        }
    }
    printf("converted: %s\n",build(a));

    free(c);
    free(d);
    free(z);
    free(ta);
    free(tb);
    return 0;
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
struct lambda* reduce(struct lambda* l) {

    if (l->t == DEF) {
        l->body = reduce(l->body);
        return l;
    }
    if (l->t == VAR) return l;
    

    struct lambda* f = l->left;
    if (f->t == APP) f = reduce(f);

    struct lambda* x = l->right;
    if (x->t == APP) x = reduce(x);

    printf("reducing : %s (:%d:)\n", build(l), l->uid);
    printf("    f: %s (:%d:)\n", build(f), f->uid);
    printf("    x: %s (:%d:)\n", build(x), x->uid);
    struct lambda* b = subst(f, x);
    if (b == NULL) return l;

    b = reduce(b);

    return b;
}


