#ifndef __SOLVERALGORITHM_H
#define __SOLVERALGORITHM_H

#include "variable.h"
#include "solver.h"



struct MyEdge{
    Variable *var; //an endpoint of the edge
    int num;       //another endpoint of the edge, the num must be in domain of var
    int direction; //if 1, direction is from var to num; if 0, direction is from num to var
    bool vital; //whether the edge bebongs to every max_matching
    bool used; //whether the edge belongs to some max_matching
};

MyEdge *newMyEdge(Variable* var, int num);



ConstraintNode *constraintNormalise(Solver *solver, ConstraintNode *node, vector<int> &domain);

void solverAddConstr(Solver *solver, Node *node);

double solverSolve(Solver *solver, bool testing);
#endif


