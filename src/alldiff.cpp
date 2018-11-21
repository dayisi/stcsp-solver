#include<iostream>
#include "constraint.h"
#include "variable.h"
#include "solveralgorithm.h"
#include "node.h"
#include "util.h"
#include "token.h"
#include "y.tab.h"
#include "solver.h"
#include "graph.h"


using namespace std;
bool findAugPathFromVarRe(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* start_var, Variable* var);
bool findAugPathFromNumRe(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* start_var, int num);
bool findAugPathFromVar(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* var, int point);
void buildMaxMatchingFrom(vector<MyEdge*>* graph, vector<MyEdge*>* partial_matching, vector<MyEdge*>* max_matching, VariableQueue* variables, int point);
vector<MyEdge* >* computeMaxMatch(vector<MyEdge*>* graph, VariableQueue* variables, int point);
vector<MyEdge*>  *buildGraph(Constraint *constr, int point);
vector<MyEdge*>* greedilyFindMatch(vector<MyEdge*> *graph);
vector<MyEdge*>* removeEdgeFromG(vector<MyEdge*> *graph, vector<MyEdge*>* matching);
bool computeSCCFromRe(vector<MyEdge*>*graph, MyEdge* start_edge, MyEdge* curr_edge, vector<MyEdge*>* my_stack);
bool computeSCCFrom(vector<MyEdge*>* graph, MyEdge* start_edge);
void computeSCC(vector<MyEdge*>* graph, vector<MyEdge*> *matching);

//using BFS to find augmentation path


//continue to search the augmenting path from the var vertex;
bool findAugPathFromVarRe(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* start_var, Variable* var){
    bool found = false;
    for(int i = 0; i < graph->size(); i++){
        //if no such edge in the graph, the augmenting path terminates with the var vertex, the path searched fails. 
        if(graph->at(i)->var == var && graph->at(i)->direction == 0){
            found = findAugPathFromNumRe(graph, partial_matching, var, graph->at(i)->num);
            if(found){
                graph->at(i)->direction = 1;
                partial_matching->push_back(graph->at(i));
                return found;
            }
        
        }
    }
    
    return found;
}

//continue to search the augmenting path from the num vertex;
bool findAugPathFromNumRe(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* start_var, int num){
    bool found = true;
    for(int i = 0; i < partial_matching->size(); i++){
        //if the num is not in the matching, the augmenting path terminates with the num vertex, the path searched succeeds.
        if(partial_matching->at(i)->num == num){
            found = findAugPathFromVarRe(graph, partial_matching,var, partial_matching->at(i)->var);
            if(found){
                partial_matching->at(i)->direction = 0;
            }
            break;
        }        
    }
    return found;
}

bool findAugPathFromVar(vector<MyEdge*> *graph, vector<MyEdge*>* partial_matching, Variable* var, int point){
    bool found = false;
    for(int i = 0; !found && i < var->currDM->at(point).size(); i++){
        int num = var->currDM->at(point).at(i);
        bool = findAugPathFromNumRe(graph, partial_matching, var, num);
        if(found){
            for(int j = 0; j < graph->size(); j++){
                if(graph->at(j)->var == var && graph->at(j)->num == num){
                    graph->at(j)->direction = 1;
                    partial_matching->push_back(graph->at(j));
                    break;
                }
            }
        }
    }
    return found;
}



void buildMaxMatchingFrom(vector<MyEdge*>* graph, vector<MyEdge*>* partial_matching, vector<MyEdge*>* max_matching, VariableQueue* variables, int point){
    //find unmatched variables
    deque<Variable*>  unmatched_vars = deque<Variable*>();
    for(int i = 0; i < variables->size(); i++)
    {
        Variable* var = variables->at(i);
        bool in_match = false;
        for(int j = 0; !in_match && j < partial_matching->size(); j++)
        {
            if(var == partial_matching->at(j)->var){ 
                in_match = true;
            }        
        }
        if(!in_match){
            unmatched_vars.push_back(var);
        }
    }
    //for every unmatched variable, backtrace an alternating path and try a max matching
    bool found = true;
    while(found && !unmatched_vars.empty()){
        Variable *var = unmatched_vars.front();
        found = findAugPathFromVar(graph, partial_matching, var);
        if(found){
            unmatched_vars->pop_front();
        }else{
            // some varaible cannot be added into matching without conflict
            max_matching = new vector<int>(); 
        }
    }
    if(found){
       max_matching = partial_matching;
    }
}

//max_matching is ideally the matching covering variable vertice
//if no maximum matching, return empty mathing
vector<MyEdge* >* computeMaxMatch(vector<MyEdge*>* graph, VariableQueue* variables, int point){
    vector<int> domain = vector<int>();  
    vector<MyEdge*> *max_matching = new vector<MyEdge*> ();
    vector<MyEdge*> *partial_matching = new vector<MyEdge*>(); 
    for(int i = 0; i < variables->size(); i++){
        Variable * var = variables->at(i);
        for(int j = 0; j < var->currDM->at(point).size(); j++){
            domain.push_back(var->currDM->at(point).at(j));
        }
    }
    deleteDup(domain);
    if(domain.size() < variables->size())
    {
        return max_matching;
    } else{
        partial_matching = greedilyFindMatch(graph);
        if(partial_matching->size() < variables->size()){
            //the matching does not covers variables, trying to find maximum matching using the partial matching
            buildMaxMatchingFrom(graph, partial_matching, max_matching, variables, point);
            if(max_matching->size()==0){
                myFree(partial_matching);
            }
            return max_matching;
        } else if(partial_matching->size == variables->size()){
            //the matching covers variable vertice
            max_matching = partial_matching;
        }
    }

    return max_matching;    
}





//initialise all edges between variables and domain num, direction is from num to variable vertex 
vector<MyEdge*>  *buildGraph(Constraint *constr, int point){
    VariableQueue variables = constr->variables;
    vector<MyEdge> * graph = new vector<MyEdge>();
    for(int i = 0; i < variables->size(); i++){
        Variable * var = variables->at(i);
        vector<int>* temp_dm = &(var->currDM->at(point));
        for(int j = 0; j < temp_dm->size(); j++){
            MyEdge* temp = newMyEdge(var, temp_dm->at(j));
            graph->push_back(temp);
        }
    }
    return graph;
}

//greedily assign variable vertex with domain num unmatched with other varaible vertex
//if no such num, bypass the variable num and return the matching
//num_of_var is the number of variables, which should be the upper bound of the size of matching
vector<MyEdge*>* greedilyFindMatch(vector<MyEdge*> *graph){
    vector<Variable* > matchedVars =vector<Variable *>();
    vector<int> matchedNums = vector<int>(); 
    vector<MyEdge*>* matching = new vector<MyEdge*>();
    for(int i = 0; i < graph->size();i++){
        if(graph->at(i)->direction == 0){
            Variable var = graph->at(i)->var;
            int num = graph->at(i)->num;
            bool found = variableQueueFind(&matchedVars, var);
            if(found){
                continue;
            } else{
                for(int j = 0; !found && j < matchedNums.size(); j++){
                    if(matchedNums.at(j) == num){
                        found = true;
                    }
                }
            }
            if(!found){
                graph->at(i)->direction = 1;
                matchedVars.push_back(var);
                matchedNums.push_back(num);
                matching->push_back(graph->at(i));
            }
        }
    }
    return matching; 
}



vector<MyEdge*>* removeEdgeFromG(vector<MyEdge*> *graph, vector<MyEdge*>* matching){
    //mark all edges in graph as unused
    for(int i = 0; i < graph->size(); i++){
        graph->at(i)->used = false;
    }
    vector<MyEdge*> *removed_edges = new vector<MyEdge*>();

    //do BFS starting from free vertices, that is, free num vertices
    //mark all traversed edges as used
    deque<MyEdge*> BFS_queue = deque<MyEdge*>();
    vector<int> num_in_match = vector<int>();
    for(int i = 0; i < matching->size(); i++){
        num_in_match.push_back(matching->at(i)->num);
    }
    for(int i = 0; i < graph->size(); i++){
        bool in_match = false;
        for(int j = 0; !in_match && j < num_in_match.size(); j++){
            if( num_in_match.at(j)== graph->at(i)->num){
                in_match = true;
            }
        }
        if(!in_match){
            graph->at(i)->used = true;
            BFS_queue.push_back(graph->at(i));
        }
    }
    while(!BFS_queue.empty){
        MyEdge* edge = BFS_queue.front();
        BFS_queue.pop_front();
        if(edge.direction == 0){
            for(int i = 0; i < graph->size(); i++){
                if((!graph->at(i)->used) && graph->at(i)->var == edge->var && graph->at(i)->direction == 1){
                    graph->at(i)->used = true;
                    BFS_queue.push_back(graph->at(i));
                }
            }
        } else{
            for(int i = 0; i < graph->size(); i++){
                if((!graph->at(i)->used) && graph->at(i)->num == edge->num && graph->at(i)->direction == 0){
                    graph->at(i)->used = true;
                    BFS_queue.push_back(graph->at(i));
                }
            }
        }
    }

    //compute the strongly connected components of graph, mark as used
    computeSCC(graph, matching); 
    
    //mark vital edges and build removed_edges
    for(int i = graph->size()-1; i >= 0 ; i--){
        if(!graph->at(i)->used){
            for(int j = 0; j < matching->size(); j++){
                if(matching->at(j)->var == graph->at(i)->var && matching->at(j)->num == graph->at(i)->num){
                    graph->at(i)->vital = true;``
                }
            }
            if(!graph->at(i)->vital){
                removed_edges->push_back(graph->at(i));
                graph->erase(graph->begin + i);
            }
        }
    }
    return removed_edges;
}

bool computeSCCFromRe(vector<MyEdge*>*graph, MyEdge* start_edge, MyEdge* curr_edge, vector<MyEdge*>* my_stack){
    bool found = false;
    bool has_cycle = false;
    for(int i = 0; i < graph->size(); i++){
        if(curr_edge->direction == 1 && graph->at(i)->num == curr_edge->num && graph->at(i)->direction == 0){
            my_stack->push_back(graph->at(i));
            found = computeSCCFromRe(graph, start_edge, graph->at(i),my_stack); 
            if(found){
                curr_edge->used = true;
                has_cycle = true;
            }
            my_stack->pop_back();
        } else if(curr_edge->direction == 0){
            if(curr_edge->var == start_edge->var){
                curr_edge->used = true;
                return true;
            } else if(graph->at(i)->var == curr_edge->var && graph->at(i)->direction == 1){
                my_stack->push_back(graph->at(i));
                found = computeSCCFromRe(graph, start_edge, graph->at(i), my_stack);
                if(found){
                    curr_edge->used = true;
                    has_cycle = true;
                }
                my_stack->pop_back();
            }
        }
    }
    return has_cycle;
}

bool computeSCCFrom(vector<MyEdge*>* graph, MyEdge* start_edge){
    bool found = false;
    vector<MyEdge*>* my_stack = new vector<int>();
    my_stack->push_back(start_edge);
    found = computeSCCFromRe(graph, start_edge,start_edge, my_stack);
    while(!my_stack->empty())
    {
        my_stack->pop_back();
    }
    return found;
}

void computeSCC(vector<MyEdge*>* graph, vector<MyEdge*> *matching){
    for(int i = 0; i < matching->size(); i++){
        if(!matching->at(i).used){
            bool found = computeSCCFrom(graph, matching->at(i));
            if(found){
                matching->at(i).used = true; //should be done in computeSCCFrom, not necessary?
            }
        }
    }
}

/*
void findCricle(vector<MyEdge> &graph, Constraint *constr){
	for(int i = 0; i < constr->variables.size(); i++){
		Variable *var = constr->variables.at(i);
		vector<MyEdge> outEdges = vector<MyEdge>();
		findSuccVar(graph, var, outEdges);
		for(int j = 0; j < outEdges.size(); j++){
			MyEdge edge = outEdges.at(j);
			if(!edge.used){
				bool found = findCircleRe(var,edge);
				if(found){
					edge.used = true;
				}
			}
		}
	}
}

bool findCircleRe(Variable *startPoint, MyEdge &edge){
	bool found = false;
	if(edge.direction == 1){
		vector<MyEdge> outEdges = vector<MyEdge> (); 
		findSuccNum(edge.num, outEdges);
		for(int i = 0; !found && i < outEdges.size(); i++){
			found = findCircleRe(startPoint,outEdges.at(i));
		}
	} else if(edge.direction == 0){
		if(edge.var == startPoint){
			found = true;
			edge.used = true;
			return found;
		}
	}
	if(found){
		edge.used = true;
	}	
	return found;
}
*/
