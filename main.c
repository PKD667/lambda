#include "main.h"

int main() {
    //char* expr = calloc(1024,1);
    //scanf("%s",expr);
    char* expr = MUL "(λc.λd.c(cd))" "(λa.λb.a(ab))";
    printf("expr: %s\n",expr);
    struct lambda* l = parse(expr,strlen(expr));
    char* out = build(l);
    printf("out: %s\n",out);
    visualize(l, 0);
    l = reduce(l);
    visualize(l, 0);
    char* new = build(l);
    printf("reduced: %s\n",new);

    return 0;
}