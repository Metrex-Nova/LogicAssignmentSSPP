#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Node structure for parse tree
typedef struct Node {
    char value;
    struct Node* left;
    struct Node* right;
} Node;

// Structure to store truth values
typedef struct {
    char variable;
    int value; // 0 for false, 1 for true
} TruthAssignment;

// Structure for DIMACS clauses
typedef struct {
    int* literals;  // Array of literals (positive or negative integers)
    int size;       // Number of literals in clause
} Clause;

// Structure for DIMACS CNF formula
typedef struct {
    Clause* clauses;
    int numClauses;
    int numVars;
} DIMACSFormula;

// Variable mapping structure (char <-> int)
typedef struct {
    char charVar;
    int intVar;
} VarMapping;

// Global variable mapping
VarMapping varMap[100];
int varMapSize = 0;

// Create a new node
Node* createNode(char value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->value = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Check if character is an operator
int isOperator(char c) {
    return (c == '~' || c == '+' || c == '*' || c == '>');
}

// Check if character is a binary operator
int isBinaryOperator(char c) {
    return (c == '+' || c == '*' || c == '>');
}

// Global index for parsing
int index_pos = 0;

// Forward declarations
Node* parseExpression(char* formula);
Node* parseOperand(char* formula);
void freeTree(Node* root);
void extractLiterals(Node* clause, int* literals, int* litCount);

// Parse operand (variable or subexpression)
Node* parseOperand(char* formula) {
    if (index_pos >= strlen(formula)) {
        return NULL;
    }
    
    if (formula[index_pos] == '(') {
        return parseExpression(formula);
    }
    
    if (formula[index_pos] == '~') {
        return parseExpression(formula);
    }
    
    char var = formula[index_pos];
    index_pos++;
    return createNode(var);
}

// Parse expression recursively
Node* parseExpression(char* formula) {
    if (index_pos < strlen(formula) && formula[index_pos] == '(') {
        index_pos++;
    }
    
    if (index_pos < strlen(formula) && formula[index_pos] == '~') {
        index_pos++;
        Node* operatorNode = createNode('~');
        operatorNode->left = parseExpression(formula);
        
        if (index_pos < strlen(formula) && formula[index_pos] == ')') {
            index_pos++;
        }
        
        return operatorNode;
    }
    
    Node* left = parseOperand(formula);
    
    if (index_pos < strlen(formula) && isBinaryOperator(formula[index_pos])) {
        char operator = formula[index_pos];
        index_pos++;
        
        Node* right = parseOperand(formula);
        
        Node* operatorNode = createNode(operator);
        operatorNode->left = left;
        operatorNode->right = right;
        
        if (index_pos < strlen(formula) && formula[index_pos] == ')') {
            index_pos++;
        }
        
        return operatorNode;
    }
    
    if (index_pos < strlen(formula) && formula[index_pos] == ')') {
        index_pos++;
    }
    
    return left;
}

// Build parse tree from infix formula
Node* buildParseTree(char* infixFormula) {
    index_pos = 0;
    return parseExpression(infixFormula);
}

// Convert parse tree to prefix notation
void treeToPrefix(Node* root, char* result, int* resultIndex) {
    if (root == NULL) {
        return;
    }
    
    result[(*resultIndex)++] = root->value;
    result[(*resultIndex)++] = ' ';
    
    treeToPrefix(root->left, result, resultIndex);
    treeToPrefix(root->right, result, resultIndex);
}

// Convert infix to prefix
void infixToPrefix(char* infixFormula, char* prefixResult) {
    Node* root = buildParseTree(infixFormula);
    
    int resultIndex = 0;
    treeToPrefix(root, prefixResult, &resultIndex);
    
    if (resultIndex > 0 && prefixResult[resultIndex - 1] == ' ') {
        resultIndex--;
    }
    prefixResult[resultIndex] = '\0';
    
    freeTree(root);
}

// TASK 2: Build parse tree from prefix expression
Node* prefixToTree(char* prefix, int* index) {
    while (prefix[*index] == ' ') {
        (*index)++;
    }
    
    if (prefix[*index] == '\0') {
        return NULL;
    }
    
    char current = prefix[*index];
    (*index)++;
    
    Node* node = createNode(current);
    
    if (isOperator(current)) {
        node->left = prefixToTree(prefix, index);
        
        if (isBinaryOperator(current)) {
            node->right = prefixToTree(prefix, index);
        }
    }
    
    return node;
}

Node* buildTreeFromPrefix(char* prefix) {
    int index = 0;
    return prefixToTree(prefix, &index);
}

// TASK 3: In-order traversal to get infix expression
void inorderTraversal(Node* root) {
    if (root == NULL) return;
    
    if (isOperator(root->value)) {
        printf("(");
    }
    
    if (root->value == '~') {
        printf("~");
        inorderTraversal(root->left);
    } else {
        inorderTraversal(root->left);
        printf("%c", root->value);
        inorderTraversal(root->right);
    }
    
    if (isOperator(root->value)) {
        printf(")");
    }
}

// TASK 4: Calculate height of parse tree
int calculateHeight(Node* root) {
    if (root == NULL) {
        return -1;
    }
    
    int leftHeight = calculateHeight(root->left);
    int rightHeight = calculateHeight(root->right);
    
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// TASK 5: Evaluate truth value of formula
int getTruthValue(char variable, TruthAssignment* assignments, int numAssignments) {
    for (int i = 0; i < numAssignments; i++) {
        if (assignments[i].variable == variable) {
            return assignments[i].value;
        }
    }
    return -1;
}

int evaluateFormula(Node* root, TruthAssignment* assignments, int numAssignments) {
    if (root == NULL) {
        return -1;
    }
    
    if (!isOperator(root->value)) {
        return getTruthValue(root->value, assignments, numAssignments);
    }
    
    int leftVal = evaluateFormula(root->left, assignments, numAssignments);
    int rightVal = (root->right != NULL) ? evaluateFormula(root->right, assignments, numAssignments) : 0;
    
    switch (root->value) {
        case '~':
            return !leftVal;
        case '+':
            return leftVal || rightVal;
        case '*':
            return leftVal && rightVal;
        case '>':
            return !leftVal || rightVal;
        default:
            return -1;
    }
}

// TASK 6: CNF Conversion Helper Functions

Node* cloneTree(Node* root) {
    if (root == NULL) return NULL;
    
    Node* newNode = createNode(root->value);
    newNode->left = cloneTree(root->left);
    newNode->right = cloneTree(root->right);
    
    return newNode;
}

Node* eliminateImplications(Node* root) {
    if (root == NULL) return NULL;
    
    root->left = eliminateImplications(root->left);
    root->right = eliminateImplications(root->right);
    
    if (root->value == '>') {
        Node* notNode = createNode('~');
        notNode->left = root->left;
        
        Node* orNode = createNode('+');
        orNode->left = notNode;
        orNode->right = root->right;
        
        free(root);
        return orNode;
    }
    
    return root;
}

Node* moveNegationsInward(Node* root) {
    if (root == NULL) return NULL;
    
    if (root->value != '~') {
        root->left = moveNegationsInward(root->left);
        root->right = moveNegationsInward(root->right);
        return root;
    }
    
    if (root->left != NULL && root->left->value == '~') {
        Node* temp = root->left->left;
        free(root->left);
        free(root);
        return moveNegationsInward(temp);
    }
    
    if (root->left != NULL && root->left->value == '*') {
        Node* andNode = root->left;
        
        Node* notLeft = createNode('~');
        notLeft->left = andNode->left;
        
        Node* notRight = createNode('~');
        notRight->left = andNode->right;
        
        Node* orNode = createNode('+');
        orNode->left = moveNegationsInward(notLeft);
        orNode->right = moveNegationsInward(notRight);
        
        free(andNode);
        free(root);
        return orNode;
    }
    
    if (root->left != NULL && root->left->value == '+') {
        Node* orNode = root->left;
        
        Node* notLeft = createNode('~');
        notLeft->left = orNode->left;
        
        Node* notRight = createNode('~');
        notRight->left = orNode->right;
        
        Node* andNode = createNode('*');
        andNode->left = moveNegationsInward(notLeft);
        andNode->right = moveNegationsInward(notRight);
        
        free(orNode);
        free(root);
        return andNode;
    }
    
    return root;
}

Node* distributeOrOverAnd(Node* root) {
    if (root == NULL) return NULL;
    
    root->left = distributeOrOverAnd(root->left);
    root->right = distributeOrOverAnd(root->right);
    
    if (root->value == '+') {
        if (root->left != NULL && root->left->value == '*') {
            Node* andNode = root->left;
            Node* p = andNode->left;
            Node* q = andNode->right;
            Node* r = root->right;
            
            Node* or1 = createNode('+');
            or1->left = p;
            or1->right = cloneTree(r);
            
            Node* or2 = createNode('+');
            or2->left = q;
            or2->right = r;
            
            Node* newAnd = createNode('*');
            newAnd->left = distributeOrOverAnd(or1);
            newAnd->right = distributeOrOverAnd(or2);
            
            free(andNode);
            free(root);
            return newAnd;
        }
        
        if (root->right != NULL && root->right->value == '*') {
            Node* andNode = root->right;
            Node* q = andNode->left;
            Node* r = andNode->right;
            Node* p = root->left;
            
            Node* or1 = createNode('+');
            or1->left = cloneTree(p);
            or1->right = q;
            
            Node* or2 = createNode('+');
            or2->left = p;
            or2->right = r;
            
            Node* newAnd = createNode('*');
            newAnd->left = distributeOrOverAnd(or1);
            newAnd->right = distributeOrOverAnd(or2);
            
            free(andNode);
            free(root);
            return newAnd;
        }
    }
    
    return root;
}

Node* convertToCNF(Node* root) {
    if (root == NULL) return NULL;
    
    root = eliminateImplications(root);
    root = moveNegationsInward(root);
    root = distributeOrOverAnd(root);
    
    return root;
}

// TASK 7: Validity Check
void extractClauses(Node* root, Node** clauses, int* count, int maxClauses) {
    if (root == NULL || *count >= maxClauses) return;
    
    if (root->value == '*') {
        extractClauses(root->left, clauses, count, maxClauses);
        extractClauses(root->right, clauses, count, maxClauses);
    } else {
        clauses[(*count)++] = root;
    }
}

bool isValidCNF(Node* cnfRoot) {
    if (cnfRoot == NULL) return false;

    Node* clauses[100];
    int clauseCount = 0;
    extractClauses(cnfRoot, clauses, &clauseCount, 100);

    if (clauseCount == 0) return false;

    for (int i = 0; i < clauseCount; i++) {
        int literals[50];
        int litCount = 0;
        varMapSize = 0; // Reset mapping for each clause
        extractLiterals(clauses[i], literals, &litCount);

        // Check for complementary pair
        bool foundComplementary = false;
        for (int j = 0; j < litCount; j++) {
            for (int k = 0; k < litCount; k++) {
                if (j != k && literals[j] == -literals[k]) {
                    foundComplementary = true;
                    break;
                }
            }
            if (foundComplementary) break;
        }
        if (!foundComplementary) {
            return false;
        }
    }
    return true;
}

// ========== DIMACS FORMAT SUPPORT ==========

// Get or create integer variable for a character variable
int getIntVar(char charVar) {
    for (int i = 0; i < varMapSize; i++) {
        if (varMap[i].charVar == charVar) {
            return varMap[i].intVar;
        }
    }
    
    // Create new mapping
    varMap[varMapSize].charVar = charVar;
    varMap[varMapSize].intVar = varMapSize + 1;
    varMapSize++;
    
    return varMapSize;
}

// Get character variable from integer
char getCharVar(int intVar) {
    for (int i = 0; i < varMapSize; i++) {
        if (varMap[i].intVar == intVar) {
            return varMap[i].charVar;
        }
    }
    return '?';
}

// Extract literals from a clause (OR expression)
void extractLiterals(Node* clause, int* literals, int* litCount) {
    if (clause == NULL) return;
    
    // If it's an OR node, recursively extract from both sides
    if (clause->value == '+') {
        extractLiterals(clause->left, literals, litCount);
        extractLiterals(clause->right, literals, litCount);
    }
    // If it's a negation
    else if (clause->value == '~') {
        if (clause->left != NULL && !isOperator(clause->left->value)) {
            int varNum = getIntVar(clause->left->value);
            literals[(*litCount)++] = -varNum;
        }
    }
    // If it's a literal (variable)
    else if (!isOperator(clause->value)) {
        int varNum = getIntVar(clause->value);
        literals[(*litCount)++] = varNum;
    }
}

// Convert parse tree (CNF) to DIMACS format
DIMACSFormula* treeToDIMACS(Node* cnfRoot) {
    DIMACSFormula* formula = (DIMACSFormula*)malloc(sizeof(DIMACSFormula));
    
    // Reset variable mapping
    varMapSize = 0;
    
    // Extract all clauses
    Node* clauseNodes[100];
    int clauseCount = 0;
    extractClauses(cnfRoot, clauseNodes, &clauseCount, 100);
    
    // Allocate memory for clauses
    formula->clauses = (Clause*)malloc(clauseCount * sizeof(Clause));
    formula->numClauses = clauseCount;
    
    // Convert each clause
    for (int i = 0; i < clauseCount; i++) {
        int literals[50];
        int litCount = 0;
        
        extractLiterals(clauseNodes[i], literals, &litCount);
        
        formula->clauses[i].literals = (int*)malloc(litCount * sizeof(int));
        formula->clauses[i].size = litCount;
        
        for (int j = 0; j < litCount; j++) {
            formula->clauses[i].literals[j] = literals[j];
        }
    }
    
    formula->numVars = varMapSize;
    
    return formula;
}

// Print DIMACS format
void printDIMACS(DIMACSFormula* formula) {
    printf("c DIMACS CNF Format\n");
    printf("c Generated from parse tree\n");
    printf("p cnf %d %d\n", formula->numVars, formula->numClauses);
    
    for (int i = 0; i < formula->numClauses; i++) {
        for (int j = 0; j < formula->clauses[i].size; j++) {
            printf("%d ", formula->clauses[i].literals[j]);
        }
        printf("0\n");
    }
}

// Save DIMACS to file
void saveDIMACS(DIMACSFormula* formula, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return;
    }
    
    fprintf(file, "c DIMACS CNF Format\n");
    fprintf(file, "c Generated from parse tree\n");
    fprintf(file, "p cnf %d %d\n", formula->numVars, formula->numClauses);
    
    for (int i = 0; i < formula->numClauses; i++) {
        for (int j = 0; j < formula->clauses[i].size; j++) {
            fprintf(file, "%d ", formula->clauses[i].literals[j]);
        }
        fprintf(file, "0\n");
    }
    
    fclose(file);
    printf("DIMACS formula saved to %s\n", filename);
}

// Read DIMACS from file
DIMACSFormula* readDIMACS(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    DIMACSFormula* formula = (DIMACSFormula*)malloc(sizeof(DIMACSFormula));
    char line[1024];
    int numVars = 0, numClauses = 0;
    int clauseIndex = 0;
    
    // Read header
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue; // Comment line
        
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &numVars, &numClauses);
            formula->numVars = numVars;
            formula->numClauses = numClauses;
            formula->clauses = (Clause*)malloc(numClauses * sizeof(Clause));
            break;
        }
    }
    
    // Read clauses
    while (fgets(line, sizeof(line), file) && clauseIndex < numClauses) {
        if (line[0] == 'c' || line[0] == '%' || line[0] == '0') continue;
        
        int literals[100];
        int litCount = 0;
        
        char* token = strtok(line, " \t\n");
        while (token != NULL) {
            int lit = atoi(token);
            if (lit == 0) break;
            literals[litCount++] = lit;
            token = strtok(NULL, " \t\n");
        }
        
        if (litCount > 0) {
            formula->clauses[clauseIndex].literals = (int*)malloc(litCount * sizeof(int));
            formula->clauses[clauseIndex].size = litCount;
            
            for (int i = 0; i < litCount; i++) {
                formula->clauses[clauseIndex].literals[i] = literals[i];
            }
            
            clauseIndex++;
        }
    }
    
    fclose(file);
    printf("DIMACS formula loaded: %d variables, %d clauses\n", numVars, clauseIndex);
    return formula;
}

// Evaluate DIMACS formula with assignment
bool evaluateDIMACS(DIMACSFormula* formula, int* assignment) {
    // Formula is true if all clauses are true
    for (int i = 0; i < formula->numClauses; i++) {
        bool clauseTrue = false;
        
        // Clause is true if at least one literal is true
        for (int j = 0; j < formula->clauses[i].size; j++) {
            int lit = formula->clauses[i].literals[j];
            int var = abs(lit);
            bool value = assignment[var];
            
            // Check if literal is satisfied
            if ((lit > 0 && value) || (lit < 0 && !value)) {
                clauseTrue = true;
                break;
            }
        }
        
        if (!clauseTrue) {
            return false; // Formula is false
        }
    }
    
    return true; // All clauses satisfied
}

// Print variable mapping
void printVarMapping() {
    printf("\nVariable Mapping:\n");
    printf("Char -> Integer\n");
    for (int i = 0; i < varMapSize; i++) {
        printf("  %c   ->   %d\n", varMap[i].charVar, varMap[i].intVar);
    }
}

// Free DIMACS formula
void freeDIMACS(DIMACSFormula* formula) {
    if (formula == NULL) return;
    
    for (int i = 0; i < formula->numClauses; i++) {
        free(formula->clauses[i].literals);
    }
    free(formula->clauses);
    free(formula);
}

// Free the tree
void freeTree(Node* root) {
    if (root == NULL) return;
    
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

// Print functions
void printPreorder(Node* root) {
    if (root == NULL) return;
    
    printf("%c ", root->value);
    printPreorder(root->left);
    printPreorder(root->right);
}

// Print tree visually (sideways)
void printTree(Node* root, int space) {
    if (root == NULL) return;
    space += 4;
    printTree(root->right, space);
    printf("\n");
    for (int i = 4; i < space; i++)
        printf(" ");
    printf("%c\n", root->value);
    printTree(root->left, space);
}

// Print tree in directory-like structure (ASCII style)
void printTreeRec(Node* node, char* connector, char* indent);
void printTreeAscii(Node* root) {
    if (root == NULL) return;
    printf("%c\n", root->value);
    printTreeRec(root->left, "|-- ", root->right != NULL ? "|   " : "    ");
    printTreeRec(root->right, "`-- ", "    ");
}

void printTreeRec(Node* node, char* connector, char* indent) {
    if (node == NULL) return;
    printf("%s%s%c\n", indent, connector, node->value);
    char newIndent[100];
    strcpy(newIndent, indent);
    if (node->left != NULL || node->right != NULL) {
        strcat(newIndent, "|   ");
    } else {
        strcat(newIndent, "    ");
    }
    printTreeRec(node->left, "|-- ", newIndent);
    printTreeRec(node->right, "`-- ", newIndent);
}

// Print tree level by level (root at top, indented for binary tree structure)
void printTreeRooted(Node* root, int level) {
    if (root == NULL) return;
    for (int i = 0; i < level; i++) printf("    ");
    printf("%c\n", root->value);
    printTreeRooted(root->left, level + 1);
    printTreeRooted(root->right, level + 1);
}

// Collect unique variables from the tree
void collectVariables(Node* root, char* vars, int* count) {
    if (root == NULL) return;
    if (!isOperator(root->value)) {
        // Check if already present
        for (int i = 0; i < *count; i++) {
            if (vars[i] == root->value) return;
        }
        vars[*count] = root->value;
        (*count)++;
    }
    collectVariables(root->left, vars, count);
    collectVariables(root->right, vars, count);
}

// Menu-driven main function
int main() {
    char formula[200];
    char prefix[200];
    char filename[100];
    int choice = -1;
    Node* tree = NULL;
    DIMACSFormula* dimacsFormula = NULL;
    
    printf("=============================================\n");
    printf("  Propositional Logic Parser with DIMACS\n");
    printf("=============================================\n");
    printf("Operators: ~ (NOT), + (OR), * (AND), > (IMPLICATION)\n\n");
    
    while (1) {
        printf("\n=== MENU ===\n");
        printf("1.  Convert Infix to Prefix\n");
        printf("2.  Build Tree from Prefix\n");
        printf("3.  Display Infix (In-order Traversal)\n");
        printf("4.  Calculate Tree Height\n");
        printf("5.  Evaluate Formula\n");
        printf("6.  Convert to CNF\n");
        printf("7.  Check Validity of CNF\n");
        printf("8.  Convert CNF to DIMACS Format\n");
        printf("9.  Save DIMACS to File\n");
        printf("10. Load DIMACS from File\n");
        printf("11. Display DIMACS Formula\n");
        printf("12. Evaluate DIMACS Formula\n");
        printf("13. Show Variable Mapping\n");
        printf("14. Run Demo\n");
        printf("15. Display Parse Tree from Infix\n");
        printf("0.  Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();
        
        switch (choice) {
            case 1:
                printf("Enter infix formula (fully parenthesized): ");
                fgets(formula, sizeof(formula), stdin);
                formula[strcspn(formula, "\n")] = 0;
                
                infixToPrefix(formula, prefix);
                printf("Prefix: %s\n", prefix);
                break;
                
            case 2:
                printf("Enter prefix formula: ");
                fgets(prefix, sizeof(prefix), stdin);
                prefix[strcspn(prefix, "\n")] = 0;

                if (tree != NULL) freeTree(tree);
                tree = buildTreeFromPrefix(prefix);
                printf("Parse tree built successfully!\n");
                printf("Tree structure (rooted binary):\n");
                printTreeRooted(tree, 0); // <-- Use new rooted display
                break;
                
            case 3:
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    printf("Infix expression: ");
                    inorderTraversal(tree);
                    printf("\n");
                }
                break;
                
            case 4:
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    printf("Tree height: %d\n", calculateHeight(tree));
                }
                break;
                
            case 5: {
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    char vars[26];
                    int varCount = 0;
                    collectVariables(tree, vars, &varCount);

                    TruthAssignment assignments[26];
                    printf("Detected %d variable(s): ", varCount);
                    for (int i = 0; i < varCount; i++) {
                        printf("%c ", vars[i]);
                    }
                    printf("\n");

                    for (int i = 0; i < varCount; i++) {
                        assignments[i].variable = vars[i];
                        printf("Truth value for %c (0/1): ", vars[i]);
                        scanf("%d", &assignments[i].value);
                    }

                    int result = evaluateFormula(tree, assignments, varCount);
                    printf("Formula evaluates to: %s\n", result ? "TRUE" : "FALSE");
                }
                break;
            }
                
            case 6:
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    Node* cnfTree = convertToCNF(cloneTree(tree));
                    printf("CNF form: ");
                    inorderTraversal(cnfTree);
                    printf("\n");
                    
                    printf("Store as current tree? (y/n): ");
                    char ch;
                    scanf(" %c", &ch);
                    if (ch == 'y' || ch == 'Y') {
                        freeTree(tree);
                        tree = cnfTree;
                    } else {
                        freeTree(cnfTree);
                    }
                }
                break;
                
            case 7:
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    Node* cnfTree = convertToCNF(cloneTree(tree));
                    bool valid = isValidCNF(cnfTree);
                    printf("Formula is %s\n", valid ? "VALID" : "NOT VALID (or cannot determine)");
                    freeTree(cnfTree);
                }
                break;
                
            case 8:
                if (tree == NULL) {
                    printf("No tree loaded. Use option 2 first.\n");
                } else {
                    // Ensure tree is in CNF
                    Node* cnfTree = convertToCNF(cloneTree(tree));
                    
                    if (dimacsFormula != NULL) {
                        freeDIMACS(dimacsFormula);
                    }
                    
                    dimacsFormula = treeToDIMACS(cnfTree);
                    printf("\nDIMACS Format:\n");
                    printDIMACS(dimacsFormula);
                    printVarMapping();
                    
                    freeTree(cnfTree);
                }
                break;
                
            case 9:
                if (dimacsFormula == NULL) {
                    printf("No DIMACS formula available. Use option 8 first.\n");
                } else {
                    printf("Enter filename: ");
                    fgets(filename, sizeof(filename), stdin);
                    filename[strcspn(filename, "\n")] = 0;
                    
                    saveDIMACS(dimacsFormula, filename);
                }
                break;
                
            case 10:
                printf("Enter filename: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                
                if (dimacsFormula != NULL) {
                    freeDIMACS(dimacsFormula);
                }
                
                dimacsFormula = readDIMACS(filename);
                break;
                
            case 11:
                if (dimacsFormula == NULL) {
                    printf("No DIMACS formula loaded.\n");
                } else {
                    printf("\nDIMACS Formula:\n");
                    printDIMACS(dimacsFormula);
                }
                break;
                
            case 12:
                if (dimacsFormula == NULL) {
                    printf("No DIMACS formula loaded.\n");
                } else {
                    int* assignment = (int*)calloc(dimacsFormula->numVars + 1, sizeof(int));
                    
                    printf("Enter truth assignments (0/1) for %d variables:\n", dimacsFormula->numVars);
                    for (int i = 1; i <= dimacsFormula->numVars; i++) {
                        printf("Variable %d: ", i);
                        scanf("%d", &assignment[i]);
                    }
                    
                    bool result = evaluateDIMACS(dimacsFormula, assignment);
                    printf("Formula evaluates to: %s\n", result ? "TRUE (SAT)" : "FALSE (UNSAT)");
                    
                    free(assignment);
                }
                break;
                
            case 13:
                if (varMapSize == 0) {
                    printf("No variable mapping available. Convert to DIMACS first.\n");
                } else {
                    printVarMapping();
                }
                break;
                
            case 14:
                printf("\n=== DEMO: DIMACS Format ===\n\n");
                
                printf("Example 1: (p+q)\n");
                strcpy(formula, "(p+q)");
                index_pos = 0;
                Node* demo1 = buildParseTree(formula);
                printf("  Original: ");
                inorderTraversal(demo1);
                printf("\n");
                
                DIMACSFormula* dimacs1 = treeToDIMACS(demo1);
                printf("  DIMACS:\n");
                printDIMACS(dimacs1);
                printVarMapping();
                freeDIMACS(dimacs1);
                freeTree(demo1);
                varMapSize = 0;
                
                printf("\nExample 2: ((p>q)*(~r))\n");
                strcpy(formula, "((p>q)*(~r))");
                index_pos = 0;
                Node* demo2 = buildParseTree(formula);
                printf("  Original: ");
                inorderTraversal(demo2);
                printf("\n");
                
                Node* cnf2 = convertToCNF(cloneTree(demo2));
                printf("  CNF: ");
                inorderTraversal(cnf2);
                printf("\n");
                
                DIMACSFormula* dimacs2 = treeToDIMACS(cnf2);
                printf("  DIMACS:\n");
                printDIMACS(dimacs2);
                printVarMapping();
                freeDIMACS(dimacs2);
                freeTree(cnf2);
                freeTree(demo2);
                varMapSize = 0;
                
                printf("\nExample 3: SAT 2002 Compatible\n");
                printf("Creating a sample DIMACS file...\n");
                
                strcpy(formula, "((p+q)*(~p+r))");
                index_pos = 0;
                Node* demo3 = buildParseTree(formula);
                Node* cnf3 = convertToCNF(cloneTree(demo3));
                
                DIMACSFormula* dimacs3 = treeToDIMACS(cnf3);
                saveDIMACS(dimacs3, "sample.cnf");
                
                printf("\nNow reading it back:\n");
                DIMACSFormula* loaded = readDIMACS("sample.cnf");
                if (loaded != NULL) {
                    printDIMACS(loaded);
                    freeDIMACS(loaded);
                }
                
                freeDIMACS(dimacs3);
                freeTree(cnf3);
                freeTree(demo3);
                varMapSize = 0;
                
                break;

            case 15:
                printf("Enter infix formula (fully parenthesized): ");
                fgets(formula, sizeof(formula), stdin);
                formula[strcspn(formula, "\n")] = 0;

                if (tree != NULL) freeTree(tree);
                tree = buildParseTree(formula);
                printf("Parse tree built successfully!\n");
                printf("Tree structure (directory like):\n");
                printTreeAscii(tree);
                break;

            case 0:
                if (tree != NULL) freeTree(tree);
                if (dimacsFormula != NULL) freeDIMACS(dimacsFormula);
                printf("Exiting...\n");
                return 0;
                
            default:
                printf("Invalid choice!\n");
        }
    }
    
    return 0;
}