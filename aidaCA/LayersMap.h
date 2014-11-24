#ifndef LayersMap_h
#define LayersMap_h

//paths to be changed 
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <string>
#include <stdexcept>

// Error codes

enum {
   ERROR_ARGS = 1, 
   ERROR_XERCES_INIT,
   ERROR_PARSE,
   ERROR_EMPTY_DOCUMENT
};

//C++ class for the layers
class CAlayer{
public:
	int sequence_number;				//index in the CAlayer vector
	int layerid; 
	int cellid;
		
};

//C++ class for the layer triplets
class CAlaylist{
public:
	int listid;
	int layids[3];
	int left;	
	
};

//to read the xml configuration files
class GetConfig
{
public:
   GetConfig();
  ~GetConfig();
   void readConfigFile(std::string&) throw(std::runtime_error);
 
   std::vector<CAlayer> *getLayers() { return &CALayers; };
   std::vector<CAlaylist> *getLayLists() { return &CALayList; };

private:
   xercesc::XercesDOMParser *m_ConfigFileParser;
   char* m_LayerID;
   char* m_CellID;   
   char* m_ListID;
   char* m_Lay1ID;   
   char* m_Lay2ID;   
   char* m_Lay3ID;   
   char* m_Left;      


   XMLCh* TAG_root;
   XMLCh* TAG_Layers;
   XMLCh* TAG_Layer;
   XMLCh* TAG_LayLists;
   XMLCh* TAG_LayList;
   XMLCh* TAG_LayerID;
   XMLCh* TAG_CellID;   
   XMLCh* TAG_ListID;
   XMLCh* TAG_Lay1ID;   
   XMLCh* TAG_Lay2ID;   
   XMLCh* TAG_Lay3ID;   
   XMLCh* TAG_Left;      

   std::vector<CAlayer> CALayers;
   std::vector<CAlaylist> CALayList;

};


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
   vector<CAlayer> *glayers;
   vector<CAlaylist> *glayerslists;
};

#endif