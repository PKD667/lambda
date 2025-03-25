#include "main.h"

int main() {
    //char* expr = calloc(1024,1);
    //scanf("%s",expr);
    char* expr = ADD "(" MUL TWO TWO ")" THREE;
    printf("expr: %s\n",expr);
    struct lambda* l = parse(expr,strlen(expr));
    char* out = build(l);
    printf("out: %s\n",out);
    visualize(l, 0);
    struct lambda* n = reduce(l);
    destroy(l);
    visualize(n, 0);
    char* new = build(n);
    printf("reduced: %s\n",new);

    int r = equal(n,parse(SEVEN,strlen(SEVEN)));
    if (r) printf("EQUAL\n");
    else printf("NOT EQUAL\n");

    return 0;
}