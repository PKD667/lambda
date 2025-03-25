#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

struct lambda {

    int uid;

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


// Forward declarations from main.c
struct lambda;
int sp(char* lstr, int n, int r);
int rsp(char* lstr, int n);
int lsp(char* lstr, int n);
int isl(char* lstr);
struct lambda* parse(char* lstr, int n);
char* build(struct lambda* l);
void visualize(struct lambda* l,int d);
int* tags(struct lambda* l);
int replace(struct lambda* l, struct lambda* x, int d);
int convert(struct lambda* a, struct lambda* b);
struct lambda* subst(struct lambda* l, struct lambda* x);
struct lambda* reduce(struct lambda* l);

// Also include definitions of lambda calculus constants
#define LAMBDA "λ"

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
#define MUL "(λm.λn.λf.m(nf))"
