#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define DEFAULT_CAPACITY 10
#define MAX_LINE_LENGTH 1024

// ---- Literal Structure and Functions ----

typedef struct {
    char* name;     // Variable name
    bool negated;   // True if literal is negated
} Literal;

// Create a new literal
Literal* createLiteral(char* name, bool negated) {
    Literal* literal = (Literal*)malloc(sizeof(Literal));
    if (literal == NULL) {
        return NULL;
    }
    
    literal->name = strdup(name);
    if (literal->name == NULL) {
        free(literal);
        return NULL;
    }
    
    literal->negated = negated;
    return literal;
}

// Free memory allocated for a literal
void freeLiteral(Literal* literal) {
    if (literal != NULL) {
        free(literal->name);
        free(literal);
    }
}

// Create a copy of a literal
Literal* copyLiteral(const Literal* original) {
    if (original == NULL) {
        return NULL;
    }
    return createLiteral(original->name, original->negated);
}

// Negate a literal (return a new literal that is the negation)
Literal* negateLiteral(const Literal* original) {
    if (original == NULL) {
        return NULL;
    }
    return createLiteral(original->name, !original->negated);
}

// Check if two literals are equal
bool literalsEqual(const Literal* lit1, const Literal* lit2) {
    if (lit1 == NULL || lit2 == NULL) {
        return false;
    }
    
    return strcmp(lit1->name, lit2->name) == 0 && lit1->negated == lit2->negated;
}

// Check if two literals are complementary (one is the negation of the other)
bool literalsComplementary(const Literal* lit1, const Literal* lit2) {
    if (lit1 == NULL || lit2 == NULL) {
        return false;
    }
    
    return strcmp(lit1->name, lit2->name) == 0 && lit1->negated != lit2->negated;
}

// Parse a string into a literal (handles negation with '!')
Literal* parseLiteral(const char* str) {
    if (str == NULL || *str == '\0') {
        return NULL;
    }
    
    bool negated = false;
    const char* name = str;
    
    // Check if the literal is negated (starts with !)
    if (*str == '!') {
        negated = true;
        name++; // Skip the negation symbol
    }
    
    // Handle empty name after negation
    if (*name == '\0') {
        return NULL;
    }
    
    return createLiteral((char*)name, negated);
}

// Convert a literal to string representation
char* literalToString(const Literal* literal) {
    if (literal == NULL) {
        return NULL;
    }
    
    size_t len = strlen(literal->name) + (literal->negated ? 2 : 1); // +1 for '!' if negated, +1 for '\0'
    char* result = (char*)malloc(len);
    
    if (result == NULL) {
        return NULL;
    }
    
    if (literal->negated) {
        sprintf(result, "!%s", literal->name);
    } else {
        strcpy(result, literal->name);
    }
    
    return result;
}

// ---- Clause Structure and Functions ----

typedef struct {
    Literal** literals;  // Array of pointers to literals
    int size;           // Number of literals in the clause
    int capacity;       // Capacity of the literals array
} Clause;

// Create a new empty clause with initial capacity
Clause* createClause(int initialCapacity) {
    if (initialCapacity <= 0) {
        initialCapacity = DEFAULT_CAPACITY;
    }
    
    Clause* clause = (Clause*)malloc(sizeof(Clause));
    if (clause == NULL) {
        return NULL;
    }
    
    clause->literals = (Literal**)malloc(sizeof(Literal*) * initialCapacity);
    if (clause->literals == NULL) {
        free(clause);
        return NULL;
    }
    
    clause->size = 0;
    clause->capacity = initialCapacity;
    return clause;
}

// Free memory allocated for a clause and its literals
void freeClause(Clause* clause) {
    if (clause == NULL) {
        return;
    }
    
    // Free all literals in the clause
    for (int i = 0; i < clause->size; i++) {
        freeLiteral(clause->literals[i]);
    }
    
    free(clause->literals);
    free(clause);
}

// Add a literal to a clause
bool addLiteralToClause(Clause* clause, Literal* literal) {
    if (clause == NULL || literal == NULL) {
        return false;
    }
    
    // Check if the clause already contains this literal
    for (int i = 0; i < clause->size; i++) {
        if (literalsEqual(clause->literals[i], literal)) {
            // Literal already exists, don't add it again
            return true;
        }
    }
    
    // Check if we need to resize
    if (clause->size >= clause->capacity) {
        int newCapacity = clause->capacity * 2;
        Literal** newLiterals = (Literal**)realloc(clause->literals, sizeof(Literal*) * newCapacity);
        
        if (newLiterals == NULL) {
            return false;
        }
        
        clause->literals = newLiterals;
        clause->capacity = newCapacity;
    }
    
    // Add the literal
    clause->literals[clause->size++] = copyLiteral(literal);
    return true;
}

// Check if a clause contains a literal
bool clauseContainsLiteral(const Clause* clause, const Literal* literal) {
    if (clause == NULL || literal == NULL) {
        return false;
    }
    
    for (int i = 0; i < clause->size; i++) {
        if (literalsEqual(clause->literals[i], literal)) {
            return true;
        }
    }
    
    return false;
}

// Check if a clause is empty (has no literals)
bool isEmptyClause(const Clause* clause) {
    return clause == NULL || clause->size == 0;
}

// Create a copy of a clause
Clause* copyClause(const Clause* original) {
    if (original == NULL) {
        return NULL;
    }
    
    Clause* copy = createClause(original->capacity);
    if (copy == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < original->size; i++) {
        Literal* literalCopy = copyLiteral(original->literals[i]);
        if (literalCopy == NULL || !addLiteralToClause(copy, literalCopy)) {
            freeLiteral(literalCopy);
            freeClause(copy);
            return NULL;
        }
        freeLiteral(literalCopy);  // addLiteralToClause creates its own copy
    }
    
    return copy;
}

// Parse a string into a clause (format: "A !B C" = A OR NOT B OR C)
Clause* parseClause(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    Clause* clause = createClause(DEFAULT_CAPACITY);
    if (clause == NULL) {
        return NULL;
    }
    
    // Make a copy of the string that we can modify
    char* strCopy = strdup(str);
    if (strCopy == NULL) {
        freeClause(clause);
        return NULL;
    }
    
    // Tokenize the string by whitespace
    char* token = strtok(strCopy, " \t\n");
    while (token != NULL) {
        Literal* literal = parseLiteral(token);
        if (literal != NULL) {
            if (!addLiteralToClause(clause, literal)) {
                freeLiteral(literal);
                free(strCopy);
                freeClause(clause);
                return NULL;
            }
            freeLiteral(literal);  // addLiteralToClause creates its own copy
        }
        
        token = strtok(NULL, " \t\n");
    }
    
    free(strCopy);
    return clause;
}

// Convert a clause to string representation
char* clauseToString(const Clause* clause) {
    if (clause == NULL) {
        return NULL;
    }
    
    if (clause->size == 0) {
        // Empty clause (contradiction)
        return strdup("[]");
    }
    
    // First pass: determine the total length needed
    size_t totalLength = 2;  // For '(' and ')'
    for (int i = 0; i < clause->size; i++) {
        char* literalStr = literalToString(clause->literals[i]);
        if (literalStr == NULL) {
            return NULL;
        }
        
        totalLength += strlen(literalStr);
        free(literalStr);
        
        if (i < clause->size - 1) {
            totalLength += 3;  // For " ∨ "
        }
    }
    
    // Allocate the result string
    char* result = (char*)malloc(totalLength + 1);  // +1 for null terminator
    if (result == NULL) {
        return NULL;
    }
    
    // Build the result string
    char* ptr = result;
    *ptr++ = '(';
    
    for (int i = 0; i < clause->size; i++) {
        char* literalStr = literalToString(clause->literals[i]);
        if (literalStr == NULL) {
            free(result);
            return NULL;
        }
        
        strcpy(ptr, literalStr);
        ptr += strlen(literalStr);
        free(literalStr);
        
        if (i < clause->size - 1) {
            strcpy(ptr, " ∨ ");
            ptr += 3;
        }
    }
    
    *ptr++ = ')';
    *ptr = '\0';
    
    return result;
}

// ---- Formula Structure and Functions ----

typedef struct {
    Clause** clauses;   // Array of pointers to clauses
    int size;           // Number of clauses in the formula
    int capacity;       // Capacity of the clauses array
} Formula;

// Create a new empty formula with initial capacity
Formula* createFormula(int initialCapacity) {
    if (initialCapacity <= 0) {
        initialCapacity = DEFAULT_CAPACITY;
    }
    
    Formula* formula = (Formula*)malloc(sizeof(Formula));
    if (formula == NULL) {
        return NULL;
    }
    
    formula->clauses = (Clause**)malloc(sizeof(Clause*) * initialCapacity);
    if (formula->clauses == NULL) {
        free(formula);
        return NULL;
    }
    
    formula->size = 0;
    formula->capacity = initialCapacity;
    return formula;
}

// Free memory allocated for a formula and its clauses
void freeFormula(Formula* formula) {
    if (formula == NULL) {
        return;
    }
    
    // Free all clauses in the formula
    for (int i = 0; i < formula->size; i++) {
        freeClause(formula->clauses[i]);
    }
    
    free(formula->clauses);
    free(formula);
}

// Add a clause to a formula
bool addClauseToFormula(Formula* formula, Clause* clause) {
    if (formula == NULL || clause == NULL) {
        return false;
    }
    
    // Check for duplicate clauses
    for (int i = 0; i < formula->size; i++) {
        bool isDuplicate = true;
        
        // If clauses have different sizes, they can't be duplicates
        if (formula->clauses[i]->size != clause->size) {
            isDuplicate = false;
            continue;
        }
        
        // Check if each literal in the clause is in the existing clause
        for (int j = 0; j < clause->size; j++) {
            bool literalFound = false;
            for (int k = 0; k < formula->clauses[i]->size; k++) {
                if (literalsEqual(clause->literals[j], formula->clauses[i]->literals[k])) {
                    literalFound = true;
                    break;
                }
            }
            
            if (!literalFound) {
                isDuplicate = false;
                break;
            }
        }
        
        if (isDuplicate) {
            return true; // Clause already exists, don't add it again
        }
    }
    
    // Check if we need to resize
    if (formula->size >= formula->capacity) {
        int newCapacity = formula->capacity * 2;
        Clause** newClauses = (Clause**)realloc(formula->clauses, sizeof(Clause*) * newCapacity);
        
        if (newClauses == NULL) {
            return false;
        }
        
        formula->clauses = newClauses;
        formula->capacity = newCapacity;
    }
    
    // Add the clause
    formula->clauses[formula->size++] = copyClause(clause);
    return true;
}

// Check if a formula contains the empty clause
bool formulaContainsEmptyClause(const Formula* formula) {
    if (formula == NULL) {
        return false;
    }
    
    for (int i = 0; i < formula->size; i++) {
        if (isEmptyClause(formula->clauses[i])) {
            return true;
        }
    }
    
    return false;
}

// Helper function to find resolvable literals between two clauses
bool findResolvableLiterals(const Clause* clause1, const Clause* clause2, int* literalIndex1, int* literalIndex2) {
    for (int i = 0; i < clause1->size; i++) {
        for (int j = 0; j < clause2->size; j++) {
            if (literalsComplementary(clause1->literals[i], clause2->literals[j])) {
                *literalIndex1 = i;
                *literalIndex2 = j;
                return true;
            }
        }
    }
    return false;
}

// Apply resolution between two clauses and add the result to the formula
// Returns true if a new clause was added to the formula
bool applyResolution(Formula* formula, const Clause* clause1, const Clause* clause2) {
    if (formula == NULL || clause1 == NULL || clause2 == NULL) {
        return false;
    }
    
    int literalIndex1, literalIndex2;
    if (!findResolvableLiterals(clause1, clause2, &literalIndex1, &literalIndex2)) {
        return false; // No resolvable literals
    }
    
    // Create a new clause with all literals from both clauses except the complementary ones
    Clause* resolvent = createClause(clause1->size + clause2->size - 2);
    if (resolvent == NULL) {
        return false;
    }
    
    // Add literals from the first clause (except the complementary one)
    for (int i = 0; i < clause1->size; i++) {
        if (i != literalIndex1) {
            if (!addLiteralToClause(resolvent, clause1->literals[i])) {
                freeClause(resolvent);
                return false;
            }
        }
    }
    
    // Add literals from the second clause (except the complementary one)
    for (int i = 0; i < clause2->size; i++) {
        if (i != literalIndex2) {
            if (!addLiteralToClause(resolvent, clause2->literals[i])) {
                freeClause(resolvent);
                return false;
            }
        }
    }
    
    // Check if the resolvent is a tautology (contains both p and !p)
    for (int i = 0; i < resolvent->size; i++) {
        for (int j = i + 1; j < resolvent->size; j++) {
            if (literalsComplementary(resolvent->literals[i], resolvent->literals[j])) {
                freeClause(resolvent);
                return false; // Tautology, don't add it
            }
        }
    }
    
    // Add the resolvent to the formula
    bool added = addClauseToFormula(formula, resolvent);
    freeClause(resolvent);
    return added;
}

// Convert a formula to string representation
char* formulaToString(const Formula* formula) {
    if (formula == NULL) {
        return NULL;
    }
    
    if (formula->size == 0) {
        return strdup("(empty formula)");
    }
    
    // First pass: determine the total length needed
    size_t totalLength = 0;
    for (int i = 0; i < formula->size; i++) {
        char* clauseStr = clauseToString(formula->clauses[i]);
        if (clauseStr == NULL) {
            return NULL;
        }
        
        totalLength += strlen(clauseStr);
        free(clauseStr);
        
        if (i < formula->size - 1) {
            totalLength += 3;  // For " ∧ "
        }
    }
    
    // Allocate the result string
    char* result = (char*)malloc(totalLength + 1);  // +1 for null terminator
    if (result == NULL) {
        return NULL;
    }
    
    // Build the result string
    char* ptr = result;
    *ptr = '\0';
    
    for (int i = 0; i < formula->size; i++) {
        char* clauseStr = clauseToString(formula->clauses[i]);
        if (clauseStr == NULL) {
            free(result);
            return NULL;
        }
        
        strcpy(ptr, clauseStr);
        ptr += strlen(clauseStr);
        free(clauseStr);
        
        if (i < formula->size - 1) {
            strcpy(ptr, " ∧ ");
            ptr += 3;
        }
    }
    
    return result;
}

// Parse a formula from a file
Formula* parseFormulaFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    Formula* formula = createFormula(DEFAULT_CAPACITY);
    if (formula == NULL) {
        fclose(file);
        return NULL;
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        Clause* clause = parseClause(line);
        if (clause != NULL) {
            addClauseToFormula(formula, clause);
            freeClause(clause);
        }
    }
    
    fclose(file);
    return formula;
}

// Perform resolution by refutation on the formula
// Returns true if the formula is unsatisfiable
bool isUnsatisfiable(Formula* formula) {
    if (formula == NULL) {
        return false;
    }
    
    if (formulaContainsEmptyClause(formula)) {
        return true;
    }
    
    // Keep track of pairs of clauses that have been resolved
    int maxPairs = formula->size * formula->size;
    bool** resolvedPairs = (bool**)malloc(sizeof(bool*) * formula->size);
    if (resolvedPairs == NULL) {
        return false;
    }
    
    for (int i = 0; i < formula->size; i++) {
        resolvedPairs[i] = (bool*)calloc(formula->size, sizeof(bool));
        if (resolvedPairs[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(resolvedPairs[j]);
            }
            free(resolvedPairs);
            return false;
        }
    }
    
    // Keep applying resolution until we derive the empty clause or no new clauses can be derived
    bool newClauseAdded;
    do {
        newClauseAdded = false;
        int currentSize = formula->size;
        
        for (int i = 0; i < currentSize; i++) {
            for (int j = i + 1; j < currentSize; j++) {
                if (!resolvedPairs[i][j]) {
                    resolvedPairs[i][j] = true;
                    
                    if (applyResolution(formula, formula->clauses[i], formula->clauses[j])) {
                        newClauseAdded = true;
                        
                        // Check if the empty clause was derived
                        if (formulaContainsEmptyClause(formula)) {
                            // Free memory
                            for (int k = 0; k < formula->size; k++) {
                                free(resolvedPairs[k]);
                            }
                            free(resolvedPairs);
                            return true;
                        }
                    }
                }
            }
        }
        
        // Add pairs for the new clauses
        for (int i = currentSize; i < formula->size; i++) {
            resolvedPairs[i] = (bool*)calloc(formula->size, sizeof(bool));
            if (resolvedPairs[i] == NULL) {
                for (int j = 0; j < i; j++) {
                    free(resolvedPairs[j]);
                }
                free(resolvedPairs);
                return false;
            }
        }
    } while (newClauseAdded);
    
    // Free memory
    for (int i = 0; i < formula->size; i++) {
        free(resolvedPairs[i]);
    }
    free(resolvedPairs);
    
    return false;
}

// Write formula to a file
bool writeFormulaToFile(const Formula* formula, const char* filename) {
    if (formula == NULL || filename == NULL) {
        return false;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    
    for (int i = 0; i < formula->size; i++) {
        for (int j = 0; j < formula->clauses[i]->size; j++) {
            Literal* lit = formula->clauses[i]->literals[j];
            if (lit->negated) {
                fprintf(file, "!");
            }
            fprintf(file, "%s", lit->name);
            
            if (j < formula->clauses[i]->size - 1) {
                fprintf(file, " ");
            }
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    return true;
}

// ---- Main function to test the implementation ----

int main() {
    printf("Logic Resolution Solver - Testing\n");
    printf("=================================\n\n");
    
    // Test 1: Create and manipulate literals
    printf("Test 1: Literals\n");
    Literal* litA = createLiteral("A", false);
    Literal* litNotA = createLiteral("A", true);
    Literal* litB = createLiteral("B", false);
    
    printf("Literal A: %s\n", literalToString(litA));
    printf("Literal !A: %s\n", literalToString(litNotA));
    printf("Literals A and !A are complementary: %s\n", 
           literalsComplementary(litA, litNotA) ? "true" : "false");
    printf("Literals A and B are complementary: %s\n", 
           literalsComplementary(litA, litB) ? "true" : "false");
    
    // Test 2: Create and manipulate clauses
    printf("\nTest 2: Clauses\n");
    Clause* clause1 = createClause(10);
    addLiteralToClause(clause1, litA);
    addLiteralToClause(clause1, litB);
    
    Clause* clause2 = createClause(10);
    addLiteralToClause(clause2, litNotA);
    
    char* str1 = clauseToString(clause1);
    char* str2 = clauseToString(clause2);
    printf("Clause 1: %s\n", str1);
    printf("Clause 2: %s\n", str2);
    free(str1);
    free(str2);
    
    // Test 3: Parse literals and clauses from strings
    printf("\nTest 3: Parsing\n");
    Literal* parsedLit = parseLiteral("!X");
    printf("Parsed literal !X: %s\n", literalToString(parsedLit));
    freeLiteral(parsedLit);
    
    Clause* parsedClause = parseClause("P !Q R");
    printf("Parsed clause 'P !Q R': %s\n", clauseToString(parsedClause));
    freeClause(parsedClause);
    
    // Test 4: Create and manipulate formulas
    printf("\nTest 4: Formulas\n");
    Formula* formula = createFormula(10);
    addClauseToFormula(formula, clause1);
    addClauseToFormula(formula, clause2);
    
    char* formulaStr = formulaToString(formula);
    printf("Formula: %s\n", formulaStr);
    free(formulaStr);
    
    // Test 5: Resolution
    printf("\nTest 5: Resolution\n");
    bool resolved = applyResolution(formula, clause1, clause2);
    printf("Resolution applied: %s\n", resolved ? "true" : "false");
    
    formulaStr = formulaToString(formula);
    printf("Formula after resolution: %s\n", formulaStr);
    free(formulaStr);
    
    // Test 6: Unsatisfiability check
    printf("\nTest 6: Unsatisfiability Check\n");
    // Create a simple unsatisfiable formula: (A) ∧ (!A)
    Formula* unsatFormula = createFormula(10);
    Clause* clauseA = createClause(1);
    addLiteralToClause(clauseA, litA);
    Clause* clauseNotA = createClause(1);
    addLiteralToClause(clauseNotA, litNotA);
    
    addClauseToFormula(unsatFormula, clauseA);
    addClauseToFormula(unsatFormula, clauseNotA);
    
    char* unsatStr = formulaToString(unsatFormula);
    printf("Unsatisfiable formula: %s\n", unsatStr);
    free(unsatStr);
    
    bool isUnsat = isUnsatisfiable(unsatFormula);
    printf("Is formula unsatisfiable? %s\n", isUnsat ? "Yes" : "No");
    
    // Test 7: Create a satisfiable formula
    printf("\nTest 7: Satisfiable Formula\n");
    // Create a satisfiable formula: (A ∨ B) ∧ (A ∨ !B)
    Formula* satFormula = createFormula(10);
    Clause* clauseAOrB = createClause(2);
    addLiteralToClause(clauseAOrB, litA);
    addLiteralToClause(clauseAOrB, litB);
    
    Clause* clauseAOrNotB = createClause(2);
    addLiteralToClause(clauseAOrNotB, litA);
    Literal* litNotB = createLiteral("B", true);
    addLiteralToClause(clauseAOrNotB, litNotB);
    
    addClauseToFormula(satFormula, clauseAOrB);
    addClauseToFormula(satFormula, clauseAOrNotB);
    
    char* satStr = formulaToString(satFormula);
    printf("Satisfiable formula: %s\n", satStr);
    free(satStr);
    
    bool isSat = !isUnsatisfiable(satFormula);
    printf("Is formula satisfiable? %s\n", isSat ? "Yes" : "No");
    
    // Test 8: Write to file and read from file
    printf("\nTest 8: File I/O\n");
    const char* filename = "formula.txt";
    printf("Writing formula to %s\n", filename);
    bool written = writeFormulaToFile(formula, filename);
    printf("Write successful: %s\n", written ? "Yes" : "No");
    
    printf("Reading formula from %s\n", filename);
    Formula* readFormula = parseFormulaFromFile(filename);
    if (readFormula != NULL) {
        char* readStr = formulaToString(readFormula);
        printf("Read formula: %s\n", readStr);
        free(readStr);
        freeFormula(readFormula);
    } else {
        printf("Failed to read formula from file\n");
    }
    
    // Cleanup
    freeLiteral(litA);
    freeLiteral(litNotA);
    freeLiteral(litB);
    freeLiteral(litNotB);
    freeClause(clause1);
    freeClause(clause2);
    freeClause(clauseA);
    freeClause(clauseNotA);
    freeClause(clauseAOrB);
    freeClause(clauseAOrNotB);
    freeFormula(formula);
    freeFormula(unsatFormula);
    freeFormula(satFormula);
    
    printf("\nAll tests completed!\n");
    
    return 0;
}