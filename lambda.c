/*
    This is a very small implementation of a lambda calculus interpreter.
*/

#include "main.h"
#include <string.h>

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

char* unspace(char* lstr, int* n) {
    char* s = calloc(strlen(lstr)+1,1);
    int k = 0;
    int j = 0;
    for (int i = 0; i < strlen(lstr); i++) {
        if (lstr[i] != ' ') {
            s[j] = lstr[i];
            j++;
        } else {
            k++;
        }
    }
    if (j == strlen(lstr)) return lstr;
    *n -= k;
    return s;

}


// first we need to parse the lambda string into a structured form
struct lambda* parse(char* lstr, int n) {

    lstr = unspace(lstr,&n); 


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


    if (isl(lstr)) {
        ex->t = DEF;
        ex->arg = *(lstr+strlen(LAMBDA));
        assert(*(lstr+strlen(LAMBDA)+1) == '.');
        ex->body = parse(lstr+strlen(LAMBDA)+2,n-strlen(LAMBDA)-2);
        goto return_free;
    }

    if (n == 1) {
        //printf("'%c'\n",*lstr);
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

int treq(struct lambda* l1, struct lambda* l2) {
    if (l1 == NULL && l2 == NULL) {
        return 1;
    }
    if (l1 == NULL || l2 == NULL) {
        return 0;
    }
    if (l1->t != l2->t) {
        return 0;
    }
    switch (l1->t) {
        case VAR: // VAR
            return l1->tag == l2->tag;
        case APP: // APP
            return treq(l1->left, l2->left) && treq(l1->right, l2->right);
        case DEF: // LAMBDA
            return l1->arg == l2->arg && treq(l1->body, l2->body);
    }
}

struct lambda* copy(struct lambda* l) {
    struct lambda* n = calloc(1, sizeof(struct lambda));
    n->t = l->t;
    switch (l->t) {
        case VAR:
            n->tag = l->tag;
            break;
        case APP:
            n->left = copy(l->left);
            n->right = copy(l->right);
            break;
        case DEF:
            n->arg = l->arg;
            n->body = copy(l->body);
            break;
    }
    return n;
}

int destroy(struct lambda* l) {
    switch (l->t) {
        case VAR:
            free(l);
            break;
        case APP:
            destroy(l->left);
            destroy(l->right);
            free(l);
            break;
        case DEF:
            destroy(l->body);
            free(l);
            break;
    }
    return 0;
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
    int* t = calloc(27, sizeof(int));
    for (int i = 0; i < strlen(str); i++) {
        if (isalpha(str[i]) && str[i+1]== '.' ) t[str[i]-'a'] = 1;
    }
    free(str);
    return t;
}

int move(struct lambda* l, int d, int r) {
    switch (l->t) {
        case VAR:
            if (l->tag == d) {
                l->tag = r;
                return 1;
            }
            return 0;
        case APP:
            move(l->left, d, r);
            move(l->right, d, r);
            return 1;
        case DEF:
            if (l->arg == d) {
                l->arg = r;
            }
            move(l->body, d, r);
            return 1;
    }
}

int equal(struct lambda* a, struct lambda* b) {

    struct lambda* be = copy(b);
    convert(be, a);
    struct lambda* bd = copy(be);

    // in order to do an extensive comparisoon
    // we need to set the same variables
    int* ta = tags(a);
    int* tb = tags(be);

    // if not the same number of vars, return false
    int la = 0;
    int lb = 0;

    int* tbl = calloc(26, sizeof(int));
    int* tal = calloc(26, sizeof(int));

    for (int i = 0; i < 26; i++) {

        if (tb[i]) tbl[lb] = i;
        if (ta[i]) tal[la] = i;
        la += ta[i];
        lb += tb[i];
    }
    if (la != lb) return 0;

    // move all tags in be by their corresponding tags in a
    for (int i = 0; i < lb; i++) {
        move(be,tbl[i]+'a',tal[i]+'a');
    }
    // now compare the tree
    int r = treq(a,be);
    if (!r) {
        // move all tags in be by their corresponding tags in a
        for (int i = 0; i < lb; i++) {
            move(bd,tbl[lb-i-1]+'a',tal[i]+'a');
        }
        r = treq(a,bd);
    }

    free(be);

    free(ta);
    free(tb);


    return r;
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

   //printf("s: %d\n",s);

    int k = 0;
    // replace all instances of a variable in a with the corresponding z
    for (int i = 0; i < 26; i++) {
        if (c[i]) {
            //printf("'%c' -> '%c'\n",i+'a',z[k]+'a');
            move(a,i+'a',z[k]+'a');
            k++;
        }
    }
    //printf("converted: %s\n",build(a));

    free(c);
    free(d);
    free(z);
    free(ta);
    free(tb);
    return 0;
}




// replace all instances of a variable with a lambda expression
struct lambda* replace(struct lambda* l, struct lambda* x, int d) {

    struct lambda* n = calloc(1, sizeof(struct lambda));
    n->t = l->t;

    switch (l->t) {
        case VAR:
            //printf("'%c' == '%c'\n",l->tag,d);
            if (l->tag == d) {
                // replace
                n = copy(x);
            } else {
                n->tag = l->tag;
            }
            break;
        case APP:
            n->left = replace(l->left, x,d);
            n->right = replace(l->right, x,d);
            break;
        case DEF:
            // TODO: handle collisions
            n->body = replace(l->body, x,d);
            if (l->arg == d) {
                n->arg = x->tag;
            } else {
                n->arg = l->arg;
            }
            break;
    }

    return n;
}



// implement subsitution for a function in a lambda expression
struct lambda* subst(struct lambda* l, struct lambda* x) {
    printf("<====================>\n");
    printf("Subst %s into %s (:%p:)\n",build(x),build(l),l);
    //convert(l,x);

    // variable to replace
    int r = l->arg;
    struct lambda* b = l->body;

    // replace all instances of r in x with b
    b = replace(b,x,r);
    printf("Substituted: %s (:%p:)\n",build(b),b);
    printf("<====================>\n");

    return b;
}


// beta-reduce a lambda expression
struct lambda* reduce(struct lambda* l) {

    static int depth = 0;
    depth++;

    static struct lambda* root = NULL;
    if (root == NULL) root = l;
    //printf("root : %s\n", build(root));

    if (l->t == DEF) {
        // reduce deeper
        //printf("deeper\n");
        l->body = reduce(l->body);
        //printf("deeper reduced\n");
        depth--;
        //printf("depth: %d\n",depth);
        return l;
    }
    // If we found a variable, go back up
    if (l->t == VAR) {
        depth--;
        //printf("depth: %d\n",depth);
        return l;
    }
    
    //printf("LEFT: %s (:%p:)\n", build(l->left), l->right);
    struct lambda* f = l->left;
    // reducing the left side
    if (f->t == APP) f = reduce(f);

    //printf("RIGHT %s (:%p:)\n", build(l->right), l->right);
    struct lambda* x = l->right;
    // reducing the right side
    if (x->t == APP) x = reduce(x);

    l = (struct lambda*)realloc(l, sizeof(struct lambda));
    l->t = APP;
    l->left = f;
    l->right = x;

    if (f->t != DEF) {
        //printf("depth: %d\n",depth);
        depth--;
        return l;
    }


    printf("reducing : %s (:%p:)\n", build(l), l);
    printf("    f: %s (:%p:)\n", build(f), f);
    printf("    x: %s (:%p:)\n", build(x), x);
    // if the left side is a lambda definition, substitute the right side
    struct lambda* b = subst(f, x);

    //printf("reduced (%p): \n",b);
    //visualize(b,0);

    b = reduce(b);

    //printf("depth: %d\n",depth);
    depth--;
    return b;
}


