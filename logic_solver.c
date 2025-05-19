/*
 * Enhanced Logic Resolution Solver Implementation
 * Based on Robinson's Resolution with modern improvements
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <windows.h>

// Color definitions for Windows
#define COLOR_RESET     7   // White (default)
#define COLOR_RED       12  // Light Red
#define COLOR_GREEN     10  // Light Green
#define COLOR_YELLOW    14  // Light Yellow
#define COLOR_BLUE      9   // Light Blue
#define COLOR_MAGENTA   13  // Light Magenta
#define COLOR_CYAN      11  // Light Cyan

// Function to set console color
void set_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void print_credits() {
    set_color(COLOR_CYAN);
    printf("\n");
    printf("███████╗███████╗██╗    ███████╗██╗  ██╗    ██╗███╗   ██╗██╗\n");
    printf("██╔════╝██╔════╝██║    ██╔════╝╚██╗██╔╝    ██║████╗  ██║██║\n");
    printf("█████╗  ███████╗██║    █████╗   ╚███╔╝     ██║██╔██╗ ██║██║\n");
    printf("██╔══╝  ╚════██║██║    ██╔══╝   ██╔██╗     ██║██║╚██╗██║██║\n");
    printf("███████╗███████║██║    ███████╗██╔╝ ██╗    ██║██║ ╚████║██║\n");
    printf("╚══════╝╚══════╝╚═╝    ╚══════╝╚═╝  ╚═╝    ╚═╝╚═╝  ╚═══╝╚═╝\n");
    
    set_color(COLOR_YELLOW);
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║                   Developed by:                        ║\n");
    set_color(COLOR_GREEN);
    printf("║           LASFER  •  MOUSLIM  •  HECHEHOUCHE         ║\n");
    set_color(COLOR_YELLOW);
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    set_color(COLOR_MAGENTA);
    printf("\n              ESI-EX-INI Project - 2024                \n");
    printf("               Advanced Logic Project                   \n\n");
    set_color(COLOR_RESET);
}

#define MAX_VAR_NAME 64
#define INITIAL_CAPACITY 100
#define GROWTH_FACTOR 2
#define MAX_LINE_LENGTH 1024

// Structure to represent a variable name
typedef struct {
    char name[MAX_VAR_NAME];
} Variable;

// Structure to represent a literal (variable or its negation)
typedef struct {
    Variable var;
    bool is_negated;
} Literal;

// Structure to represent a clause (disjunction of literals)
typedef struct {
    Literal *literals;
    int num_literals;
    int capacity;
} Clause;

// Structure to represent a formula (conjunction of clauses)
typedef struct {
    Clause *clauses;
    int num_clauses;
    int capacity;
    Variable *variables;  // Track unique variables
    int num_variables;
    int var_capacity;
} Formula;

// Function to initialize a variable
void init_variable(Variable *var) {
    var->name[0] = '\0';
}

// Function to compare variables
bool variables_equal(Variable *v1, Variable *v2) {
    return strcmp(v1->name, v2->name) == 0;
}

// Function to initialize a clause
bool init_clause(Clause *clause) {
    clause->literals = malloc(INITIAL_CAPACITY * sizeof(Literal));
    if (!clause->literals) return false;
    clause->num_literals = 0;
    clause->capacity = INITIAL_CAPACITY;
    return true;
}

// Function to free a clause
void free_clause(Clause *clause) {
    free(clause->literals);
    clause->literals = NULL;
    clause->num_literals = 0;
    clause->capacity = 0;
}

// Function to initialize a formula
bool init_formula(Formula *formula) {
    formula->clauses = malloc(INITIAL_CAPACITY * sizeof(Clause));
    formula->variables = malloc(INITIAL_CAPACITY * sizeof(Variable));
    if (!formula->clauses || !formula->variables) {
        free(formula->clauses);
        free(formula->variables);
        return false;
    }
    formula->num_clauses = 0;
    formula->capacity = INITIAL_CAPACITY;
    formula->num_variables = 0;
    formula->var_capacity = INITIAL_CAPACITY;
    return true;
}

// Function to free a formula
void free_formula(Formula *formula) {
    for (int i = 0; i < formula->num_clauses; i++) {
        free_clause(&formula->clauses[i]);
    }
    free(formula->clauses);
    free(formula->variables);
    formula->clauses = NULL;
    formula->variables = NULL;
    formula->num_clauses = 0;
    formula->capacity = 0;
    formula->num_variables = 0;
    formula->var_capacity = 0;
}

// Function to find or add a variable to the formula
int find_or_add_variable(Formula *formula, const char *name) {
    // First try to find the variable
    for (int i = 0; i < formula->num_variables; i++) {
        if (strcmp(formula->variables[i].name, name) == 0) {
            return i;
        }
    }
    
    // If not found, add it
    if (formula->num_variables >= formula->var_capacity) {
        int new_capacity = formula->var_capacity * GROWTH_FACTOR;
        Variable *new_vars = realloc(formula->variables, new_capacity * sizeof(Variable));
        if (!new_vars) return -1;
        formula->variables = new_vars;
        formula->var_capacity = new_capacity;
    }
    
    strncpy(formula->variables[formula->num_variables].name, name, MAX_VAR_NAME - 1);
    formula->variables[formula->num_variables].name[MAX_VAR_NAME - 1] = '\0';
    return formula->num_variables++;
}

// Function to add a literal to a clause
bool add_literal(Clause *clause, const char *var_name, bool is_negated) {
    if (clause->num_literals >= clause->capacity) {
        int new_capacity = clause->capacity * GROWTH_FACTOR;
        Literal *new_literals = realloc(clause->literals, new_capacity * sizeof(Literal));
        if (!new_literals) return false;
        clause->literals = new_literals;
        clause->capacity = new_capacity;
    }
    
    strncpy(clause->literals[clause->num_literals].var.name, var_name, MAX_VAR_NAME - 1);
    clause->literals[clause->num_literals].var.name[MAX_VAR_NAME - 1] = '\0';
    clause->literals[clause->num_literals].is_negated = is_negated;
    clause->num_literals++;
    return true;
}

// Function to check if a clause contains a literal
bool clause_contains(Clause *clause, const char *var_name, bool is_negated) {
    for (int i = 0; i < clause->num_literals; i++) {
        if (strcmp(clause->literals[i].var.name, var_name) == 0 && clause->literals[i].is_negated == is_negated) {
            return true;
        }
    }
    return false;
}

// Function to check if a clause is a tautology (contains both p and ¬p)
bool is_tautology(Clause *clause) {
    for (int i = 0; i < clause->num_literals; i++) {
        const char *var = clause->literals[i].var.name;
        bool is_negated = clause->literals[i].is_negated;
        // Check if the opposite polarity exists in the clause
        if (clause_contains(clause, var, !is_negated)) {
            return true;
        }
    }
    return false;
}

// Function to check if a clause is empty (contradiction)
bool is_empty_clause(Clause *clause) {
    return clause->num_literals == 0;
}

// Function to check if two clauses are identical
bool clauses_equal(Clause *c1, Clause *c2) {
    if (c1->num_literals != c2->num_literals) {
        return false;
    }
    
    // Check if every literal in c1 is in c2
    for (int i = 0; i < c1->num_literals; i++) {
        if (!clause_contains(c2, c1->literals[i].var.name, c1->literals[i].is_negated)) {
            return false;
        }
    }
    
    // Check if every literal in c2 is in c1
    for (int i = 0; i < c2->num_literals; i++) {
        if (!clause_contains(c1, c2->literals[i].var.name, c2->literals[i].is_negated)) {
            return false;
        }
    }
    
    return true;
}

// Function to check if a formula already contains a clause
bool formula_contains(Formula *formula, Clause *clause) {
    for (int i = 0; i < formula->num_clauses; i++) {
        if (clauses_equal(&formula->clauses[i], clause)) {
            return true;
        }
    }
    return false;
}

// Function to resolve two clauses on a variable
bool resolve(Clause *c1, Clause *c2, const char *var_name, bool is_negated, Clause *result) {
    init_clause(result);
    
    // Add literals from c1 except var
    for (int i = 0; i < c1->num_literals; i++) {
        if (strcmp(c1->literals[i].var.name, var_name) != 0 || c1->literals[i].is_negated != is_negated) {
            add_literal(result, c1->literals[i].var.name, c1->literals[i].is_negated);
        }
    }
    
    // Add literals from c2 except -var
    for (int i = 0; i < c2->num_literals; i++) {
        if (strcmp(c2->literals[i].var.name, var_name) != 0 || c2->literals[i].is_negated != !is_negated) {
            add_literal(result, c2->literals[i].var.name, c2->literals[i].is_negated);
        }
    }
    
    // Check if result is a tautology
    if (is_tautology(result)) {
        return false;  // Don't use tautologies
    }
    
    return true;
}

// Function to check if a string is a valid variable name
bool is_valid_variable_name(const char *name) {
    if (!name || !*name) return false;
    
    // First character must be a letter or underscore
    if (!isalpha(name[0]) && name[0] != '_') return false;
    
    // Rest can be letters, numbers, or underscores
    for (int i = 1; name[i]; i++) {
        if (!isalnum(name[i]) && name[i] != '_') return false;
    }
    
    return true;
}

// Function to copy a clause properly
bool copy_clause(Clause *dest, Clause *src) {
    if (!init_clause(dest)) return false;
    
    for (int i = 0; i < src->num_literals; i++) {
        if (!add_literal(dest, src->literals[i].var.name, src->literals[i].is_negated)) {
            free_clause(dest);
            return false;
        }
    }
    return true;
}

// Function to add a clause to a formula
bool add_clause(Formula *formula, Clause *clause) {
    if (formula->num_clauses >= formula->capacity) {
        int new_capacity = formula->capacity * GROWTH_FACTOR;
        Clause *new_clauses = realloc(formula->clauses, new_capacity * sizeof(Clause));
        if (!new_clauses) return false;
        formula->clauses = new_clauses;
        formula->capacity = new_capacity;
    }
    
    // Make a proper copy of the clause
    if (!copy_clause(&formula->clauses[formula->num_clauses], clause)) {
        return false;
    }
    formula->num_clauses++;
    return true;
}

// Function to perform unit propagation
bool unit_propagation(Formula *formula) {
    bool changes_made;
    do {
        changes_made = false;
        for (int i = 0; i < formula->num_clauses; i++) {
            if (formula->clauses[i].num_literals == 1) {
                // Found a unit clause
                Literal unit = formula->clauses[i].literals[0];
                
                // Propagate through other clauses
                for (int j = 0; j < formula->num_clauses; j++) {
                    if (i == j) continue;
                    
                    Clause *clause = &formula->clauses[j];
                    for (int k = 0; k < clause->num_literals; k++) {
                        if (strcmp(clause->literals[k].var.name, unit.var.name) == 0) {
                            if (clause->literals[k].is_negated == unit.is_negated) {
                                // Remove this clause (it's satisfied)
                                formula->clauses[j] = formula->clauses[formula->num_clauses - 1];
                                formula->num_clauses--;
                                j--;
                                changes_made = true;
                                break;
                            } else {
                                // Remove this literal
                                clause->literals[k] = clause->literals[clause->num_literals - 1];
                                clause->num_literals--;
                                k--;
                                changes_made = true;
                            }
                        }
                    }
                }
            }
        }
    } while (changes_made);
    
    return true;
}

// Function to perform resolution by refutation
bool resolution(Formula *formula) {
    // Create a working set of clauses
    Clause *work_clauses = malloc(INITIAL_CAPACITY * sizeof(Clause));
    int work_capacity = INITIAL_CAPACITY;
    int work_size = 0;
    
    if (!work_clauses) return true;  // Memory error, assume satisfiable
    
    // Copy original clauses to working set
    for (int i = 0; i < formula->num_clauses; i++) {
        if (work_size >= work_capacity) {
            int new_capacity = work_capacity * GROWTH_FACTOR;
            Clause *new_work = realloc(work_clauses, new_capacity * sizeof(Clause));
            if (!new_work) {
                // Clean up and return
                for (int j = 0; j < work_size; j++) {
                    free_clause(&work_clauses[j]);
                }
                free(work_clauses);
                return true;  // Memory error, assume satisfiable
            }
            work_clauses = new_work;
            work_capacity = new_capacity;
        }
        
        if (!copy_clause(&work_clauses[work_size], &formula->clauses[i])) {
            // Clean up and return
            for (int j = 0; j < work_size; j++) {
                free_clause(&work_clauses[j]);
            }
            free(work_clauses);
            return true;  // Memory error, assume satisfiable
        }
        work_size++;
    }
    
    // Perform resolution
    bool found_empty = false;
    int start = 0;
    
    while (start < work_size && !found_empty) {
        int end = work_size;
        
        for (int i = 0; i < end && !found_empty; i++) {
            for (int j = i + 1; j < end && !found_empty; j++) {
                // Try each literal in clause i
                for (int k = 0; k < work_clauses[i].num_literals; k++) {
                    const char *var = work_clauses[i].literals[k].var.name;
                    bool is_negated = work_clauses[i].literals[k].is_negated;
                    
                    // Look for complementary literal
                    if (clause_contains(&work_clauses[j], var, !is_negated)) {
                        Clause resolvant;
                        if (!init_clause(&resolvant)) continue;
                        
                        if (resolve(&work_clauses[i], &work_clauses[j], var, is_negated, &resolvant)) {
                            if (is_empty_clause(&resolvant)) {
                                found_empty = true;
                                free_clause(&resolvant);
                                break;
                            }
                            
                            // Check if this is a new clause
                            bool is_new = true;
                            for (int m = 0; m < work_size; m++) {
                                if (clauses_equal(&work_clauses[m], &resolvant)) {
                                    is_new = false;
                                    break;
                                }
                            }
                            
                            if (is_new) {
                                if (work_size >= work_capacity) {
                                    int new_capacity = work_capacity * GROWTH_FACTOR;
                                    Clause *new_work = realloc(work_clauses, new_capacity * sizeof(Clause));
                                    if (!new_work) {
                                        free_clause(&resolvant);
                                        continue;
                                    }
                                    work_clauses = new_work;
                                    work_capacity = new_capacity;
                                }
                                work_clauses[work_size++] = resolvant;
                            } else {
                                free_clause(&resolvant);
                            }
                        }
                    }
                }
            }
        }
        start = end;
    }
    
    // Clean up
    for (int i = 0; i < work_size; i++) {
        free_clause(&work_clauses[i]);
    }
    free(work_clauses);
    
    return !found_empty;
}

// Function to read a formula from a file
bool read_formula_from_file(const char *filename, Formula *formula) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        return false;
    }
    
    if (!init_formula(formula)) {
        printf("Error: Failed to initialize formula\n");
        fclose(file);
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Remove trailing newline and whitespace
        char *end = line + strlen(line) - 1;
        while (end >= line && isspace(*end)) *end-- = '\0';
        
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;
        
        Clause clause;
        if (!init_clause(&clause)) {
            free_formula(formula);
            fclose(file);
            return false;
        }
        
        char *token = strtok(line, " \t");
        while (token) {
            bool is_negated = (token[0] == '!');
            char *var_name = token + (is_negated ? 1 : 0);
            
            if (!is_valid_variable_name(var_name)) {
                free_clause(&clause);
                free_formula(formula);
                fclose(file);
                return false;
            }
            
            if (!add_literal(&clause, var_name, is_negated)) {
                free_clause(&clause);
                free_formula(formula);
                fclose(file);
                return false;
            }
            
            token = strtok(NULL, " \t");
        }
        
        if (clause.num_literals > 0 && !is_tautology(&clause)) {
            if (!add_clause(formula, &clause)) {
                free_clause(&clause);
                free_formula(formula);
                fclose(file);
                return false;
            }
        }
        free_clause(&clause);
    }
    
    fclose(file);
    return true;
}

// Main function with improved formatting
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    Formula formula;
    if (!read_formula_from_file(argv[1], &formula)) {
        return 1;
    }
    
    bool is_satisfiable = resolution(&formula);
    
    if (is_satisfiable) {
        printf("SATISFIABLE\n");
    } else {
        printf("UNSATISFIABLE\n");
    }
    
    free_formula(&formula);
    return 0;
}
