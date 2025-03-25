#include "main.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------
// TEST FUNCTIONS
// ------------------------

//--------
// UTILS
//--------

int treequal(struct lambda* l1, struct lambda* l2) {
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
            return treequal(l1->left, l2->left) && treequal(l1->right, l2->right);
        case DEF: // LAMBDA
            return l1->arg == l2->arg && treequal(l1->body, l2->body);
    }
}

void test_parse_and_build() {
    // Array of test lambda expressions.
    const char* tests[] = {
        "λa.a",
        "λa.(aa)",
        "(λa.a)(b)",
        "λa.λb.(ab)",
        "((λa.a)(λb.b))",
        "λa.(λb.(ba))",
        NULL
    };

    for (int i = 0; tests[i] != NULL; i++) {
        const char* expr = tests[i];
        struct lambda* l = parse((char*)expr, strlen(expr));
        char* rebuilt = build(l);
        // re parse
        struct lambda* l2 = parse(rebuilt, strlen(rebuilt));
        assert(treequal(l, l2));
        free(rebuilt);
    }
}

void test_tags() {
    char* expr = "λa.(λb.ab)";
    struct lambda* l = parse(expr, strlen(expr));
    int* t = tags(l);
    // Expect tags for variables 'a' and 'b'
    // 'a' index is 0 and 'b' index is 1 in the array.
    assert(t[0] == 1);
    assert(t[1] == 1);
    free(t);
}

void test_replace() {
    // Create a variable to be used as a replacement.
    struct lambda* replacement = calloc(1, sizeof(struct lambda));
    replacement->t = 0; // VAR
    replacement->tag = 'y';
    
    // Create a lambda that is just a variable 'x'
    struct lambda* varExpr = calloc(1, sizeof(struct lambda));
    varExpr->t = 0; // VAR
    varExpr->tag = 'x';

    // Replace 'x' with the replacement.
    replace(varExpr, replacement, 'x');
    assert(varExpr->tag == 'y');
    
    free(replacement);
    free(varExpr);
}

void test_convert() {
    // Two identical lambda abstractions that share the same bound variable.
    char* expr1 = "λa.a";
    char* expr2 = "λa.a";
    struct lambda* l1 = parse(expr1, strlen(expr1));
    struct lambda* l2 = parse(expr2, strlen(expr2));
    // This should rename bound variables in l1 to avoid collision.
    convert(l1, l2);
    char* rebuilt = build(l1);
    printf("rebuilt: %s\n", rebuilt);
    // The rebuilt expression should be "λb.b"
    struct lambda* expected = parse("λb.b", strlen("λb.b"));
    assert(treequal(l1, expected));
    free(rebuilt);
}

void test_subst() {
    // Test substitution: ((λa.a)(b)) should reduce to b.
    char* expr = "(λa.a)(b)";
    struct lambda* app = parse(expr, strlen(expr));
    // subst expects the left side to be a lambda definition.
    struct lambda* result = subst(app->left, app->right);
    char* rebuilt = build(result);
    // The result should be simply "b"
    assert(strcmp(rebuilt, "b") == 0);
    free(rebuilt);
}

void test_reduce() {
    // Array of test cases, each with an expression and its expected reduced result.
    struct {
        char* expr;
        char* expected;
    } cases[] = {
        {"(λa.a)(b)", "b"},                   // identity reduction
        {"((λa.λb.a)(x))(y)", "x"},            // two-step reduction: (λa.λb.a) applied to x then y
        {"(λa.(λb.b))(c)", "λb.b"},            // reduction within lambda abstraction
        {"λa.a", "λa.a"},                     // no reduction for a lambda abstraction
        {"(λa.λb.(ab))(x)(y)", "xy"},          // reduction of a lambda abstraction applied to two arguments
        {"(λa.λb.(ab))(x)(λc.c)", "x(λc.c)"},    // reduction of a lambda abstraction applied to two arguments, one of which is a lambda abstraction
        // Additional cases:
        {"((λa.a)((λb.b)))", "λb.b"},                     // extra parentheses around redex
        {"(λa.(λb.(ab)))(c)", "λb.(cb)"},               // reduction in outer lambda yields a lambda
        {"((λa.λb.(ab))(λc.(λd.(cd))))(λe.e)", "λd.d"}, // multi-step reduction with nested abstractions
        {"((λa.a)((λb.b)))", "λb.b"},                   // application with extra spaces and parentheses
        {NULL, NULL}
    };

    for (int i = 0; cases[i].expr != NULL; i++) {
        struct lambda* l = parse(cases[i].expr, strlen(cases[i].expr));
        struct lambda* reduced = reduce(l);
        struct lambda* expected = parse(cases[i].expected, strlen(cases[i].expected));
        assert(treequal(reduced, expected));
        free(reduced);
        free(expected);
        
    }
}

int main() {
    test_parse_and_build();
    test_tags();
    test_replace();
    test_convert();
    test_subst();
    test_reduce();
    printf("All tests passed.\n");
    return 0;
}