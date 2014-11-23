#ifndef CAHitsGenerator_h
#define CAHitsGenerator_h

#ifdef USE_DD4HEP
#ifdef USE_LCIO


// c++
#include <iostream>
#include <map>
#include <string>

// lcio
#include "lcio.h"
#include "IO/LCWriter.h"
#include "EVENT/LCEvent.h"
#include "EVENT/LCCollection.h"
#include "EVENT/SimTrackerHit.h"
#include "UTIL/ILDConf.h"

#include <IMPL/LCCollectionVec.h>
#include "IMPL/TrackImpl.h"

// DD4hep
#include "DD4hepGeometry.hh"
#include "DD4hep/LCDD.h"
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/SurfaceManager.h"


// aidaTT
#include "AidaTT.hh"
#include "ConstantSolenoidBField.hh"
#include "analyticalPropagation.hh"
#include "simplifiedPropagation.hh"
#include "GBLInterface.hh"
#include "fitResults.hh"

//root
#include "TVector3.h"

#include <unordered_map>

//CA includes
#include "LayersMap.h"
#include "CACell.h"
#include "HitSet.h"

//
// CAHitsGenerator class declaration
//


class CAHitsGenerator{
public:
    
      CAHitsGenerator(int debug, bool maketrees, double EtaCut, LayersMap LayMap);
      ~CAHitsGenerator();
    
      void init(std::string);
      void hitSets(std::vector<HitSet>);
	  
	//Generate CA cells from the produced triplets
    void CAcellGenerator(HitSet,  int);
	//Fit neighborly cells
	void fitTripletSeeds(CAcell*, aidaTT::ConstantSolenoidBField*, aidaTT::DD4hepGeometry*);
	//Forward step of the CA algo
    int ForwardCA();
	//Backward step of the CA algo
	void BackwardCA();
	//mark a cell as "used" by the CA
	CAcell* define_used(CAcell*, int, HitSet*);
	
	/**Some useful functions**/
 	//Intersect the neighbor lists of two cells (used in the Neighborhood map definition)
    std::vector<CAcell *> ListIntersect(std::list<CAcell *>, std::list<CAcell *>);

	//Delta eta
	double DeltaEta(GlobalPoint, GlobalPoint);
	//avoid duplicates
	int IsAtLeft(int identif);

	//maximum CA_status
	int max_status; //Now set to 10, in general to build n-tuplets set to > n-2

private:
    
        //Debug switch (set in the _cfi  ==0 no debud output, ==1 text output, ==2 text+debug tree, ==3 +time measurements)
        int m_debug;
        bool m_tree;
 	
        //CAcell collection
        std::vector<CAcell> tripletCollection;
        std::vector<CAcell*> fittedTripletCollection;
    
		//he geometry 
    	aidaTT::DD4hepGeometry geom;
		//map of surfaces
    	std::map< long64, const aidaTT::ISurface* > surfMap ;
		//constant field parallel to z, 1T
    	aidaTT::ConstantSolenoidBField*  bfield;
    	
    	//theLayersMap
    	LayersMap theLayersMap;
    
    double dEta_cut;

	int eventcounter;
	std::vector<HitSet>multiplets;

    //data structure for the neighborhood maps
    std::unordered_map<unsigned int, std::list<CAcell *>> hitUsage;
	
	bool makehistos;
};

#endif
#endif

#endif
