#include "LayersMap.h"

LayersMap::LayersMap(std::string layers_config){
	//this contains the xml reader
}

LayersMap::~LayersMap(){
}

int Map_translate(long64 old_id){
	//it can be done in a really smarter way using a std::list
	 int i = 0;
	while (i<lay_ids.size() && old_id !=lay_ids[i].first())
		i++;
	if (i == lay_ids.size()){
		std::cout<<"ERROR the layer ID is not in the map"<<std::endl;
		return -1;
	}
	return lay_ids[i].second();
}

int LayersMap::DetIDClassification(HitSet sHit){
	
	long64 id0 = sHit[0]->getCellID0();
	long64 id1 = sHit[0]->getCellID0();
	long64 id2 = sHit[0]->getCellID0();

int new_id0 = Map_translate(id0);
int new_id1 = Map_translate(id1);
int new_id2 = Map_translate(id2);

int identifier = 100*new_id2+10*new_id1+new_id0;

//just a check
int i = 0;
while (i<trip_ids.size() && identifier !=trip_ids[i]){
	i++;
if(i == trip_ids.size()){
		std::cout<<"ERROR impossible triplet"<<std::endl;
		return -1;
	}
return identifier;
}


std::vector<std::pair<int, int>> LeftList(){
	//again a reader from xml? 
}