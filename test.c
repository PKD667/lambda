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
        assert(treq(l, l2));
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
    // Create a replacement lambda that is a variable 'y'
    struct lambda* replacement = calloc(1, sizeof(struct lambda));
    replacement->t = VAR;
    replacement->tag = 'y';

    // Create a lambda that is just a variable 'x'
    struct lambda* varExpr = calloc(1, sizeof(struct lambda));
    varExpr->t = VAR;
    varExpr->tag = 'x';

    // Replace 'x' with the replacement.
    struct lambda* result = replace(varExpr, replacement, 'x');

    // Create an expected lambda tree: a variable 'y'
    struct lambda* expected = calloc(1, sizeof(struct lambda));
    expected->t = VAR;
    expected->tag = 'y';

    // Use treq to check the result tree against the expected tree.
    assert(treq(result, expected));

    free(replacement);
    free(varExpr);
    free(expected);
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
    assert(treq(l1, expected));
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
        {"(λa.λb.(b(a)))(λc.(cc))(λd.(d))", "λc.(cc)"}, // reduction with multiple arguments
        {"(λa.λb.λc.abc)(λx.λy.x)(λz.z)(λw.w)","λz.z"},
        {"(λm.λn.λf.λx.mf(nfx))(λa.λc.a(ac))(λb.λe.b(b(be)))", "λf.λx.(f)((f)((f)((f)((f)(x)))))"},
        {NULL, NULL}
    };

    for (int i = 0; cases[i].expr != NULL; i++) {
        printf("---------------------------\n");
        printf("expr: %s\n", cases[i].expr);
        struct lambda* l = parse(cases[i].expr, strlen(cases[i].expr));
        struct lambda* reduced = reduce(l);
        struct lambda* expected = parse(cases[i].expected, strlen(cases[i].expected));
        printf("reduced: %s\n", build(reduced));
        assert(treq(reduced, expected));
        free(reduced);
        free(expected);
        
    }
}

void test_equal() {
    char* as = "(λm.λn.λf.m(nf))(λa.λb.a(a(a(ab))))(λa.λb.ab)";
    struct lambda* a = parse(as, strlen(as));
    char* bs = "(λe.λg.e(e(e(eg))))";
    struct lambda* b = parse(bs, strlen(bs));

    // a and b should not be equal
    assert(!equal(a, b));

    // reduce a
    struct lambda* ra = reduce(a);
    // a and b should be equivalent
    assert(equal(ra, b));

}

int main() {
    test_parse_and_build();
    test_tags();
    test_replace();
    test_convert();
    test_subst();
    test_reduce();
    test_equal();
    printf("All tests passed.\n");
    return 0;
}