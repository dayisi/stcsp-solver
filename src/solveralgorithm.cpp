#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "util.h"
#include "token.h"
#include "node.h"
#include "variable.h"
#include "constraint.h"
#include "solver.h"
#include "solveralgorithm.h"
#include "y.tab.h"
#include "graph.h"

int test_num = 0;

//ConstraintNode *constraintNormalise(Solver *solver, ConstraintNode* constrNode, vector<int> &domain);
/* OmegaSolver */
void solverAddConstr(Solver *solver, Node *node) {
	vector<int> temp = vector<int>();
	Constraint *constr = constraintNew(solver, constraintNormalise(solver, constraintNodeParse(solver, node), temp));
    if (!constraintNodeTautology(constr->node)) {
        solverConstraintQueuePush(solver->constrQueue, constr, solver, true);
    } else {
        constraintFree(constr);
    }
	//cout<<"add constr "<<constr->id <<"succeed"<<endl;
}

/* OmegaSolver */
void solverAddConstrFirstEqFirst(Solver *solver, Variable *x, Variable *y) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL, NULL,
                                                  constraintNodeNewFirst(constraintNodeNewVar(x)),
                                                  constraintNodeNewFirst(constraintNodeNewVar(y))));
}

/* OmegaSolver */
void solverAddConstrVarEqNext(Solver *solver, Variable *x, Variable *y) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL, NULL,
                                constraintNodeNewVar(x),
                                constraintNodeNewNext(constraintNodeNewVar(y))));
}

/* OmegaSolver */
void solverAddConstrVarEqNode(Solver *solver, Variable *x, ConstraintNode *node) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL, NULL,
                                constraintNodeNewVar(x),
                                node));
}

void solverAddConstrVarUntilVar(Solver *solver, Variable * x, Variable *y) {
    solverAddConstrNode(solver, constraintNodeNew(UNTIL_CON, 0, NULL, NULL,
                                constraintNodeNewVar(x),
                                constraintNodeNewVar(y)));
}

void solverAddConstrVarEqAt(Solver *solver, Variable * x, Variable * y, int timepoint) {
    solverAddConstrNode(solver, constraintNodeNew(EQ_CON, 0, NULL, NULL,
                                constraintNodeNewVar(x),
                                constraintNodeNewAt(y, timepoint)));
}

/* OmegaSolver */
ConstraintNode *constraintNormalise(Solver *solver, ConstraintNode *node, vector<int> &domain) {
    ConstraintNode *constrNode = NULL;
    ConstraintNode *constrNodeLeft = NULL;
    ConstraintNode *constrNodeRight = NULL;
    Variable *x, *y, *z;
    //int myLB, myUB, myLB2, myUB2;
	vector<int> myDM, myDM2;
	myDM = vector<int>();
	myDM2 = vector<int>();

    if (node != NULL) {
        if (node->token != IDENTIFIER && node->token != CONSTANT && node->token != ARR_IDENTIFIER && node->token != LIST_ELEMENT) {
            myLog(LOG_DETAILED_TRACE, "Token: %s\n", tokenString(solver->tokenTable, node->token));
        } else if ( node->token == CONSTANT ) {
            myLog(LOG_DETAILED_TRACE, "Constant: %d\n", node->num);
        } else if ( node->token == IDENTIFIER) {
            myLog(LOG_DETAILED_TRACE, "Identifier: %s\n", node->var->name);
        } else if ( node->token == ARR_IDENTIFIER ) {
            myLog(LOG_DETAILED_TRACE, "ARR_Identifier: %s\n", node->array->name);
		} else if ( node->token == LIST_ELEMENT){
			myLog(LOG_DETAILED_TRACE, "LIST_ELEMENT: \n");
		}

        if (node->token == FIRST) {
            if (node->right->token == CONSTANT) {
				domain = vector<int>();	
				domain.push_back(node->right->num);
                constrNode = node->right;
                myFree(node);
            } else if (node->right->token == IDENTIFIER) {
				domain = node->right->var->domain;
                constrNode = node;
            } else if (node->right->token == FBY) {
                constrNodeRight = node->right->left;
                constraintNodeFree(node->right->right);
                myFree(node->right);
                node->right = constrNodeRight;
                constrNode = constraintNormalise(solver, node, domain);
            } else {
                constrNodeRight = constraintNormalise(solver, node->right, myDM);
                if (constrNodeRight->token == FIRST) {
                    ConstraintNode *temp = constrNodeRight->right;
                    myFree(constrNodeRight);
                    constrNodeRight = temp;
                }
                node->right = constrNodeRight;
                constrNode = node;
				domain = myDM;
            }
        } else if (node->token == NEXT) {
            if (node->right->token == CONSTANT) {
				vector<int> temp;
				temp.push_back(node->right->num);	
				domain = temp;
                constrNode = node->right;
                myFree(node);
            } else if (node->right->token == IDENTIFIER) {
				domain = node->right->var->domain;
                x = solverAuxVarNew(solver, NULL, domain);
                solverAddConstrVarEqNext(solver, x, node->right->var);
                myFree(node->right);
                myFree(node);
                constrNode = constraintNodeNew(IDENTIFIER, 0, x, NULL, NULL, NULL);
            } else if (node->right->token == FBY) {
                constrNode = node->right->right;
                constraintNodeFree(node->right->left);
                myFree(node->right);
                myFree(node);
				constrNode = constraintNormalise(solver, constrNode, domain);
            } else {
				constrNodeRight = constraintNormalise(solver, node->right, myDM);
                if (constrNodeRight->token == FIRST) {
                    myFree(node);
                    constrNode = constrNodeRight;
                } else if (constrNodeRight->token == CONSTANT || constrNodeRight->token == IDENTIFIER) {
                    node->right = constrNodeRight;
					constrNode = constraintNormalise(solver, node, myDM);
                } else {
					x = solverAuxVarNew(solver, NULL, myDM);
                    solverAddConstrVarEqNode(solver, x, constrNodeRight);
					y = solverAuxVarNew(solver, NULL, myDM);
                    solverAddConstrVarEqNext(solver, y, x);
                    constrNode = constraintNodeNew(IDENTIFIER, 0, y, NULL, NULL, NULL);
                    myFree(node);
                }
				domain = myDM;
            }
        } else if (node->token == FBY) {
			//cout<<"node token is FBY"<<endl;
            if (node->left->token == IDENTIFIER) {
				myDM = node->left->var->domain;
                y = node->left->var;
				//cout<<"node->left->var->name is "<<y->name<<endl;
                myFree(node->left);
            } else {
				//cout<<"for left node of FYB node"<<endl;
                constrNodeLeft = constraintNormalise(solver, node->left, myDM);
                y = solverAuxVarNew(solver, NULL, myDM);
                solverAddConstrVarEqNode(solver, y, constrNodeLeft);
            }
            if (node->right->token == IDENTIFIER) {
                myDM2 = node->right->var->domain;
                z = node->right->var;
				//cout<<"node->right->var->name is "<<z->name<<endl;
                myFree(node->right);
            } else {
				//cout<<"for right node of FBY node"<<endl;
                constrNodeRight = constraintNormalise(solver, node->right, myDM2);
				//cout<<"normalise of right node finished"<<endl;
				z = solverAuxVarNew(solver, NULL, myDM2);
				//cout<<"new aux var built"<<endl;
                solverAddConstrVarEqNode(solver, z, constrNodeRight);
				//cout<<"right part of node finished"<<endl;
            }
			
            domain = myDM;
			for(int i = 0; i < myDM2.size(); i++){
				domain.push_back(myDM2.at(i));
			}
			deleteDup(domain);
			x = solverAuxVarNew(solver, NULL, domain);
			//cout<<"add first eq first constr..."<<endl;
            solverAddConstrFirstEqFirst(solver, x, y);
			//cout<<"end of add first eq first connstr"<<endl;
			//cout<<"add var eq next constr..."<<endl;
            solverAddConstrVarEqNext(solver, z, x);
			//cout<<"end of add var eq next constr"<<endl;
            constrNode = constraintNodeNew(IDENTIFIER, 0, x, NULL, NULL, NULL);
			//cout<<"end of normalise of FBY"<<endl;
            myFree(node);
			//cout<<"end of free node"<<endl;
        } else if (node->token == AT) {
            if (node->left->token == CONSTANT) {
				domain = vector<int>();
				domain.push_back(node->left->num);
                constrNode = node->left;
                myFree(node);
            } else if (node->left->token == NEXT) {
                ConstraintNode * temp_node = node->left;
                int timepoint = 0;
                while(node->left != NULL){
                    if(node->left->token != NEXT )
                        break;
                    else {
                        timepoint++;
                        node->left = node->left->right;
                        myFree(temp_node);
                        temp_node = node->right;
                    }
                }
                node->right->num += timepoint;
                constrNode = node;
            } else {
                if(node->left->token == IDENTIFIER) {
					myDM = node->left->var->domain;
                    y = node->left->var;
                    myFree(node->left);
                } else {
					constrNodeLeft = constraintNormalise(solver, node->left, myDM);
					y = solverAuxVarNew(solver, NULL, myDM);
                    solverAddConstrVarEqNode(solver, y, constrNodeLeft);
                }
				domain = myDM;
				x = solverAuxVarNew(solver, NULL, domain);
                solverAddConstrVarEqAt(solver, x, y, node->right->num);
                constrNode = constraintNodeNew(IDENTIFIER, 0, x, NULL, NULL, NULL);
                myFree(node);
            } 
        } else if (node->token == IDENTIFIER || node->token == CONSTANT) {
            if (node->token == IDENTIFIER) {
				domain = node->var->domain;
            } else {
				domain = vector<int>();
				domain.push_back(node->num);
				//cout<<"CONSTANT node has value "<<node->num<<endl;
            }
            constrNode = node;
        } else if (node->token == ARR_IDENTIFIER) {
			node->right = constraintNormalise(solver, node->right, myDM);
			/*
            vector<int> elements = node->array->elements;
            int size = elements.size();
            lb = elements[1];
            ub = elements[0];
            for(int i = 0; i != size; i++) {
                lb = elements[i] < lb ? elements[i] : lb;
                ub = elements[i] > ub ? elements[i] : ub;
            }
			*/
			domain = node->array->elements;
            constrNode = node;
        } else if (node->token == UNTIL_CON ) {
			/*
            constrNodeLeft = constraintNormalise(solver, node->left, myLB, myUB);
            constrNodeRight = constraintNormalise(solver, node->right, myLB2, myUB2);
			*/
			constrNodeLeft = constraintNormalise(solver, node->left, myDM);
            constrNodeRight = constraintNormalise(solver, node->right, myDM2);

            if (node->left->token != IDENTIFIER ){
				vector<int> temp_dm = vector<int>();
				temp_dm.push_back(0);
				temp_dm.push_back(1);
				x = solverAuxVarNew(solver, NULL, temp_dm);
                node->left = constraintNodeNewVar(x);
                solverAddConstrVarEqNode(solver, x, constrNodeLeft);
            }
            if (node->right->token != IDENTIFIER ){
				vector<int> temp_dm = vector<int>();
				temp_dm.push_back(0);
				temp_dm.push_back(1);
                y = solverAuxVarNew(solver, NULL, temp_dm);
                node->right = constraintNodeNewVar(y);
                solverAddConstrVarEqNode(solver, y, constrNodeRight);
            }
            constrNode = node;
        } else if (node->token == node->token == NOT_OP) {
			constrNodeRight = constraintNormalise(solver, node->right, myDM);
			domain = vector<int>();
			domain.push_back(0);
			domain.push_back(1);
            node->right = constrNodeRight;
            constrNode = node;
		} else if(node->token == MAX_OP || node->token == MIN_OP){
			ConstraintNode *list = node->right; 
			ConstraintNode *list_node = node->right;
			//constrNodeRight = constraintNormalise(solver,list_node->right,myLB,myUB);
			//lb = myLB;
			//ub = myUB;
			constrNodeRight = constraintNormalise(solver,list_node->right,myDM);
			domain = myDM;
			list_node->right = constrNodeRight;
			while(list_node->left!=NULL){
				list_node = list_node->left;
				//constrNodeRight = constraintNormalise(solver,list_node->right,myLB,myUB);
				//lb = lb < myLB ? myLB : lb;
				//ub = ub < myUB ? myUB : ub;
				constrNodeRight = constraintNormalise(solver,list_node->right,myDM);
				domain.insert(domain.end(), myDM.begin(), myDM.end());
				list_node->right = constrNodeRight;
			}
			deleteDup(domain);
			/*
			for(int i = 0; i < domain.size(); i++){
				for(int j = domain.size()-1; j > i; j --){
					if(domain.at(j) == domain.at(i)){
						domain.erase(domain.begin() + j);
					}
				}
			}
			*/

			node->right=list;	
			constrNode = node;
		}  
		else {
			constrNodeLeft = constraintNormalise(solver, node->left, myDM);
            constrNodeRight = constraintNormalise(solver, node->right, myDM2);

            if (node->token == ABS) {
				/*
                if (myLB2 < 0 && myUB2 < 0) {
                    lb = -myUB2;
                    ub = -myLB2;
                } else if (myLB2 < 0 && myUB2 > 0) {
                    lb = 0;
                    ub = MAX(-myLB2, myUB2);
                } else {
                    lb = myLB2;
                    ub = myUB2;
                }
				*/
				for(int j=0; j < myDM.size();j++){
					if(myDM[j]<0){
						myDM[j]=-myDM[j];
					}
				}
				for(int j=0; j < myDM2.size();j++){
					if(myDM2[j]<0){
						myDM2[j]=-myDM[j];
					}
				}
				domain = myDM;
				domain.insert(domain.end(), myDM2.begin(), myDM2.end());
				deleteDup(domain);
            } else if (node->token == IF) {
                //lb = myLB2;
                //ub = myUB2;
				domain = myDM2;
				deleteDup(domain);
            } else if (node->token == THEN) {
                //lb = MIN(myLB, myLB2);
                //ub = MAX(myUB, myUB2);
				domain = myDM;
				domain.insert(domain.end(), myDM2.begin(), myDM2.end());
				deleteDup(domain);
            } else if (node->token == '<' || node->token == '>' || node->token == LE_CON || node->token == GE_CON ||
                       node->token == EQ_CON || node->token == NE_CON || node->token == IMPLY_CON ||
                       node->token == LT_OP || node->token == GT_OP || node->token == LE_OP || node->token == GE_OP ||
                       node->token == EQ_OP || node->token == NE_OP ||
                       node->token == AND_OP || node->token == OR_OP) {
                //lb = 0;
                //ub = 1;
				domain = vector<int>();
				domain.push_back(0);
				domain.push_back(1);
				//cout<<"end of constr with EQ_CON"<<endl;
            } else if (node->token == '+') {
				domain = vector<int>();	
				for(int j =0; j < myDM.size(); j++){
					for(int k = 0; k < myDM2.size(); k++){
						domain.push_back(myDM[j] + myDM2[k]);
					}
				}
				deleteDup(domain);
            } else if (node->token == '-') {
				domain = vector<int>();	
				for(int j =0; j < myDM.size(); j++){
					for(int k = 0; k < myDM2.size(); k++){
						domain.push_back(myDM[j] - myDM2[k]);
					}
				}
				deleteDup(domain);
            } else if (node->token == '*') {

				domain = vector<int>();	
				for(int j =0; j < myDM.size(); j++){
					for(int k = 0; k < myDM2.size(); k++){
						domain.push_back(myDM[j] * myDM2[k]);
					}
				}
				deleteDup(domain);
            } else if (node->token == '/') {
				domain = vector<int>();	
				for(int j =0; j < myDM.size(); j++){
					for(int k = 0; k < myDM2.size(); k++){
						domain.push_back(myDM[j] / myDM2[k]);
					}
				}
				deleteDup(domain);
            } else if (node->token == '%') {
				domain = vector<int>();	
				for(int j =0; j < myDM.size(); j++){
					for(int k = 0; k < myDM2.size(); k++){
						domain.push_back(myDM[j] % myDM2[k]);
					}
				}
				deleteDup(domain);
			}
			//delete duplicate value in domains
			deleteDup(domain);
			/*
			for(int i = 0; i < domain.size(); i++){
				for(int j = domain.size()-1; j > i; j --){
					if(domain.at(j) == domain.at(i)){
						domain.erase(domain.begin() + j);
					}
				}
			}		
			*/
            node->left = constrNodeLeft;
            node->right = constrNodeRight;
            constrNode = node; //constraintNodeNew(node->token, 0, NULL, constrNodeLeft, constrNodeRight);
        }
    } else {
        // myLog(LOG_ERROR, "ConstraintNode to be normalised is NULL!\n");
    }
	/*
	for( int j = 0; j < domain.size(); j++ ){
		for( int k = domain.size()-1; k > j; k--){
			if(domain.at(k) == domain.at(j)){
				domain.erase(domain.begin()+k);
			}
		}
	}
	*/
    return constrNode;
}

/* OmegaSolver */
// this recursive function is to evaluate the expression in constraint and return the value to upper
int solverValidateRe(ConstraintNode *node, bool & valid) {
	//cout<<"in solverValidateRe"<<endl;
    int left = 0;
    int right = 0;
    int temp = 0;
    int res = 0;
    if (node != NULL) {
        if (node->token == IDENTIFIER) {
            res = node->var->propagateValue;
        } else if (node->token == ARR_IDENTIFIER) {
            temp = solverValidateRe(node->right, valid);
            myLog(LOG_TRACE, "temp: %d\n", temp);
            if(temp < 0 || temp >= node->array->size ){
                res = valid = false;
            } else {
                res = node->array->elements[temp];
            }
            myLog(LOG_TRACE, "res: %d\n", res);
        } else if (node->token == CONSTANT) {
            res = node->num;
        } else if (node->token == ABS) {
            res = abs(solverValidateRe(node->right, valid));
        } else if (node->token == IF) {
            temp = solverValidateRe(node->left, valid);
            if (temp) {
                res = solverValidateRe(node->right->left, valid);
            } else {
                res = solverValidateRe(node->right->right, valid);
            }
        } else if (node->token == FIRST) {
            res = solverValidateRe(node->right, valid);
        } else if (node->token == AT ) {
            res = solverValidateRe(node->left, valid);
        } else if (node->token == NOT_OP) {
            temp = solverValidateRe(node->right, valid);
            res = (temp == 0? 1 : 0);
        } else if (node->token == AND_OP) {
            temp = solverValidateRe(node->left, valid);
            if (temp) {
                res = solverValidateRe(node->right, valid);
            } else {
                res = 0;
            }
        } else if (node->token == OR_OP) {
            temp = solverValidateRe(node->left, valid);
            if (temp) {
                res = 1;
            } else {
                res = solverValidateRe(node->right, valid);
            }
        } else if (node->token == IMPLY_CON) {
            left = solverValidateRe(node->left, valid);
            if ( left == 0 ){
                res = 1;
            } else {
                right = solverValidateRe(node->right, valid);
                res = (left <= right);
            }
		} else if(node->token == MAX_OP){
			ConstraintNode *list_node = node->right;
			right = solverValidateRe(list_node->right, valid);
			res = right;
			while(list_node->left != NULL){
				list_node = list_node->left;
				right = solverValidateRe(list_node->right, valid);
				res = res > right ? res : right;
			}
		} else if(node->token == MIN_OP){
			ConstraintNode *list_node = node->right;
			right = solverValidateRe(list_node->right, valid);
			res = right;
			while(list_node->left != NULL){
				list_node = list_node->left;
				right = solverValidateRe(list_node->right, valid);
				res = res < right ? res : right;
			}
		}else {
            left = solverValidateRe(node->left, valid);
            right = solverValidateRe(node->right, valid);
            if (!valid) {
                res = false;
            } else if (node->token == '<' || node->token == LT_OP) {
                res = (left < right);
            } else if (node->token == '>' || node->token == GT_OP) {
                res = (left > right);
            } else if (node->token == LE_CON || node->token == LE_OP) {
                res = (left <= right);
            } else if (node->token == GE_CON || node->token == GE_OP) {
                res = (left >= right);
            } else if (node->token == EQ_CON || node->token == EQ_OP) {
                res = (left == right);
            } else if (node->token == NE_CON || node->token == NE_OP) {
                res = (left != right);
            } else if (node->token == '+') {
                res = left + right;
            } else if (node->token == '-') {
                res = left - right;
            } else if (node->token == '*') {
                res = left * right;
            } else if (node->token == '/') {
                res = left / right;
            } else if (node->token == '%') {
                res = left % right;
			} 
        }
    }
	//cout<<"end of one validateRe"<<endl;
    return res;
}

// Validates a constraint given propagation values. Used in consistency enforcement
// and hence implicitly for constraint checking.
bool validate(Constraint *constr) {
    bool valid = true;
    return solverValidateRe(constr->node, valid);
}

// Recursive search for findSupport. Recursing in index, i.e. the index of the current
// searched variable in constr's variable list.
bool findSupportRe(Constraint *constr, Variable *var, int point, int index, int numVar) {
    Variable *thisVar = (*constr->variables)[index];
    if (index == numVar-1) {
        if (thisVar == var) {
            return validate(constr);
        } else {
            bool supported = false;
			vector<int> domain = thisVar->currDM[point];
			/*
			int size = domain.size();
			for(int i = 0; i < size; i++){
				for(int j = size - 1; j > i; j--){
					if(domain.at(j) == domain.at(i)){
						domain.erase(domain.begin() + j);
						size--;
					}		
				}
			}
			if(domain.size() < thisVar->currDM[point].size()){
				thisVar->currDM[point] = domain;
			}
			*/
			bool hasDup = deleteDup(domain);
			if(hasDup){
				thisVar->currDM[point] = domain;
			}
			for(int i = 0; !supported && i < thisVar->currDM[point].size(); i++){
				thisVar->propagateValue = thisVar->currDM[point].at(i);
				supported = validate(constr);
			}
            return supported;
        }
    } else {
        if (thisVar == var) {
            return findSupportRe(constr, var, point, index+1, numVar);
        } else {
            bool supported = false;
			vector<int> domain = thisVar->currDM[point];
			/*
			int size = domain.size();
			for(int i = 0; i < size; i++){
				for(int j = size - 1; j > i; j--){
					if(domain.at(j) == domain.at(i)){
						domain.erase(domain.begin() + j);
						size--;
					}		
				}
			}
			if(domain.size() < thisVar->currDM[point].size()){
				thisVar->currDM[point] = domain;
			}
			*/
			bool hasDup = deleteDup(domain);
			if(hasDup){
				thisVar->currDM[point] = domain;
			}
			for(int c = 0; !supported && c < thisVar->currDM[point].size(); c++){
				thisVar->propagateValue = thisVar->currDM[point].at(c);
				supported = findSupportRe(constr, var, point, index+1, numVar);
			}
            return supported;
        }
    }
}

// Returns true if there is a support for var in constr at time point.
bool findSupport(Constraint *constr, Variable *var, int point) {
    int numVar = constr->variables->size();
    return findSupportRe(constr, var, point, 0, numVar);
}

// Assumes constr is a pointwise constraint.
// Returns true if there is no inconsistency for the arc (var, constr) at time point.
// check variable upper and lower bound of variable
// for each value of var, find support in context of constraint
bool enforcePointConsistencyAt(Constraint *constr, Variable *var, bool &change, int point) {
    bool supported = false;
	vector<int> domain = var->currDM[point];


	// if there exist duplicated value in domain, delete and rebuild domain
	/*
	int size = domain.size();
	for(int cur_index = 0; cur_index < size; cur_index++){
		for(int index = size - 1; index > cur_index; index--){
			if(domain.at(index) == domain.at(cur_index)){
				domain.erase(domain.begin()+index);
				size--;
			}
		}
	}	
	if(domain.size() < var->currDM[point].size()){
		var->currDM[point] = domain;
	}
	*/
	bool hasDup = deleteDup(domain);
	if(hasDup){
		var->currDM[point] = domain;
	}

		

    myLog(LOG_DEBUG, "enforce point consistency at point :%d\n", point);
    constraintPrint(constr);
    //myLog(LOG_DEBUG, "variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, lb, ub);

	//size = var->currDM[point].size();
	/*	
	for(int i = var->currDM[point].size()-1; i >= 0; i--){
		var->propagateValue = var->currDM[point].at(i);
		supported = findSupport(constr, var, point);
		if(!supported){
			change = true;
			backup_dm(&var->currDM[point]);
			if(var->currDM[point].at(i) == var->propagateValue)
				var->currDM[point].erase(var->currDM[point].begin()+i);
			//cout<<"unsupported once"<<endl;
			
			for(int j = var->currDM[point].size()-1; j>=0; j--)
			{
				if(var->currDM[point].at(j) == var->propagateValue){
					var->currDM[point].erase(var->currDM[point].begin() + j);
					size--;
				}
			}
			
		}

	}
	*/
	//cout<<"start to find support"<<endl;
	for( int i = domain.size()-1; i >= 0; i -- ){
		var->propagateValue = domain.at(i);
		supported = findSupport(constr, var, point);
		if(!supported){
			change = true;
			domain.erase(domain.begin()+ i);
		}
	}
	//cout<<"after find support"<<endl;
	if(domain.size() < var->currDM[point].size()){
		//cout<<"change and backup"<<endl;
		backup_dm(&var->currDM[point]);
		var->currDM[point] = domain;
		//cout<<"finish backup"<<endl;
	}
	
	//cout<<"assign support"<<endl;
	if(var->currDM[point].size()==0){
		supported = false;
	} else if(var->currDM[point].size() > 0){
		supported = true;
	}
	
	//cout<<"log..."<<endl;

    //myLog(LOG_DEBUG, "after consistency, variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, var->currLB[point], var->currUB[point]);
	/*
	myLog(LOG_DEBUG, "after consistency, variable: %s, domain:{", var->name);
	for(int i = 0; i < var->currDM[point].size() - 1; i++){
		myLog(LOG_DEBUG, "%d, ",var->currDM[point].at(i));
	}
	if(var->currDM[point].size() > 0){
		myLog(LOG_DEBUG, "%d}\n",var->currDM[point].back());
	} else{
		myLog(LOG_DEBUG, "} domain is empty\n");
	}
	*/
	//cout<<"end of log..."<<endl;
	//cout<<"finish find support, return then"<<endl;
    myLog(LOG_DEBUG, "supported: %d\n", supported);
    return supported;
}

// Assumes constr is a pointwise constraint.
// Returns true if the arc (var, constr) can be made consistent at all k time points.
bool enforcePointConsistency(Solver *solver, Constraint *constr, Variable *var, bool &change) {
    myLog(LOG_DEBUG, "solver, enforce point consistency at time point :%d\n", solver->timePoint);
	//cout<<endl;
	//cout<<"constr to be enforced: "<<constr->id<<endl;
	//myPrintLevel();
    if (!constr->hasFirst) {
        bool consistent = true;
        int k = solver->prefixK;
        for (int c = 0; consistent && c < k; c++) {
            consistent = enforcePointConsistencyAt(constr, var, change, c);
			
			//cout<<"VAR name:"<<var->name<<" , after consistency at point "<<c<<" :domain: ";
			//for(int i = 0; i < var->currDM[c].size(); i++){
			//	cout<<var->currDM[c].at(i)<<" ";
		//	}
		//	cout<<endl;
			
        }
		
        return consistent;
    } else {
        return enforcePointConsistencyAt(constr, var, change, 0);
    }
}

// Assumes constr is a constraint of form X == next Y.
// Returns true if the arc (var, constr) can be made consistent.
// check the upper bound and lower bound of variable X & Y
bool enforceNextConsistency(Solver *solver, Constraint *constr, Variable *var, bool &change) {
    myLog(LOG_DEBUG, "enforce next consistency \n");
    constraintPrint(constr);
    bool consistent = true;
	vector<int> temp_dm1, temp_dm2, retained_dm;
    //if variable is child var Y 
    if (var == constr->node->right->right->var) {
		int k = solver->prefixK;
        for (int c = 1; consistent && c < k; c++) {
            /*
			myLog(LOG_DEBUG, "variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, var->currLB[c], var->currLB[c]);
            if (var->currLB[c] < constr->node->left->var->currLB[c-1]) {
                change = true;
                backup(&var->currLB[c]);
                var->currLB[c] = constr->node->left->var->currLB[c-1];
            }
            if (var->currUB[c] > constr->node->left->var->currUB[c-1]) {
                change = true;
                backup(&var->currUB[c]);
                var->currUB[c] = constr->node->left->var->currUB[c-1];
            }
            if (var->currLB[c] > var->currUB[c]) {
                consistent = false;
            }
            myLog(LOG_DEBUG, "after consistency, variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, var->currLB[c], var->currUB[c]);
        }
			*/
			//compare the domain of child var Y and parent var X, retain their intersection
			temp_dm1 = var->currDM[c];
			/*
			for( int i = 0; i < temp_dm1.size(); i++ ){
				for( int j = temp_dm1.size()-1; j > i; j-- ){
					if(temp_dm1.at(j) == temp_dm1.at(i)){
						temp_dm1.erase(temp_dm1.begin() + j);
					}
				}
			}
			if(temp_dm1.size()<var->currDM[c].size()){
				var->currDM[c] = temp_dm1;	
			}
			*/
			bool hasDup = deleteDup(temp_dm1);
			if(hasDup){
				var->currDM[c] = temp_dm1;	
			}
			temp_dm2 = constr->node->left->var->currDM[c-1];
			hasDup = deleteDup(temp_dm2);
			if(hasDup){
				constr->node->left->var->currDM[c-1] = temp_dm2;
			}
			/*
			for( int i = 0; i < temp_dm2.size(); i++ ){
				for( int j = temp_dm2.size()-1; j > i; j-- ){
					if(temp_dm2.at(j) == temp_dm2.at(i)){
						temp_dm2.erase(temp_dm2.begin() + j);
					}
				}
			}
			if(temp_dm2.size() < constr->node->left->var->currDM[c-1].size()){
				constr->node->left->var->currDM[c-1] = temp_dm2;
			}
			*/

			retained_dm = vector<int> ();
			for(int i = 0; i < temp_dm1.size(); i++){
				for(int j = 0; j < temp_dm2.size(); j++){
					if(temp_dm1.at(i) == temp_dm2.at(j)){
						retained_dm.push_back(temp_dm1.at(i));
						break;
					}
				}
			}
			/*
			int size = retained_dm.size();
			for(int i = 0; i < size; i++){
				for(int j = size - 1; j > i; j--){
					if(retained_dm.at(j) == retained_dm.at(i)){
						retained_dm.erase(retained_dm.begin()+j);
						size--;
					}
				}
			}
			*/
			if(retained_dm.size() > 0 && retained_dm.size() < temp_dm1.size()){
				change = true;
				backup_dm(&var->currDM[c]);
				var->currDM[c] = retained_dm;
			}
			if(retained_dm.size() == 0){
				consistent = false;
			}
    	}

    // else if variable is parent var X 
	} else {
        Variable *otherVar = constr->node->right->right->var;
        int k = solver->prefixK;
        for (int c = 0; consistent && c < k-1; c++) {
            /*
			myLog(LOG_DEBUG, "variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, var->currLB[c], var->currLB[c]);
            if (var->currLB[c] < otherVar->currLB[c+1]) {
                change = true;
                backup(&var->currLB[c]);
                var->currLB[c] = otherVar->currLB[c+1];
            }
            if (var->currUB[c] > otherVar->currUB[c+1]) {
                change = true;
                backup(&var->currUB[c]);
                var->currUB[c] = otherVar->currUB[c+1];
            }
            if (var->currLB[c] > var->currUB[c]) {
                consistent = false;
            }
            myLog(LOG_DEBUG, "after consistency, variable: %s, lower_bound: %d, upper_bound: %d\n", var->name, var->currLB[c], var->currUB[c]);
			*/
			temp_dm1 = var->currDM[c];	
			bool hasDup = deleteDup(temp_dm1);
			if(hasDup){
				var->currDM[c] = temp_dm1;
			}
			/*
			for( int i = 0; i < temp_dm1.size(); i++ ){
				for( int j = temp_dm1.size()-1; j > i; j-- ){
					if(temp_dm1.at(j) == temp_dm1.at(i)){
						temp_dm1.erase(temp_dm1.begin() + j);
					}
				}
			}
			if(temp_dm1.size()<var->currDM[c].size()){
				var->currDM[c] = temp_dm1;	
			}
			*/
			temp_dm2 = otherVar->currDM[c+1];
			/*
			for( int i = 0; i < temp_dm2.size(); i++ ){
				for( int j = temp_dm2.size() - 1; j > i; j-- ){
					if(temp_dm2.at(j) == temp_dm2.at(i)){
						temp_dm2.erase(temp_dm2.begin() + j);
					}
				}
			}
			if(temp_dm2.size()<otherVar->currDM[c+1].size()){
				otherVar->currDM[c+1] = temp_dm2;	
			}
			*/
			hasDup = deleteDup(temp_dm2);
			if(hasDup){
				otherVar->currDM[c+1] = temp_dm2;
			}
			retained_dm = vector<int> ();
			for(int i = 0; i < temp_dm1.size(); i++){
				for(int j = 0; j < temp_dm2.size(); j++){
					if(temp_dm1.at(i) == temp_dm2.at(j)){
						retained_dm.push_back(temp_dm1.at(i));
						break;
					}
				}
			}
			/*
			int size = retained_dm.size();
			for(int i = 0; i < size; i++){
				for(int j = size - 1; j > i; j--){
					if(retained_dm.at(j) == retained_dm.at(i)){
						retained_dm.erase(retained_dm.begin()+j);
						size--;
					}
				}
			}
			*/
			deleteDup(retained_dm);
			if(retained_dm.size() > 0 && retained_dm.size() < temp_dm1.size()){
				change = true;
				backup_dm(&var->currDM[c]);
				var->currDM[c] = retained_dm;
			}
			else if(retained_dm.size() == 0){
				consistent = false;
			}
        }
    }
    myLog(LOG_DEBUG, "exit from next consistency\n");
    return consistent;
}

// Assumes constr is a constraint of form X until Y.
// Returns true if the arc (var, constr) can be made consistent.
// check the upper bound and lower bound of variable X & Y
bool enforceUntilConsistency(Solver *solver, Constraint *constr, Variable *var, bool &change){
    bool consistent = true;
    // if variable is right var Y
    Variable * left_var = constr->node->left->var;
    Variable * right_var = constr->node->right->var;
	deleteDup(left_var->currDM[0]);
	deleteDup(right_var->currDM[0]);
    if ( constr->expire )
        return consistent;
	
    else if ( left_var->currDM[0].size() == 1 && right_var->currDM[0].size() == 1 ) {
        if ( right_var->currDM[0].at(0) != 1 && left_var->currDM[0].at(0) != 1 ){
            consistent = false;
        }
        return consistent;
		
    } else {
        return consistent;
    }
	
	return consistent;
}

// GAC algorithm
bool generalisedArcConsistent(Solver *solver) {
	test_num ++;
	//cout<<test_num<<" times in generalisedArcConsistent"<<endl;
//	cout<<"one gac"<<endl;
    myLog(LOG_TRACE, "enter generalisedArcConsistent\n");
    bool consistent = true; // consistent is to check whether the consistency is satisfied
    bool change = false; // change is to check whether the range has changed
    // Push in all arcs
    // between every variable and its corresponding constraint, there is an arc
    // free solver arcqueue if it is not empty 
    while (!solver->arcQueue->empty()) {
        Arc *arc = solver->arcQueue->front();
        solver->arcQueue->pop_front();
        arc->inqueue = false;
    }
    
    int numConstr = solver->constrQueue->size();
    for (int c = 0; c < numConstr; c++) {
        Constraint *constr = (*(solver->constrQueue))[c];
        int numVar = constr->numVar;
        
        ArcQueue *arcs = constr->arcs;
        int numArc = arcs->size();
        for (int d = 0; d < numArc; d++) {
            Arc * temp = (*arcs)[d];
            temp->inqueue = true;
            solver->arcQueue->push_back(temp);
			//cout<<"ARC: constr: "<<temp->constr->id<<", var name: "<<temp->var->name<<endl;
        }
    }
    // Enforce consistency
	//cout<<"general arc start to prepare consistency"<<endl;
    while (consistent && !solver->arcQueue->empty()) {
        Arc *arc = solver->arcQueue->front();
        solver->arcQueue->pop_front();
        arc->inqueue = false;
        Constraint *constr = arc->constr;
        Variable *var = arc->var;

        // Enforce arc
        if (constr->type == CONSTR_NEXT) {
			//cout<<"enforce next consistency"<<endl;
            consistent = enforceNextConsistency(solver, constr, var, change);
        } else if (constr->type == CONSTR_POINT) {
			//cout<<"enforce point consistency"<<endl;
            consistent = enforcePointConsistency(solver, constr, var, change);
        } else if (constr->type == CONSTR_UNTIL ) {
			//cout<<"enforce until constraint consistency"<<endl;
            consistent = enforceUntilConsistency(solver, constr, var, change);
        } else if (constr->type == CONSTR_AT ) {
            // lazy evaluation for constr_at 
            // constraint translate <var>@0 to first <var>
            consistent = true;
        }
		//cout<<"enforce arc and get result"<<endl;
        if (!consistent) {
            myLog(LOG_DEBUG, "inconsistent variable: %s\n", var->name);
            constraintNodeLogPrint(constr->node, solver);
            myLog(LOG_DEBUG, ";\n");
        }
		//cout<<"enforce done, continue..."<<endl;
		//cout<<"consistency: "<<consistent<<endl;
		//cout<<"change: "<<change<<endl;
        if (consistent && change) {
			//cout<<"fix change"<<endl;
            // Push in relevant arcs
            if(strcmp(var->name, "_V1") == 0){
                myLog(LOG_DEBUG, "constraintID: %d\n", solver->constraintID);
                myLog(LOG_DEBUG, "consistent: variable: %s\n", var->name);
                constraintNodeLogPrint(constr->node, solver);
                myLog(LOG_DEBUG, ";\n");
            }

            ConstraintQueue *constraints = var->constraints;
            int size = constraints->size();
            for (int c = 0; c < size; c++) {
                if ((*constraints)[c] != constr) {
                    ArcQueue *arcs = (*constraints)[c]->arcs;
                    int numArc = arcs->size();
                    for (int d = 0; d < numArc; d++) {
                        Arc * temp = (*arcs)[d];
                        if(!temp->inqueue){
                            temp->inqueue = true;
                            solver->arcQueue->push_back(temp);
                        }   
                    }
                }
            }
        }
		//cout<<"after deal enforce arc result"<<endl;
        arc->inqueue = false;
        change = false;
    }
	//cout<<"arc done "<<endl;
    while (!solver->arcQueue->empty()) {
        Arc *arc = solver->arcQueue->front();
        solver->arcQueue->pop_front();
        arc->inqueue = false;
    }
	//cout<<"one gac finished..."<<endl;
    //myLog(LOG_TRACE, "exit generalisedArcConsistent, consistent: %d\n",consistent);
    return consistent;
}

// Outputs solution in the DOT format.
void solverOut(Solver *solver) {
    FILE *fp = fopen("solutions.dot", "w");
    fprintf(fp, "# Number of nodes = %d\n", solver->numNodes);
    fprintf(fp, "#");
    int numVar = solver->varQueue->size();
    for (int c = 0; c < numVar; c++) {
        fprintf(fp, " %s", ((*(solver->varQueue))[c])->name);
    }
    fprintf(fp, "\n");
    fprintf(fp, "#");
    for (int c = 0; c < numVar; c++) {
        if (((*(solver->varQueue))[c])->isSignature) {
            fprintf(fp, " %s", ((*(solver->varQueue))[c])->name);
        }
    }
    fprintf(fp, "\n");
    fprintf(fp, "digraph \"StCSP\" {\n");
    fflush(fp);
    graphOut(solver->graph, fp, numVar, solver->numSignVar, solver->numUntil);
    fprintf(fp, "}\n");
    fclose(fp);
}

/* OmegaSolver */
int solverSolveRe(Solver *solver, Vertex *vertex) {
	//cout<<"sovlerRe"<<endl;
    Vertex *temp;
    Edge *edge;
    bool ok = false;

    Variable *var = solverGetFirstUnboundVar(solver);
    if (var == NULL) {
		solver->numStates++;
      // if there is no variables left, start to solve the instanteous CSP and generate new child node
        int size = solver->varQueue->size();
        myLog(LOG_TRACE, "Before level up\n");
        levelUp();
        myLog(LOG_TRACE, "After level up\n");

        // timePoint
        backup(&solver->timePoint);
        solver->timePoint++;

        vector<ConstraintQueue *> varConstraints;
        ConstraintQueue *solverConstraintsBackup = solver->constrQueue;

        // constraint rewriting 
        bool constraintQueueFound = false;
        if (/*solver->timePoint == 1 && */ solver->hasFirst) {
            // Constraint translation and identification
            // Only applicable in the first time point, because only "first" constraints warrant translation
            // Use variable values for constraint translation
            myLog(LOG_DEBUG, "constraintID: %d\n", solver->constraintID);
            for (int c = 0; c < size; c++) {
                myLog(LOG_DEBUG, "%s: %d ", (*(solver->varQueue))[c]->name, variableGetValue((*(solver->varQueue))[c]));
            }

            myLog(LOG_DEBUG, "\n");
            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];
                backup(&var->numConstr);

                // Backup var's constraints
                varConstraints.push_back(var->constraints);
                var->constraints = constraintQueueNew();
                // Done
            }

            // Now translate constraints
            backup(&solver->constraintID);
            solver->constrQueue = constraintQueueNew();
            int oldNumConstr = solverConstraintsBackup->size();
            for (int c = 0; c < oldNumConstr; c++) {
                Constraint *output = constraintTranslate((*solverConstraintsBackup)[c], solver);
                if (output != NULL) {
                    solverConstraintQueuePush(solver->constrQueue, output, solver, true);
                }
            }
            // Done

            // int tempnum = solver->constrQueue->size();
            // for (int i = 0; i < tempnum; i++) {
            //     constraintPrint((*(solver->constrQueue))[i]);
            // }

            int numSeenConstraints = solver->seenConstraints->size();
            for (int c = 0; !constraintQueueFound && c < numSeenConstraints; c++) {
                
                if (constraintQueueEq(solver->constrQueue, (*(solver->seenConstraints))[c])) {
                    solver->constraintID = c;
                    constraintQueueFound = true;
                }
            }
            if (!constraintQueueFound) {
                solver->constraintID = numSeenConstraints;
                solver->seenConstraints->push_back(solver->constrQueue);
            }
            // Done
        }

        
        myLog(LOG_DEBUG, "constraintID: %d\n", solver->constraintID);

        // Prepare variable signature
        myLog(LOG_TRACE, "Creating signature\n");
        vector<int> varSig;
        for (int c = 0; c < size; c++) {
            if ((*(solver->varQueue))[c]->isSignature) {
                varSig.push_back(variableGetValue((*(solver->varQueue))[c]));
            }
        }

        int tem_size = solver->constrQueue->size();
        // push until signature to varSig
        for(int c = 0; c < tem_size; c++) {
            Constraint * temp_constr = (*(solver->constrQueue))[c];
            if( temp_constr->type == CONSTR_UNTIL){
                backup(&temp_constr->expire);
                if( temp_constr->expire == 1 ){
                    varSig.push_back(1);
                }
				/* else if( temp_constr->node->right->var->currLB[0] == 1 ) {
                    temp_constr->expire = 1;
                    varSig.push_back(1);
                }
			   	 	else {
                    varSig.push_back(0);
                }
				*/
				else{
					vector<int> temp_dm = temp_constr->node->right->var->currDM[0];
					// find if the minium value in domain is 1
					int min = 0;
					if(temp_dm.size() >= 1){
						min = temp_dm.at(0);
					}
					for(int i = 0; i < temp_dm.size(); i++){
						if(temp_dm.at(i) < min){
							min = temp_dm.at(i);
						}
					}	
					if(min == 1){
						temp_constr->expire = 1;
						varSig.push_back(1);
					}
					else{
						varSig.push_back(0);
					}
				}
            }
        }


        Signature *signature = new Signature(varSig, solver->constraintID);
        //delete varSig;
        myLog(LOG_TRACE, "Finish creating signature\n");
        temp = vertexTableGetVertex(solver->graph->vertexTable, *signature);
        myLog(LOG_TRACE, "Finish fetching\n");
        if (temp == NULL) {
            myLog(LOG_TRACE, "Nothing fetched\n");
            // New vertex representing the search node after have a complete edge from the "vertex" argument
            temp = vertexNew(solver->graph, signature, solver->timePoint);
            myLog(LOG_TRACE, "New state created\n");
            vertexTableAddVertex(solver->graph->vertexTable, temp);
            myLog(LOG_TRACE, "Added to table\n");

            // variable->prevValue
            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];
                variableAdvanceOneTimeStep(solver, var);
            }
            // myLog(LOG_DEBUG, "Previous value\n");

            if (generalisedArcConsistent(solver)) {
				//cout<<"after generalised Arc consistency and continue solveRe"<<endl;
                myLog(LOG_DEBUG, "After consistency1\n");
                ok = solverSolveRe(solver, temp);

            } else {
                myLog(LOG_DEBUG, "After consistency2\n");
                solver->numFails++;
                ok = false;
            }
        } else if (temp->fail) {
            // reach a previous fail state 
            myFree(signature);
            ok = false;
        } else {
            myLog(LOG_DEBUG, "*** Dominance detected\n\n");
            solver->numDominance++;
            myFree(signature);
            ok = true;
        }
        myLog(LOG_DEBUG, "After branching\n");
        if (solver->hasFirst) {
            // Undo constraint translations
            // No need to free current constrQueue because seenConstraints needs to keep it

            // Free constraint
            // solver->constrQueue
            if(constraintQueueFound){
                int queueSize = solver->constrQueue->size(); 
                for (int j = 0; j < queueSize; j++) { 
                    constraintFree((*(solver->constrQueue))[j]); 
                }
            }

            solver->constrQueue = solverConstraintsBackup;
            // Done

            for (int c = 0; c < size; c++) {
                Variable *var = (*(solver->varQueue))[c];

                // Restore var's constraints
                constraintQueueFree(var->constraints);
                var->constraints = varConstraints[c];
                // Done
            }
        }
        myLog(LOG_TRACE, "After restoring constraints\n");
        levelDown();
        myLog(LOG_TRACE, "After levelling down\n");
        if (ok) {
            edge = edgeNew(vertex, temp, solver->varQueue, solver->varQueue->size());
            vertexAddEdge(vertex, edge);
        } else {
            temp->fail = true;
            myLog(LOG_TRACE, "After denote vertex as failure\n");
        }
    } else {
		//cout<<"unbounded var"<<endl;
        // if there are unbound first variable, split the variable to upper half range and lower half range
        // and sovle the corresponding new stream CSP
        levelUp();
        variableSplitLower(var);
		//cout<<"after split lower"<<endl;

        //myLog(LOG_DEBUG, "time point: %d\n", solver->timePoint);
        //myLog(LOG_DEBUG, "selected: %s [%d,%d] \n", var->name, var->currLB[0], var->currUB[0]);

        if (generalisedArcConsistent(solver)) {
			//cout<<"splitlower and arc consistent"<<endl;
            ok |= solverSolveRe(solver, vertex);
			//cout<<"a solveRe done"<<endl;
        } else {
            solver->numFails++;
        }
		//cout<<"start to level down and return back"<<endl;
        levelDown();
		//cout<<"succeed to return back"<<endl;	
		/*
		cout<<"after levelDown, var "<<var->name<<" has domain: ";
		for(int i = 0; i < var->currDM[0].size(); i++){
			cout<<var->currDM[0].at(i);
		}
		cout<<endl;
		*/
		//cout<<"level down and start to split upper"<<endl;
        levelUp();
        variableSplitUpper(var);

        //myLog(LOG_DEBUG, "time point: %d\n", solver->timePoint);
        //myLog(LOG_DEBUG, "selected: %s [%d,%d]\n", var->name, var->currLB[0], var->currUB[0]);

        if (generalisedArcConsistent(solver)) {
            ok |= solverSolveRe(solver, vertex);
        } else {
            solver->numFails++;
        }
        levelDown();
    }
	//cout<<"end of one solveRe"<<endl;
    return ok;
}

// Entry point from solve() in solver.cpp.
double solverSolve(Solver *solver, bool testing) {
	//cout<<"prepare to solve"<<endl;
    //solverPrint(solver);
    solver->solveTime = cpuTime();
    solver->seenConstraints->push_back(solver->constrQueue);
	//cout<<"in Solver Solve"<<endl;
    //create root node for solving  
    vector<int> temp;
    Signature *signature = new Signature(temp, 0);
    solver->graph->root = vertexNew(solver->graph, signature, 0);
    vertexTableAddVertex(solver->graph->vertexTable, solver->graph->root);
    //check whether there are until constraints
    int tem_size = solver->constrQueue->size();
    solver->graph->root->final = true;
    for(int c = 0; c < tem_size; c++) {
        Constraint * temp_constr = (*(solver->constrQueue))[c];
        if( temp_constr->type == CONSTR_UNTIL ){
            solver->graph->root->final = false; 
            break;
        }
    }

    levelUp();
	//cout<<"start to gac"<<endl;
    if (generalisedArcConsistent(solver)) {
		//cout<<"succeed to gac, start to solve"<<endl;
        solverSolveRe(solver, solver->graph->root);
		//cout<<"succeed solved"<<endl;
    } else {
        solver->numFails++;
    }
    solver->solveTime = cpuTime() - solver->solveTime;
    solver->processTime = cpuTime();
    graphTraverse(solver->graph, solver->numSignVar, solver->numUntil);
    if (solver->adversarial1){
        adversarialTraverse(solver->graph, solver->varQueue);
        printf("adver1: %d; ", solver->graph->root->valid);
    }
	
    if (solver->adversarial2){
        adversarialTraverse2(solver->graph, solver->varQueue);
        printf("adver2: %d\n", solver->graph->root->valid);
    }
    
    renumberVertex(solver->graph);
    solver->processTime = cpuTime() - solver->processTime;
    levelDown();
    

    solver->numNodes = solver->graph->vertexTable->size();
    myLog(LOG_INFO, "Dominance: %d\n", solver->numDominance);
    myLog(LOG_INFO, "Nodes: %d\n", solver->numNodes);
    myLog(LOG_INFO, "Fails: %d\n", solver->numFails);

    if (solver->printSolution) {
        solverOut(solver);
    }

    if (!testing) {
        // init_time, var, con, dom, node, fail, solve_time, processTime
        printf("solver->initTime: %.2f\nsolver->varQueue->size(): %d\nsolver->constrQueue->size(): %d\nsolver->numDominance: %d\nsolver->numNodes: %d\nsolver->numFails: %d\nsolver->numStates: %d\nsolver->solverTime: %.2f\nsolver->processTime%.5f\n", solver->initTime, (int) solver->varQueue->size(), (int) solver->constrQueue->size(), solver->numDominance, solver->numNodes, solver->numFails, solver->numStates, solver->solveTime, solver->processTime);
        fflush(stdout);
    }
    return solver->solveTime + solver->processTime;
}
