#ifndef LayersMap_h
#define LayersMap_h

class LayersMap{
public:
	//Define the map starting from the possible layer configuration xml file
	LayersMap(std::string layers_config);	
	~LayersMap();
	//use a new  easy layer definition
	int Map_translate(long64);
	
	int DetIDClassification(HitSet);
	std::vector<std::pair<int, int>> LeftList();

private:
	//layer ids according to the xml dictionary 
	std::vector<std::pair<long64,int>>  lay_ids;
	//consecutive triplet ids according to the xml connection
	std::vector<int>  trip_ids;		
};


#endif