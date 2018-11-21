#include "variable.h"
#include<iostream>
#include<vector>

using namespace std;

typedef struct MyEdge_{
	Variable *var;
	int num;
	int direction;
	bool vital;
	bool used;
}MyEdge;

MyEdge* newMyEdge(Variable* var, int num){
    MyEdge* e = (MyEdge*)myMalloc(sizeof(MyEdge));
    e->var = var;
    e->num = num;
    e->direction = 0;
    e->vital = false;
    e->used = false;
    e->visited = false;
    return e;
}
