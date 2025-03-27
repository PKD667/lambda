#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

// Define a structure for a lambda library entry
typedef struct {
    const char* name;
    const char* expr;
} LambdaEntry;

// Static array that stores lambda expressions such as Church numerals and addition
static const LambdaEntry lambda_library[] = {
    { "zero", ZERO },
    { "one",  ONE },
    { "two",  TWO },
    { "add",  ADD },
    { "mul",  MUL }
};
static const size_t lambda_library_size = sizeof(lambda_library) / sizeof(lambda_library[0]);

int main() {
    char expr[BUFFER_SIZE];

    while (1) {
        printf("Enter lambda expression (or 'exit' to quit): ");
        if (fgets(expr, BUFFER_SIZE, stdin) == NULL)
            break;

        // Remove trailing newline if present
        size_t len = strlen(expr);
        if (len > 0 && expr[len - 1] == '\n') {
            expr[len - 1] = '\0';
            len--;
        }
        
        // Exit command
        if (strcmp(expr, "exit") == 0)
            break;

        // Command to list the library of lambda expressions
        if (strcmp(expr, "library") == 0) {
            printf("Lambda Library:\n");
            for (size_t i = 0; i < lambda_library_size; i++) {
                printf(" - %s: %s\n", lambda_library[i].name, lambda_library[i].expr);
            }
            continue;
        }
        
        // Scan for tokens and substitute any token prefixed with '$'
        if (strchr(expr, '$') != NULL) {
            char substituted[BUFFER_SIZE] = "";
            char *token = strtok(expr, " ");
            int errorFlag = 0;
            while (token != NULL) {
                // Check for a library token
                if (token[0] == '$') {
                    const char* libName = token + 1; // skip '$'
                    int found = 0;
                    for (size_t i = 0; i < lambda_library_size; i++) {
                        if (strcmp(libName, lambda_library[i].name) == 0) {
                            if (strlen(substituted) > 0)
                                strncat(substituted, " ", BUFFER_SIZE - strlen(substituted) - 1);
                            strncat(substituted, lambda_library[i].expr, BUFFER_SIZE - strlen(substituted) - 1);
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        printf("No library entry found for %s\n", libName);
                        errorFlag = 1;
                        break;
                    }
                } else {
                    if (strlen(substituted) > 0)
                        strncat(substituted, " ", BUFFER_SIZE - strlen(substituted) - 1);
                    strncat(substituted, token, BUFFER_SIZE - strlen(substituted) - 1);
                }
                token = strtok(NULL, " ");
            }
            if (errorFlag)
                continue;
            strncpy(expr, substituted, BUFFER_SIZE);
            expr[BUFFER_SIZE - 1] = '\0';
            len = strlen(expr);
        }
        
        struct lambda* l = parse(expr, len);
        if (!l) {
            printf("Error parsing expression.\n");
            continue;
        }
        
        struct lambda* n = reduce(l);
        //destroy(l);
        
        char* newExpr = build(n);
        if (newExpr)
            printf("%s\n", newExpr);
        else
            printf("Error building reduced expression.\n");
    }
    return 0;
}