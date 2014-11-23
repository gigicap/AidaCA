#pragma once

// system include files
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <queue>
#include <iostream>
#include <set>
#include <limits>
#include <array>
#include <algorithm>
#include <math.h>
#include <unordered_map>

//Class CAcell definition
class CAcell{
    
public:
	//CAcell hits (from seeds)
  	HitSet hits;

    
	bool FitSuccessful;

	double LocalMomentum;

	int eventNumber;
    
    //triplet identifier e.g. (1,2,3) =  123
    int tripletIdentifier;

	int hitId0;
	int hitId1;
	int hitId2;

    GlobalPoint p0 ,p1, p2;
    
    double dEta;
    
    unsigned int rawId0;
    unsigned int rawId1;
    unsigned int rawId2;
    
    //neighborMAP
    std::vector<CAcell *> left_neighbors;       //list of the cells sharing hits 0-1 with the cell
    std::vector<CAcell *> right_neighbors;      //list of the cells sharing hits 1-2 with the cell

    //How many neighbors does the cell have?
    int IsNeighborly;
    
    
    //just for convenience and debug purposes
    int TripletNumber;

    //for the backwardCA
    int IsUsed;
    
    //for fitting
    PTrajectoryStateOnDet seedStartState;
    
    CAcell(){
        TripletNumber = 0;
        CAstatus = 0;
        tripletIdentifier = 0;
        }

    

double getLocalMomentum(){ return LocalMomentum;}
int geteventNumber(){return eventNumber;}
    
int gethitId0(){return hitId0;}
int gethitId1(){return hitId1;}
int gethitId2(){return hitId2;}

    
//CAstuff
    int CAstatus;
        
};
