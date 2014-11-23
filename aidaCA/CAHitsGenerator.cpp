// Class:      	CAHitsGenerator 
//	File:			CAHitsGenerator.cpp 
//
//  Author:  Gigi Cappello

// system include files
#include "CAHitsGenerator.h"  //class CAcell defined here
#include <memory>
    
using namespace std;
using namespace lcio;

CAHitsGenerator::CAHitsGenerator(int debug, bool maketrees, double EtaCut, LayersMap LayMap) 
    {

     m_debug = debug;
     m_tree = maketrees;		//to be re-implemented. Up to now, this flag has no effects
     dEta_cut = EtaCut;
     
        	
    eventcounter = 0;
    max_status = 3;
	
	theLayersMap = Laymap;
}


CAHitsGenerator::~CAHitsGenerator()
{
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void CAHitsGenerator::init(std::string inFile)
{
    if (m_debug >1)
    	std::cout<<"Begin init---"<<std::endl;
    

//get the geometry 
    DD4hep::Geometry::LCDD& lcdd = DD4hep::Geometry::LCDD::getInstance();
    lcdd.fromCompact(inFile);
    DD4hep::Geometry::DetElement world = lcdd.world() ;
    geom(world);
// create map of surfaces
    const std::list<const aidaTT::ISurface*>& surfaces = geom.getSurfaces() ;
    for(std::list<const aidaTT::ISurface*>::const_iterator surf = surfaces.begin() ; surf != surfaces.end() ; ++surf)
            surfMap[(*surf)->id() ] = (*surf) ;
// first a constant field parallel to z, 1T
   	bfield = new aidaTT::ConstantSolenoidBField(3.5);
    
    
return;
}

//hitSets si crea evento per evento (dentro il loop di lettura degli eventi
void CAHitsGenerator::hitSets(std::vector<HitSet> resultCA)	
		//object to be defined std::vector<HitSet>. HitSet = std::vector<SimTrackerHit> (dim [5] o in generale m = n+2) to be defined -> da trasformare in aidaTT::trajectory
{

	
	//BASE ONE 
	HitTriplets *scoll = new HitTriplets();		//object to be defined std::vector<HitSet>. HitSet = std::vector<SimTrackerHit> (dim [3]) to be defined
	scoll->reserve(10000);
         
    if (m_debug > 1)  std::cout<<"Generating triplets"<<std::endl;
    
    //SLICE OF CODE FOR TRIPLET PRODUCTION HERE...
	/*																								*/
	//    ----!!
    
    tripletCollection.reserve(scoll->size());
    fittedTripletCollection.reserve(scoll->size());

        

    for (size_t is = 0; is<scoll->size(); is++) {
        CAcellGenerator(scoll[is], is); 
        
        hitUsage[tripletCollection[is].rawId0].push_back(&tripletCollection[is]);
        hitUsage[tripletCollection[is].rawId1].push_back(&tripletCollection[is]);
        hitUsage[tripletCollection[is].rawId2].push_back(&tripletCollection[is]);
        
    }
    
      
//Loop over the triplets to build the neighborhood map
//i.e. fill the "HasNeighbor" var and IsNeighborly
int ncont = 0;
for (size_t it = 0; it<tripletCollection.size(); it++) {
    int IsNeighbor = 0;
    
    
        std::list<CAcell *> list_h0 = hitUsage[tripletCollection[it].rawId0];
        std::list<CAcell *> list_h1 = hitUsage[tripletCollection[it].rawId1];
        std::list<CAcell *> list_h2 = hitUsage[tripletCollection[it].rawId2];
    
    
        std::vector<CAcell *> j_list01 = ListIntersect(list_h0, list_h1);
        std::vector<CAcell *> j_list12 = ListIntersect(list_h1, list_h2);
    
    
    if(m_debug>2){
    std::cout<<"list_h0 size == "<<list_h0.size()<<std::endl;
    std::cout<<"list_h1 size == "<<list_h1.size()<<std::endl;
    std::cout<<"list_h2 size == "<<list_h2.size()<<std::endl;
	std::cout<<"----------------------------------"<<std::endl;
    std::cout<<"j_list01 size == "<<j_list01.size()<<std::endl;
    std::cout<<"j_list12 size == "<<j_list12.size()<<std::endl;
    }


     for (size_t il = 0; il<j_list01.size(); il++){
     	//added to avoid duplicates in layer superimpositions
     	if((j_list01[il] != &tripletCollection[it]) && (j_list01[il]->tripletIdentifier == IsAtLeft(tripletCollection[it].tripletIdentifier))){ 		
         if((j_list01[il]->tripletIdentifier != tripletCollection[it].tripletIdentifier) && fabs(j_list01[il]->dEta - tripletCollection[it].dEta)< dEta_cut)
             tripletCollection[it].left_neighbors.push_back(j_list01[il]);
     	}
     }
    
    
    for (size_t il = 0; il<j_list12.size(); il++){  
    	//Do I also have to add a IsAtRight function (more complicated)? 
        if(j_list12[il] != &tripletCollection[it] && (j_list12[il]->tripletIdentifier != tripletCollection[it].tripletIdentifier) && fabs(j_list12[il]->dEta - tripletCollection[it].dEta)< dEta_cut)
            tripletCollection[it].right_neighbors.push_back(j_list12[il]);
    }
    
    if (m_debug > 4 ){ 
    std::cout<<"left_list size == "<<tripletCollection[it].left_neighbors.size()<<std::endl;
    std::cout<<"right_list size == "<<tripletCollection[it].right_neighbors.size()<<std::endl;
    }
    
    IsNeighbor = tripletCollection[it].left_neighbors.size()+tripletCollection[it].right_neighbors.size();
    tripletCollection[it].IsNeighborly = IsNeighbor;
    
    if(m_debug> 2)
        std::cout<<"triplet has = "<<tripletCollection[it].IsNeighborly<<" neighbors. Is in lays: "<<tripletCollection[it].tripletIdentifier<<std::endl;
    
    
    list_h0.clear();
    list_h1.clear();
    list_h2.clear();
    j_list01.clear();
    j_list12.clear();
    
	//End of the neighborhood map definition. Info stored in the cells
    
    //fit only the triplets with neighbors
    if (tripletCollection[it].IsNeighborly != 0) {
        ncont++;
    // fast helix fitter
        fitTripletSeeds(&tripletCollection[it], bfield, &geom);
 	//store the neighborly and fitted triplets
    	if (tripletCollection[it].FitSuccessful) {
            tripletCollection[it].CAstatus = 1;     //initialize CAstatus
            fittedTripletCollection.push_back(&tripletCollection[it]);
        }
    }
        

}

//Loops on fittedtriplets -> Forward CA
    int n_fiter = -1;
    n_fiter = ForwardCA();
    
    if (m_debug > 1) std::cout<<" -- > ForwardCA done!"<<std::endl;
    
//Connect triplets into multiplets Backward CA
    BackwardCA();
    
        if (m_debug > 1) std::cout<<" -- > BackwardCA done!"<<std::endl;

    
int nSeeds = 0;

for(size_t im = 0; im < multiplets.size(); im++){
	//std::vector<TransientTrackingRecHit::ConstRecHitPointer> multiHitPointer;
    std::vector<ConstRecHitPointer>  multiHitPointer;
	for(size_t ii = 0; ii < multiplets[im].size(); ii++)
		multiHitPointer.push_back(multiplets[im][ii]);
	if(multiHitPointer.size()!=5)
		std::cout<<"WARNING: multiset size = "<<multiHitPointer.size()<<std::endl;
	else{
    		SeedingHitSet multiset(multiHitPointer[4], multiHitPointer[3], multiHitPointer[2], multiHitPointer[1], multiHitPointer[0]);
			resultCA.push_back(multiset);
			nSeeds++;
	}
}

std::cout<<"Final number of multiSeeds = "<<resultCA.size()<<std::endl;

if(m_debug > 0){
std::cout<<"================================"<<std::endl;
std::cout<<"Processed "<<scoll->size()<<" seeds"<<std::endl;
std::cout<<"produced  "<<tripletCollection.size()<<" CAcells, of whom:"<<std::endl;
std::cout<<ncont<<" have neighbors"<<std::endl;
std::cout<<fittedTripletCollection.size()<<" have been successfully fitted!"<<std::endl;
std::cout<<"Forward iterations took n_steps = "<<n_fiter<<std::endl;
std::cout<<"Final number of multiSeeds = "<<resultCA.size()<<std::endl;
std::cout<<"================================"<<std::endl;
}


//try with swap to actually un-allocate memory 
    delete scoll;
    tripletCollection.clear();
	std::vector<CAcell>().swap(tripletCollection);
    fittedTripletCollection.clear();
	std::vector<CAcell*>().swap(fittedTripletCollection);
    hitUsage.clear();

    multiplets.clear();

eventcounter++;
//std::cout<<"last row: event counter #"<<eventcounter<<std::endl;
}

void CAHitsGenerator::CAcellGenerator(HitSet sset,  int seednumber){
    //Build the CA cells, i.e. triplets (objects containing Hits and fit parameters

   	CAcell trip;
   	trip.hits = sset;   
   if (m_debug>2) std::cout << " starting triplet size:  "<<starting_triplet.size()<< std::endl;
  
    
      trip.rawId0 = sset[0]->getCellID0();
      trip.rawId1 = sset[1]->getCellID0();
      trip.rawId2 = sset[2]->getCellID0();

     // setting global points
	//How do i implement the global position in aidaTT ? 
    trip.p0 = sset[0]->globalPosition();			//(x,y,z) absolute coordinates
    trip.p1 = sset[1]->globalPosition();
    trip.p2 = sset[2]->globalPosition();
    
    trip.dEta = DeltaEta(p0,p2);			//to be rewritten as consequence
    
    // setting triplets IDs
    trip.eventNumber = eventcounter;
    trip.tripletIdentifier = theLayersMap.DetIDClassification(sset);	//to be written
    //theLayerMap is the macro data structure to manage the layers (mostly by xml)
    
    if (m_debug>4) {
        std::cout << " layer 0 =  " << trip.hitId0 << std::endl;
        std::cout << " layer 1 =  " << trip.hitId1 << std::endl;
        std::cout << " layer 2 =  " << trip.hitId2 << std::endl;
        std::cout << " identifier = "<< trip.tripletIdentifier <<std::endl;
    }
    
    
    trip.TripletNumber = seednumber;
	trip.FitSuccessful = false;
	trip.LocalMomentum = 0;

	//Is Used (for the final Backwards CA)
	trip.IsUsed = 0;
	tripletCollection.push_back(trip);


	return; 
}

void CAHitsGenerator::fitTripletSeeds(CAcell *trip, aidaTT::ConstantSolenoidBField* mf, aidaTT::DD4hepGeometry *geom) {
 // fit performed by aidaTT 
 aidaTT::trackParameters tripTP;
 aidaTT::GBLInterface* fitter = new aidaTT::GBLInterface();
 aidaTT::analyticalPropagation* propagation = new aidaTT::analyticalPropagation();
 
aidaTT::trajectory theTripTraj(tripTP, fitter, bfield, propagation, geom);

HitSet Hits = trip->hits;
int nHit = Hits.size();

for(int i = 0 ; i < nHit ; ++i)					
          {
           SimTrackerHit* sHit = &Hits[i] ;
           long64 id = sHit->getCellID0() ;
           idDecoder.setValue(id) ;					//idDecoder to be set at init stage
           double recalcPos[3] = {0., 0., 0.}; 
           const aidaTT::ISurface* surf = surfMap[ id ] ;
           if(surf->type().isSensitive())
                  {
                        std::vector<double> precisionDummy;
                        precisionDummy.push_back(1. / 0.001);
                        precisionDummy.push_back(1. / 0.0012);
                        for(unsigned int i = 0; i < 3; ++i)
                             recalcPos[i] = sHit->getPosition()[i] * dd4hep::mm;
                                    theTripTraj.addMeasurement(recalcPos, precisionDummy, *surf, sHit);
                                }
                        }
//whatelse? 
theTripTraj.prepareForFitting();
theTripTraj.fit();			//fitta la traiettoria 

const aidaTT::fitResults& result = theMaster.getFitResults();
trip->LocalMomentum = result.estimatedParameters()(0); //where is the pt?


return;
}



int CAHitsGenerator::ForwardCA(){
    int Stop = -1;
    int step_iterator = 0;
    
    int it_max = 20; //Maximum number of iteraitons
        
    while (Stop < (int)fittedTripletCollection.size() && step_iterator<it_max) { 
        step_iterator++;
        Stop = 0;
        
        
        for (size_t t =0; t<fittedTripletCollection.size(); t++) {
            
            if (fittedTripletCollection[t]->left_neighbors.size()!=0) {
               bool en_cont = false;                       //useful variable - moved from 2 lines behind 
               for (size_t il = 0; il<fittedTripletCollection[t]->left_neighbors.size(); il++) {
                    if (fittedTripletCollection[t]->CAstatus <= fittedTripletCollection[t]->left_neighbors[il]->CAstatus && en_cont == false){
                        	(fittedTripletCollection[t]->CAstatus)++;
                      	 	en_cont = true;
                    			}
                }
                if (en_cont == false)  Stop++;  
            }
            else  Stop++;		//i.e. if fittedTripletCollection[t] does not have left neighbors
        }   //end of loop on triplets            
    }		//end of while
    
    //debug
if(m_debug>2){
    for (size_t t =0; t<fittedTripletCollection.size(); t++) {
        std::cout<<" ._._._._._._._._ "<<std::endl;
        std::cout<<" cell number "<<t<<std::endl;
        std::cout<<" has = "<<fittedTripletCollection[t]->IsNeighborly<<" neighbors. Is in lays: "<<fittedTripletCollection[t]->tripletIdentifier<<std::endl;
        std::cout<<"left_list size == "<<fittedTripletCollection[t]->left_neighbors.size()<<std::endl;
       	std::cout<<"left neighbours (Id, eta) == {";
        for(size_t k=0; k< fittedTripletCollection[t]->left_neighbors.size(); k++){
        	std::cout<<"(";
        	std::cout<<(fittedTripletCollection[t]->left_neighbors[k])->tripletIdentifier;
        	std::cout<<(fittedTripletCollection[t]->left_neighbors[k])->dEta;        	
        	std::cout<<"),";
        }
        std::cout<<"}"<<std::endl;
    	std::cout<<"right_list size == "<<fittedTripletCollection[t]->right_neighbors.size()<<std::endl;
    	std::cout<<"right neighbours (Id, eta) == {";
        for(size_t k=0; k< fittedTripletCollection[t]->right_neighbors.size(); k++){
        	std::cout<<"(";
        	std::cout<<(fittedTripletCollection[t]->right_neighbors[k])->tripletIdentifier;
        	std::cout<<(fittedTripletCollection[t]->right_neighbors[k])->dEta;        	
        	std::cout<<"),";
        }
        std::cout<<"}"<<std::endl;
        std::cout<<" processed with status: "<<fittedTripletCollection[t]->CAstatus<<std::endl;
    }
    }
    
    if(step_iterator>=it_max)
        std::cout<<"WARNING: too many CA iterations!"<<std::endl;
    
    return step_iterator;
}


void CAHitsGenerator::BackwardCA(){

int multi_id = 1;

for (size_t t =0; t<fittedTripletCollection.size(); t++) {
//start only with non-used triplets with maximum CAstatus
if (m_debug > 2) std::cout<<"Processing triplet no. "<<t<<" for the backwardCA"<<std::endl;
if(fittedTripletCollection[t]->CAstatus == max_status && fittedTripletCollection[t]->IsUsed==0){
	CAcell *current_cell = fittedTripletCollection[t];
	//TransientTrackingRecHit::RecHitContainer multicontainer;
	std::vector<ConstRecHitPointer> multicontainer;
	while(current_cell){
		current_cell = define_used(current_cell, multi_id, &multicontainer);
		}
		multi_id++;	
	if (m_debug > 2) std::cout<<"Multicontainer size =  "<<multicontainer.size()<<std::endl;
	multiplets.push_back(multicontainer);
	multicontainer.clear();
	}
}

return;
}

CAcell* CAHitsGenerator::define_used(CAcell *cell, int id, HitSet*multicontainer){
cell->IsUsed = id;
double pt_tmp = 99999.0;

if(cell->CAstatus==1){
	multicontainer->push_back(cell->hits[2]);
	multicontainer->push_back(cell->hits[1]);
	multicontainer->push_back(cell->hits[0]);
return NULL;	
}

else{
CAcell *next = new CAcell();
multicontainer->push_back(cell->hits[2]);
	for(size_t il = 0; il< cell->left_neighbors.size(); il++){
		if(cell->CAstatus - cell->left_neighbors[il]->CAstatus == 1){
		double pt_diff = fabs(cell->getLocalMomentum() - cell->left_neighbors[il]->getLocalMomentum());
		if(pt_diff <= pt_tmp){
			pt_tmp = pt_diff;
			next = cell->left_neighbors[il];
			} 
		}
	}
return next;
}

}


//Other functions

std::vector<CAcell *> CAHitsGenerator::ListIntersect(std::list<CAcell *> list_h1, std::list<CAcell *> list_h2){
    
    std::vector<CAcell *> temp;
    
    std::list<CAcell *>::const_iterator L1itr = list_h1.begin();
    std::list<CAcell *>::const_iterator L2itr = list_h2.begin();
    
    
    while(L1itr != list_h1.end() && L2itr != list_h2.end()){
        
        //check to see if they're equal, add to our temp list
        if(*L1itr == *L2itr){
            temp.push_back(*L1itr);
            L1itr++;
            L2itr++;
        }
        else if(*L1itr < *L2itr){
            L1itr++;
        }
        else{
            L2itr++;
        }
    }
    return temp;
}


double CAHitsGenerator::DeltaEta(GlobalPoint p1, GlobalPoint p2){
    //maybe it has to be redefined... 
    GlobalVector gV(p2.x()-p1.x(),p2.y()-p1.y(),p2.z()-p1.z());
    double eta = gV.eta();
    
    return eta;
}


//check is two triplets can be left-right joined 
//NB: It should be changed anlytime the Layer configuration is changed (This works for A-B-C-D)
int CAHitsGenerator::IsAtLeft(int identif){
//to be moved in 	theLayerMap
	std::vector<std::pair<int, int>> leftlist = theLayersMap.LeftList();
	int i = 0;
	while (i<leftlist.size() && identif !=leftlist[i].first())
		i++;
	if (i == leftlist.size()){
		std::cout<<"ERROR the layer ID is not in the map"<<std::endl;
		return -1;
	}
	int ret_val = leftlist[i].second();
	return ret_val;
}


//that's all folks!!!

