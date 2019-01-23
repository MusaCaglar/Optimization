#ifndef _BS_H_
#define _BS_H_
#define max(a, b) ((a) > (b) ? (a) : (b))


#define EPS 0.0000001
#include <math.h>
#include <time.h> 
#include <ilcplex/ilocplex.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib> 
#include <fstream>
#include <iomanip>
//#include "stdafx.h"
#include <string>
#include <vector>
using namespace std;

#include<stdio.h>
#include<conio.h>
//#include<stdlib.h> 

ILOSTLBEGIN

/* The following 3 declarations are for use of the random-number generator
   lcgrand and the associated functions lcgrandst and lcgrandgt for seed
   management.  This file (named lcgrand.h) should be included in any program
   using these functions by executing
       #include "lcgrand.h"
   before referencing the functions. */

float lcgrand(int stream);
void  lcgrandst(long zset, int stream);
long  lcgrandgt(int stream);

typedef IloArray<IloNumVarArray> IloNumVarArray1;
typedef IloArray<IloNumArray> IloNumArrayArray; 


void quicksort(int x[2000],int first,int last);
//void knap(int d,int s, int j);
//void DP(int cap,int gamma,int n);

//void DP(int cap,int gamma,int n, int pr[], int we[]);

#endif //_BS_H_
