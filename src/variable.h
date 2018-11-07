#ifndef __VARIABLE_H
#define __VARIABLE_H

#include <vector>
#include "constraint.h"
using namespace std;

struct Solver;

struct Variable {
    Solver *solver; // The solver that owns this variable.
    char *name; // Name of the variable in the constraint program. char * because lexer is in C.
    //int lb; // Lower bound.
    //int ub; // Upper boudn.
	vector<int> domain; // domain of variable.

    ConstraintQueue *constraints; // List of constraints the variable is involved in.
    int numConstr; // Number of such constraints.

    //int *currLB; // Array of k lower bounds, for enforcing prefix-k consistency.
    //int *currUB; // Array of k upper bounds, for enforcing prefix-k consistency.
	vector< vector<int> > currDM; //vector of k domains, for enforcing prefix-k consistency.
    int prevValue; // Value assigned to previous time point.
    int isSignature; // Whether the variable is in the signature of the St-CSP.
    int isUntil; // Whether the variable is in the until signature of the St-CSP.
    int propagateValue; // For use when enforcing the prefix-k consistency/GAC.
    int propagateTimestamp; // Not in use?
};

typedef vector<Variable *> VariableQueue;

//Variable *variableNew(struct Solver *solver, char *name, Node *domain_node);
Variable *variableNew(struct Solver *solver, char *name, vector<int> domain);
void variableFree(Variable *var);

static inline int variableGetValue(Variable *var) {
    //if (var->currLB[0] != var->currUB[0]) {
	if(var->currDM[0].size() > 1){
		int temp_val = var->currDM[0].at(0);
		for(int i = 0; i < var->currDM[0].size(); i++){
        	if(temp_val != var->currDM[0].at(i)){
				myLog(LOG_WARNING, "%s is not bounded yet.\n", var->name);
				break;
			}
    	}
	}
    return var->currDM[0].at(0);
}

void variableSplitLower(Variable *var);
void variableSplitUpper(Variable *var);

/*
void variableSetLB(Variable *var, int lb);
void variableSetUB(Variable *var, int ub);
void variableSetLBAt(Variable *var, int lb, int i);
void variableSetUBAt(Variable *var, int ub, int i);
*/
void variablePrint(Variable *var);
void variableAdvanceOneTimeStep(Solver *solver, Variable *var);

VariableQueue *variableQueueNew();
void variableQueueFree(VariableQueue *queue);
bool variableQueueFind(VariableQueue *queue, Variable *var);
void variableQueuePush(VariableQueue *queue, Variable *var);

struct Array {
    Solver *solver;
    char *name;
    int size;
    vector<int> elements;
};

typedef vector<Array *> ArrayQueue;
Array *arrayNew(struct Solver * solver, char *name, vector<int> elements);
void variableFree(Array *arr);

ArrayQueue *arrayQueueNew();
void arrayQueueFree(ArrayQueue *queue);
bool arrayQueueFind(ArrayQueue *queue, Array *arr);
void arrayQueuePush(ArrayQueue *queue, Array *arr);

#endif
