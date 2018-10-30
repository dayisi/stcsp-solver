#ifndef __SOLVERALGORITHM_H
#define __SOLVERALGORITHM_H

#include "variable.h"
#include "solver.h"

ConstraintNode *constraintNormalise(Solver *solver, ConstraintNode *node, vector<int> &domain);

void solverAddConstr(Solver *solver, Node *node);

double solverSolve(Solver *solver, bool testing);

#endif
