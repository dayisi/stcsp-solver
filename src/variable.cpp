#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include "util.h"
#include "node.h"
#include "variable.h"
#include "solver.h"
#include "y.tab.h"
using namespace std;

Variable *variableNew(Solver *solver, char *name, vector<int>domain) {
    int i;
    Variable *var = (Variable *)myMalloc(sizeof(Variable));
    var->solver = solver;
    var->name = strdup(name);
	/*
	if(!domain_node || domain_node->token != LIST){
		myLog(LOG_ERROR, "Invalid domain in variable %s\n",name);
	}
	var->domain=vector<int>();
    var->domain.push_back(domain_node->num1);
	Node *temp = domain_node;
	while(temp->left!=NULL && temp->left->token == LIST){
		temp=temp->left;
		var->domain.push_back(temp->num1);
	}	
	*/
	//delete duplicate value in domain
/*	
	int size = domain.size();
	//vector<int>temp_dm = domain;
	if(size >= 1){
		for(int i = 0; i < size; i++){
			for(int j = size-1; j > i; j--){
				if(domain.at(j) == domain.at(i)){
					domain.erase(domain.begin()+j);
					size --;
				}
			}
		}
	}
*/
	bool temp = deleteDup(domain);
	var->domain = domain;
    var->prevValue = 0;
	cout<<"var name "<<var->name<<", var domain: ";
	for(int i = 0; i < var->domain.size(); i++){
		cout<<var->domain.at(i)<<" ";
	}
	cout<<endl;

	/*
    var->currLB = (int *)myMalloc(sizeof(int) * solver->prefixK);
    var->currUB = (int *)myMalloc(sizeof(int) * solver->prefixK);
    for (i = 0; i < solver->prefixK; i++) {
        var->currLB[i] = lb;
        var->currUB[i] = ub;
    }
	*/
	var->currDM = (vector<int> *)myMalloc(sizeof(vector<int>) * solver->prefixK);
	for(i=0; i < solver->prefixK; i++){
		var->currDM[i].reserve(var->domain.size());
		var->currDM[i]=var->domain;
	}	

    var->propagateTimestamp = 0;
    var->propagateValue = 0;
    var->isSignature = 0;
    var->isUntil = 0;

    var->constraints = new vector<Constraint *>();
    var->numConstr = 0;

    //myLog(LOG_TRACE, "var %s : [ %d, %d ];\n", name, lb, ub);
	myLog(LOG_TRACE, "var %s : { ",name);
	for(i = 0; i<var->domain.size()-1;i++){
		myLog(LOG_TRACE," %d,",var->domain[i]);
	}
	myLog(LOG_TRACE, " %d };\n",var->domain.back());
    return var;
}

void variableFree(Variable *var) {
    myFree(var->name);
	//myFree(var->domain);
    constraintQueueFree(var->constraints);
	//myFree(var->currLB);
    //myFree(var->currUB);
	//for(int i = 0; i< var->solver->prefixK; i++){
//		free(var->currDM[i]);
//	}
    myFree(var->currDM);
    myFree(var);
}

// Search only the lower half of the current domain of the variable.
void variableSplitLower(Variable *var) {
    /*backup(&var->currUB[0]);
    var->currUB[0] = var->currLB[0] + ((var->currUB[0] - var->currLB[0]) / 2);
    myLog(LOG_NOTICE, "  Lower: %s [%d, %d]\n", var->name, var->currLB, var->currUB);
    */
	backup_dm(&var->currDM[0]);
	int size = var->currDM[0].size();
	/*
		vector<int> temp = vector<int>();
		for(int i = 0; i < size/2; i++){
			temp.push_back(var->currDM[0].at(i));
		}
		var->currDM[0] = temp;
	*/
	for(int i = 0; i < size/2; i++){
		var->currDM[0].pop_back();
	}
}

// Search only the upper half of the current domain of the variable.
void variableSplitUpper(Variable *var) {
	/*
    backup(&var->currLB[0]);
    var->currLB[0] = var->currLB[0] + ((var->currUB[0] - var->currLB[0]) / 2) + 1;
    myLog(LOG_NOTICE, "  Upper: %s [%d, %d]\n", var->name, var->currLB, var->currUB);
    */
	backup_dm(&var->currDM[0]);
	int size = var->currDM[0].size();
	vector<int>temp_dm = vector<int> ();
	for(int i = 0; i < size / 2; i++){
		temp_dm.push_back(var->currDM[0].back());
		var->currDM[0].pop_back();
	}	
	var->currDM[0] = temp_dm;
}

/*
 // Not used before changing domain into vector structure, also not appliable after the changing. 
void variableSetLB(Variable *var, int lb) {
    backup(&var->currLB[0]);
    var->currLB[0] = lb;
    //myLog(LOG_INFO, "  %s set lb to %d\n", var->name, lb);
}

void variableSetUB(Variable *var, int ub) {
    backup(&var->currUB[0]);
    var->currUB[0] = ub;
    //myLog(LOG_INFO, "  %s set ub to %d\n", var->name, ub);
}


void variableSetLBAt(Variable *var, int lb, int i) {
    backup(&var->currLB[i]);
    var->currLB[i] = lb;
    //myLog(LOG_INFO, "  %s set lb to %d\n", var->name, lb);
}

void variableSetUBAt(Variable *var, int ub, int i) {
    backup(&var->currUB[i]);
    var->currUB[i] = ub;
    //myLog(LOG_INFO, "  %s set ub to %d\n", var->name, ub);
}
*/

// Shifts a variable by one time point.
void variableAdvanceOneTimeStep(Solver *solver, Variable *var) {
    backup(&var->prevValue);
    for (int i = 0; i < solver->prefixK; i++){
        //backup(&var->currLB[i]);
        //backup(&var->currUB[i]);
		backup_dm(&var->currDM[i]);
    }

    var->prevValue = variableGetValue(var);
    for (int i = 0; i < solver->prefixK - 1; i++){
        //var->currLB[i] = var->currLB[i + 1];
        //var->currUB[i] = var->currUB[i + 1];
		var->currDM[i]=var->currDM[i+1];
    }
    //var->currLB[solver->prefixK - 1] = var->lb;
    //var->currUB[solver->prefixK - 1] = var->ub;
	var->currDM[solver->prefixK -1] = var->domain;
}

VariableQueue *variableQueueNew() {
    VariableQueue *queue = new vector<Variable *>();
    return queue;
}

void variableQueueFree(VariableQueue *queue) {
    /*Variable *var;
    var = queue->head;
    while (var != NULL) {
        variableFree(var);
        var = var->next;
    }*/
    myFree(queue);
    //delete queue;
}

void variableQueuePush(VariableQueue *queue, Variable *var) {
    queue->push_back(var);
}

bool variableQueueFind(VariableQueue *queue, Variable *var) {
    int size = queue->size();
    bool found = false;
    for (int c = 0; !found && c < size; c++) {
        if (var == (*queue)[c]) {
            found = true;
        }
    }
    return found;
}

void variablePrint(Variable *var) {
    if (var->isSignature) {
        myLog(LOG_TRACE, "Variable %s [S]\n", var->name);
    } else {
        myLog(LOG_TRACE, "Variable %s\n", var->name);
    }
    myLog(LOG_TRACE, "Involved in constraints: ");
    int size = var->numConstr;
    for (int i = 0; i < size-1; i++) {
        myLog(LOG_TRACE, "%d ", (*(var->constraints))[i]->id);
    }
    if (size > 0) {
        myLog(LOG_TRACE, "%d ", (*(var->constraints))[size-1]->id);
    }
    myLog(LOG_TRACE, "\n\n");
}


Array *arrayNew(struct Solver * solver, char *name, vector<int> elements){
    myLog(LOG_TRACE, "arrayNew\n");
    Array * arr = (Array*)myMalloc(sizeof(Array));
    arr->solver = solver;
    arr->name = strdup(name);
    if(elements.size() == 0){
        myLog(LOG_ERROR, "Invalid size 0 in variable %s\n", name);
        exit(1);
    }

    arr->elements = elements;
    arr->size = elements.size();

    myLog(LOG_TRACE, "arr %s, size:%d, elements: {", name, elements.size());
    for(int i = 0; i != arr->size; i++){
        myLog(LOG_TRACE, " %d", elements[i]);
    }
    myLog(LOG_TRACE, "};\n");
    return arr;
}
void variableFree(Array *arr){
    free(arr->name);
    myFree(arr);
}
ArrayQueue *arrayQueueNew(){
    ArrayQueue * queue = new vector<Array *>();
    return queue;
}
void arrayQueueFree(ArrayQueue *queue){
    delete queue;
}
bool arrayQueueFind(ArrayQueue *queue, Array *arr){
    int size = queue->size();
    bool found = false;
    for(int c = 0; !found && c < size; c++) {
        if(arr == (*queue)[c]) {
            found = true;
        }
    }
    return found;
}
void arrayQueuePush(ArrayQueue *queue, Array *arr){
    queue->push_back(arr);
}
