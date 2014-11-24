#include "LayersMap.h"

#include <string>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <vector>

using namespace xercesc;
using namespace std;

LayersMap::LayersMap(std::string layers_config){
	string configFile=layers_config; 

   GetConfig aConfig;
   aConfig.readConfigFile(configFile);
   
   std::cout<<"xml layer file read!"<<std::endl;
   
   glayers = appConfig.getLayers();
   glayerslists = appConfig.getLayLists();
   
   for(int i = 0 ; i<glayers->size(); i++){
   	std::pair laypair;
   	laypair.first = (long64)(*glayers)[i].cellid ;
   	laypair.second = (*glayers)[i].layerid;
   	lay_ids.push_back(laypair);
   }
   
   for(int i = 0 ; i<glayerslists->size(); i++){ 
   	  trip_ids.push_back((*glayerslists)[i].listid);
   }
   
}

LayersMap::~LayersMap(){
	~ aConfig();
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
	long64 id1 = sHit[1]->getCellID0();
	long64 id2 = sHit[2]->getCellID0();

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
	std::vector<std::pair<int, int>> leftlist;
	for(int i = 0 ; i<glayerslists->size(); i++){
		std::pair<int, int> isleft;
		isleft.first = (*glayerslists)[i].listid;
		isleft.second = (*glayerslists)[i].left;
		leftlist.push_back(isleft);
	}
return leftlist;
}


//XML PARSER STUFF
GetConfig::GetConfig()
{
   try
   {
      XMLPlatformUtils::Initialize();  // Initialize Xerces
   }
   catch( XMLException& e )
   {
      char* message = XMLString::transcode( e.getMessage() );
      cerr << "XML toolkit initialization error: " << message << endl;
      XMLString::release( &message );
   }


   TAG_root     = XMLString::transcode("root");
   TAG_Layers = XMLString::transcode("layers");
   TAG_Layer = XMLString::transcode("layer");
   TAG_LayLists = XMLString::transcode("laylists");
   TAG_LayList = XMLString::transcode("laylist");
   
   //elements of layer
   TAG_LayerID = XMLString::transcode("LayerID");
   TAG_CellID = XMLString::transcode("CellID");   
   //elements of laylist
   TAG_ListID = XMLString::transcode("ListID");
   TAG_Lay1ID = XMLString::transcode("lay1ID");   
   TAG_Lay2ID = XMLString::transcode("lay2ID");   
   TAG_Lay3ID = XMLString::transcode("lay3ID");   
   TAG_Left = XMLString::transcode("left");      
   
   m_ConfigFileParser = new XercesDOMParser;
}


GetConfig::~GetConfig()
{

   delete m_ConfigFileParser;

   try
   {
   XMLString::release( &TAG_root );
   XMLString::release( &TAG_Layers );
   XMLString::release( &TAG_Layer );
   XMLString::release( &TAG_LayLists);
   XMLString::release( &TAG_LayList);
         
   XMLString::release( &TAG_LayerID);
   XMLString::release( &TAG_CellID);   
   XMLString::release( &TAG_ListID );
   XMLString::release( &TAG_Lay1ID);   
   XMLString::release( &TAG_Lay2ID);   
   XMLString::release( &TAG_Lay3ID);   
   XMLString::release( &TAG_Left);      

   }
   catch( ... )
   {
      cerr << "Unknown exception encountered in TagNamesdtor" << endl;
   }

   // Terminate Xerces

   try
   {
      XMLPlatformUtils::Terminate();  // Terminate after release of memory
   }
   catch( xercesc::XMLException& e )
   {
      char* message = xercesc::XMLString::transcode( e.getMessage() );

      cerr << "XML ttolkit teardown error: " << message << endl;
      XMLString::release( &message );
   }
}


void GetConfig::readConfigFile(string& configFile)  throw( std::runtime_error )
{
   // Test to see if the file is ok.
	std::cout<<"--> In"<<std::endl;
   struct stat fileStatus;

   errno = 0;
   if(stat(configFile.c_str(), &fileStatus) == -1) // ==0 ok; ==-1 error
   {
       if( errno == ENOENT )     
          throw ( std::runtime_error("Path file_name does not exist, or path is an empty string.") );
       else if( errno == ENOTDIR )
          throw ( std::runtime_error("A component of the path is not a directory."));
       else if( errno == ELOOP )
          throw ( std::runtime_error("Too many symbolic links encountered while traversing the path."));
       else if( errno == EACCES )
          throw ( std::runtime_error("Permission denied."));
       else if( errno == ENAMETOOLONG )
          throw ( std::runtime_error("File can not be read\n"));
   }

   // Configure DOM parser.
	std::cout<<"--> Configure DOM parser."<<std::endl;

   m_ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
   m_ConfigFileParser->setDoNamespaces( false );
   m_ConfigFileParser->setDoSchema( false );
   m_ConfigFileParser->setLoadExternalDTD( false );

   try
   {
	std::cout<<"--> parse."<<std::endl;

      m_ConfigFileParser->parse( configFile.c_str() );

      DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();
      
      DOMElement* elementRoot = xmlDoc->getDocumentElement();
      if( !elementRoot ) throw(std::runtime_error( "empty XML document" ));

      DOMNodeList* layers_children = elementRoot->getElementsByTagName(TAG_Layer);
      const  XMLSize_t nodeCount1 = layers_children->getLength();

      std::cout<<"--> layers_children lenght = "<<nodeCount1 <<std::endl;

      int l_counter = 0;
      for( XMLSize_t xx = 0; xx < nodeCount1; ++xx )
      {      
      	DOMNode* node_layer = layers_children->item(xx);			//get every layer
      	CAlayer lay; 
      	 //subnodes of the layer 
      	 if( node_layer->getNodeType() &&  node_layer->getNodeType() == DOMNode::ELEMENT_NODE ) 
         			{
            		// Re-cast node as element
            		DOMElement* currentElement = dynamic_cast< xercesc::DOMElement* >( node_layer );
            		
					const XMLCh* xmlch_LayerID = currentElement->getAttribute(TAG_LayerID);
        			m_LayerID = XMLString::transcode(xmlch_LayerID);
        			int layid = atoi(m_LayerID);
        
					const XMLCh* xmlch_CellID = currentElement->getAttribute(TAG_CellID);
        			m_CellID = XMLString::transcode(xmlch_CellID);
      			    int cellid = atoi(m_CellID);
        
        
      			 	lay.layerid = layid;
        			lay.cellid = cellid;      	      	
      				lay.sequence_number = l_counter;
      				CALayers.push_back(lay);
      				l_counter++;
         			}
      }
      
      
       DOMNodeList* laylists_children = elementRoot->getElementsByTagName(TAG_LayList);
       const  XMLSize_t  nodeCount2 = laylists_children->getLength();

      	std::cout<<"--> laylists_children lenght = "<<nodeCount2<<std::endl;

      int ll_counter = 0;
      for( XMLSize_t xx = 0; xx < nodeCount2; ++xx )
      {      
      	DOMNode* node_laylist = laylists_children->item(xx);			//get every layer triplet
      	CAlaylist laylist; 
      
      	if( node_laylist->getNodeType() &&  node_laylist->getNodeType() == DOMNode::ELEMENT_NODE ) 
         			{
					// Re-cast node as element
            		DOMElement* currentElement = dynamic_cast< xercesc::DOMElement* >( node_laylist );
      	
					const XMLCh* xmlch_ListID = currentElement->getAttribute(TAG_ListID);
        			m_ListID = XMLString::transcode(xmlch_ListID);
        			int listid = atoi(m_ListID);

					const XMLCh* xmlch_Lay1ID = currentElement->getAttribute(TAG_Lay1ID);
        			m_Lay1ID = XMLString::transcode(xmlch_Lay1ID);
        			int lay1id = atoi(m_Lay1ID);
        
        			const XMLCh* xmlch_Lay2ID = currentElement->getAttribute(TAG_Lay2ID);
        			m_Lay2ID = XMLString::transcode(xmlch_Lay2ID);
        			int lay2id = atoi(m_Lay2ID);
        
  					const XMLCh* xmlch_Lay3ID = currentElement->getAttribute(TAG_Lay3ID);
        			m_Lay3ID = XMLString::transcode(xmlch_Lay3ID);
        			int lay3id = atoi(m_Lay3ID);
        
        			const XMLCh* xmlch_Left = currentElement->getAttribute(TAG_Left);
       			 	m_Left = XMLString::transcode(xmlch_Left);
        			int left = atoi(m_Left);

        			laylist.listid = listid;
        			laylist.layids[0] = lay1id;
       				laylist.layids[1] = lay2id;
        			laylist.layids[2] = lay3id;
        			laylist.left = left;
      	
      				CALayList.push_back(laylist);
      				ll_counter++;
         			}
      }      	
   }         
   catch( xercesc::XMLException& e )
   {
      char* message = xercesc::XMLString::transcode( e.getMessage() );
      ostringstream errBuf;
      errBuf << "Error parsing file: " << message << flush;
      XMLString::release( &message );
   }
   
}
