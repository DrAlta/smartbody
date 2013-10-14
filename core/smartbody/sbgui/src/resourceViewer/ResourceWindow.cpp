
#include "vhcl.h"

#include "ResourceWindow.h"
#include <sbm/lin_win.h>

#include <iostream>
#include <vector>
#include <math.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <boost/version.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <controllers/me_controller_tree_root.hpp>
#include <sb/SBScene.h>
#include <sb/SBCharacter.h>
#include <sb/SBSkeleton.h>
#include <sb/SBMotion.h>
#include <sb/SBServiceManager.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/nvbg.h>

#include <sbm/action_unit.hpp>
#include <sbm/sr_path_list.h>
#include <sb/sbm_character.hpp>
#include "FLTKListener.h"


// enum {
// 	ITEM_PHYSICS = 0,
// 	ITEM_SEQ_PATH, 
// 	ITEM_ME_PATH, 
// 	ITEM_AUDIO_PATH, 
// 	ITEM_MESH_PATH, 
// 	ITEM_SEQ_FILES,
// 	ITEM_SKELETON, 
// 	ITEM_JOINT_MAP,
// 	ITEM_MOTION, 			   
// 	ITEM_FACE_DEFINITION,
// 	ITEM_EVENT_HANDLERS,
// 	ITEM_PAWN, 
// 	ITEM_CHARACTER,   
// 	ITEM_NETURAL_MOTION,
// 	ITEM_AU_MAP,
// 	ITEM_VISEME_MAP,
// 	ITEM_SIZE };


ResourceWindowListener::ResourceWindowListener(ResourceWindow* window)
{
	_window = window;
}

void ResourceWindowListener::OnCharacterCreate( const std::string & name, const std::string & objectClass )
{
	_window->updateGUI();
}

void ResourceWindowListener::OnCharacterDelete( const std::string & name )
{
	_window->updateGUI();
}

void ResourceWindowListener::OnCharacterUpdate( const std::string & name )
{
	_window->updateGUI();
}
      
void ResourceWindowListener::OnPawnCreate( const std::string & name )
{
	_window->updateGUI();
}

void ResourceWindowListener::OnPawnDelete( const std::string & name )
{
	_window->updateGUI();
}

void ResourceWindowListener::OnReset()
{
	_window->updateGUI();
}

void ResourceWindowListener::OnSimulationStart()
{
}

void ResourceWindowListener::OnSimulationEnd()
{
}

void ResourceWindowListener::OnSimulationUpdate()
{
}


ResourceWindow::ResourceWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(w, h, name), GenericViewer(x, y, w, h)
{
	int rightPanelWidth = 450;
	itemInfoWidget = NULL;
	lastClickedItemPath = " ";

	ItemNameList.push_back("SCENE");
	ItemNameList.push_back("SERVICES");
	ItemNameList.push_back("SEQ_PATH");
	ItemNameList.push_back("ME_PATH");
	ItemNameList.push_back("AUDIO_PATH");
	ItemNameList.push_back("MESH_PATH");
	ItemNameList.push_back("SEQ_FILES");
	ItemNameList.push_back("SKELETON");
	ItemNameList.push_back("BONE MAP");
	ItemNameList.push_back("MOTION");
	ItemNameList.push_back("FACE DEFINITION");
	ItemNameList.push_back("EVENT HANDLERS");
	ItemNameList.push_back("PAWN");
	ItemNameList.push_back("CHARACTER");
	ItemNameList.push_back("CONTROLLER");
	ItemNameList.push_back("PHYSICS");
	ItemNameList.push_back("NEUTRAL MOTION");
	ItemNameList.push_back("AU MAP");
	ItemNameList.push_back("VISEME MAP");
	ItemNameList.push_back("DEFAULT");


	for (int i=0;i<ITEM_SIZE;i++)
		treeItemList[i] = NULL;

	// create Tree Info Object
	this->begin();
	resourceTree = new Fl_TreeHorizontal(10,10,w-rightPanelWidth,h-30);//new Fl_Tree(10,10,w - 300, h - 30);			
	resourceTree->showroot(0);
	pathTree = resourceTree->add("Paths");	
	pathTree->user_data((void*)ITEM_DEFAULT);
	treeItemList[ITEM_SEQ_PATH] = resourceTree->add(pathTree,"Script Paths");
	treeItemList[ITEM_ME_PATH] = resourceTree->add(pathTree,"Motion Paths");
	treeItemList[ITEM_AUDIO_PATH] = resourceTree->add(pathTree,"Audio Paths");
	treeItemList[ITEM_MESH_PATH] = resourceTree->add(pathTree,"Mesh Paths");

	treeItemList[ITEM_SEQ_FILES] = resourceTree->add("Scripts");

	treeItemList[ITEM_SKELETON] = resourceTree->add("Skeletons");
	treeItemList[ITEM_JOINT_MAP] = resourceTree->add("Character Maps");
	treeItemList[ITEM_DIPHONES] = resourceTree->add("Lip Syncing");
	treeItemList[ITEM_MOTION] =  resourceTree->add("Motions");	

	treeItemList[ITEM_FACE_DEFINITION] = resourceTree->add("Face Definitions");
	treeItemList[ITEM_EVENT_HANDLERS] = resourceTree->add("Event Handlers");

	treeItemList[ITEM_PAWN] = resourceTree->add("Pawns");
	treeItemList[ITEM_CHARACTER] = resourceTree->add("Characters");

	treeItemList[ITEM_SCENE] = resourceTree->add("Scene");
	treeItemList[ITEM_SERVICES] = resourceTree->add("Services");
	//treeItemList[ITEM_PHYSICS] = resourceTree->add("Physics");

	// set user_data to be the item enum ID
	for (int i = 0; i <= ITEM_CHARACTER; i++)
		treeItemList[i]->user_data((void*)i);

	resourceTree->callback(treeCallBack,this);

	refreshButton = new Fl_Button( w - rightPanelWidth + 20 , 10 , 100, 20, "Refresh");
	refreshButton->callback(refreshUI, this);

	resourceInfoGroup = new Fl_Group( w - rightPanelWidth + 20, 40, rightPanelWidth - 40 , h - 60);
	resourceInfoGroup->box(FL_UP_BOX);
	this->end();	
	this->resizable(resourceTree);
	updateGUI();	
	//resourceTree->close(resourceTree->root());	
	for (int i = 0; i < ITEM_SIZE; i++)
	{
		if (treeItemList[i])
			treeItemList[i]->close();
	}

	_listener = new ResourceWindowListener(this);
	
}

ResourceWindow::~ResourceWindow()
{
	SmartBody::SBScene::getScene()->removeSceneListener(_listener);
	delete _listener;
}

void ResourceWindow::label_viewer( std::string name )
{
	this->label(strdup(name.c_str()));
}

void ResourceWindow::show()
{
	SmartBody::SBScene::getScene()->addSceneListener(_listener);
	Fl_Double_Window::show();
}

void ResourceWindow::hide()
{
	SmartBody::SBScene::getScene()->removeSceneListener(_listener);
	Fl_Double_Window::hide();
}

void ResourceWindow::show_viewer()
{
	this->show();
}

void ResourceWindow::hide_viewer()
{
	this->hide();
}

void ResourceWindow::update_viewer()
{

}

int ResourceWindow::handle( int event )
{
	int ret = Fl_Double_Window::handle(event);
	switch ( event ) {
		case FL_PUSH:  
		{// do 'copy/dnd' when someone clicks on box
			if (Fl::event_button() == 2)
			{
				LOG("press middle button");
				LOG("lastClickedItemPath = %s",lastClickedItemPath.c_str());
				Fl_Tree_Item* lastItem = resourceTree->find_item(lastClickedItemPath.c_str());	
				if (lastItem)
				{
					long itemType = (long)lastItem->user_data();
					bool sendDND = false;
					std::string dndMsg = "";
					if (itemType == ITEM_SKELETON)
					{
						std::string skName = lastItem->label();
						dndMsg = "SKELETON:";
						dndMsg += skName;
						sendDND = true;
					}
					else if (itemType == ITEM_PAWN)
					{
						dndMsg = "PAWN:dummy";
						sendDND = true;
					}
					
					//Fl::copy("message",7,0);
					if (sendDND)
					{
						Fl::copy(dndMsg.c_str(),dndMsg.length(),0);
						Fl::dnd();
					}					
					ret = 1;
				}			
			}			
			break;
		}
		case FL_KEYDOWN:  
		{
		  switch (Fl::event_key())
		  {
			case FL_Delete:
				{
					// check pawns and characters for selection
					Fl_Tree_Item* tree = treeItemList[ITEM_PAWN];
					int numChildren = tree->children();
					for (int c = 0; c < numChildren; c++)
					{
						Fl_Tree_Item* child = tree->child(c);
						if (child->is_selected())
						{
							const char* name = child->label();
							SmartBody::SBScene::getScene()->removePawn(name);
							updateGUI();
							return 1;
						}
					}

					tree = treeItemList[ITEM_CHARACTER];
					numChildren = tree->children();
					for (int c = 0; c < numChildren; c++)
					{
						Fl_Tree_Item* child = tree->child(c);
						if (child->is_selected())
						{
							const char* name = child->label();
							SmartBody::SBScene::getScene()->removeCharacter(name);
							updateGUI();
							return 1;
						}
					}
				
				}
				break;
			default:
				break;
		  }
		}
	}
	return ret;
}

void ResourceWindow::update()
{
		
	if (0) //mcu.resourceDataChanged) // don't update the tree if nothing happens
	{		
		updateGUI();
		if (itemInfoWidget)
		{
			itemInfoWidget->updateWidget();
		}
		this->redraw();				
	}	
}

void ResourceWindow::draw()
{	
	Fl_Double_Window::draw();
}

void ResourceWindow::resize( int x, int y, int w, int h )
{
	Fl_Double_Window::resize(x, y, w, h);   
	this->redraw();
}

void ResourceWindow::updateGUI()
{

	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	resourceTree->sortorder(FL_TREE_SORT_ASCENDING);	
	// update path tree	
	updatePath(treeItemList[ITEM_SEQ_PATH], SmartBody::SBScene::getScene()->getAssetPaths("script"));	
	updatePath(treeItemList[ITEM_ME_PATH], SmartBody::SBScene::getScene()->getAssetPaths("motion"));	
	updatePath(treeItemList[ITEM_AUDIO_PATH], SmartBody::SBScene::getScene()->getAssetPaths("audio"));	
	updatePath(treeItemList[ITEM_MESH_PATH], SmartBody::SBScene::getScene()->getAssetPaths("mesh"));	
	

	// update sequence file list

	const std::vector<std::string> scriptPaths = SmartBody::SBScene::getScene()->getAssetPaths("script");
	resourceTree->clear_children(treeItemList[ITEM_SEQ_FILES]);
	for (size_t p = 0; p < scriptPaths.size(); p++)
	{
		updateSeqFiles(treeItemList[ITEM_SEQ_FILES], scriptPaths[p]);
	}	

	// update skeleton
	resourceTree->clear_children(treeItemList[ITEM_SKELETON]);
	std::vector<std::string> skeletons = scene->getSkeletonNames();
	for (size_t c = 0; c < skeletons.size(); c++)
	{
		SmartBody::SBSkeleton * skel = scene->getSkeleton(skeletons[c]);
		updateSkeleton(treeItemList[ITEM_SKELETON], skel);
	}

	// update joint maps
	resourceTree->clear_children(treeItemList[ITEM_JOINT_MAP]);	
	SmartBody::SBJointMapManager* jointMapManager = scene->getJointMapManager();
	std::vector<std::string> jointMapNames = jointMapManager->getJointMapNames();
	for (std::vector<std::string>::iterator iter = jointMapNames.begin();
		 iter != jointMapNames.end(); 
		 iter++)
	{
		Fl_Tree_Item* boneMapItem = resourceTree->add(treeItemList[ITEM_JOINT_MAP], (*iter).c_str());
		updateJointMap(boneMapItem, jointMapManager->getJointMap((*iter)));
	}

	// update motion map
	resourceTree->clear_children(treeItemList[ITEM_MOTION]);
	std::vector<std::string> motionNames = scene->getMotionNames();
	for (size_t i = 0; i < motionNames.size(); i++)
	{
		//resourceTree->add(treeItemList[ITEM_MOTION],mi->first.c_str());
		SmartBody::SBMotion * motion = scene->getMotion(motionNames[i]);
		updateMotion(treeItemList[ITEM_MOTION], motion);
	}

	// update face definition map
	resourceTree->clear_children(treeItemList[ITEM_FACE_DEFINITION]);
	std::vector<std::string> faceNames = scene->getFaceDefinitionNames();
	for (size_t i = 0; i < faceNames.size(); i++)
	{
		//resourceTree->add(treeItemList[ITEM_MOTION],mi->first.c_str());
		SmartBody::SBFaceDefinition * face = scene->getFaceDefinition(faceNames[i]);
		Fl_Tree_Item* faceTree = resourceTree->add(treeItemList[ITEM_FACE_DEFINITION], face->getName().c_str());
		faceTree->user_data((void*)ITEM_FACE_DEFINITION);
		updateFaceMotion(faceTree, face);
	}

	// update event handler list
	SmartBody::SBEventManager* eventManager = SmartBody::SBScene::getScene()->getEventManager();
	SmartBody::SBEventHandlerMap& eventMap = eventManager->getEventHandlers();
	SmartBody::SBEventHandlerMap::iterator ei;
	resourceTree->clear_children(treeItemList[ITEM_EVENT_HANDLERS]);
	for ( ei  = eventMap.begin();
		  ei != eventMap.end();
		  ei++)
	{
		std::string handlerKey = ei->first;
		Fl_Tree_Item* handlerItem = resourceTree->add(treeItemList[ITEM_EVENT_HANDLERS],handlerKey.c_str());
		handlerItem->user_data((void*)ITEM_EVENT_HANDLERS);
		updateEventHandler(handlerItem,ei->second);
	}
	// Below are instance objects :

	// update pawn objects
	resourceTree->clear_children(treeItemList[ITEM_PAWN]);
	const std::vector<std::string>& pawnNames = scene->getPawnNames();
	for (size_t i = 0; i < pawnNames.size(); i++)
	{
		SmartBody::SBPawn* pawn = scene->getPawn(pawnNames[i]);
		updatePawn(treeItemList[ITEM_PAWN], pawn);
	}

	// update characters
	resourceTree->clear_children(treeItemList[ITEM_CHARACTER]);
	const std::vector<std::string>& charNames = scene->getCharacterNames();
	for (size_t i = 0; i < charNames.size(); i++)
	{
		SmartBody::SBCharacter* character = scene->getCharacter(charNames[i]);
		resourceTree->sortorder(FL_TREE_SORT_ASCENDING);
		updateCharacter(treeItemList[ITEM_CHARACTER], character);
	}

	
// 	for (SBPhysicsObjMap::iterator iter = phySim->getPhysicsObjMap().begin();
// 		 iter != phySim->getPhysicsObjMap().end();
// 		 iter++)
// 	{
// 		SBPhysicsObj* obj = (*iter).second;
// 		if (dynamic_cast<SbmJointObj*>(obj) == NULL)
// 		{
// 
// 		}
// 	}

	// update services
	SmartBody::SBServiceManager* serviceManager = scene->getServiceManager();
	std::map<std::string, SmartBody::SBService*>& serviceMap = serviceManager->getServices();

	resourceTree->clear_children(treeItemList[ITEM_SERVICES]);
	for (std::map<std::string, SmartBody::SBService*>::iterator iter = serviceMap.begin();
		iter != serviceMap.end();
		iter++)
	{
		SmartBody::SBService* service = (*iter).second;
		resourceTree->sortorder(FL_TREE_SORT_ASCENDING);	
		SmartBody::SBPhysicsManager* phyManager = dynamic_cast<SmartBody::SBPhysicsManager*>(service);
		if (phyManager)
			updatePhysicsManager(treeItemList[ITEM_SERVICES],phyManager);
		else
			updateService(treeItemList[ITEM_SERVICES], service);
	}			


	Fl_Tree_Item* lastItem = resourceTree->find_item(lastClickedItemPath.c_str());	
	if (lastItem)
	{		
		long itemType = (long)lastItem->user_data();
		updateTreeItemInfo(lastItem,itemType);
	}
	else
	{
		clearInfoWidget(itemInfoWidget);
		itemInfoWidget = NULL;
	}
}


void ResourceWindow::updatePhysicsManager( Fl_Tree_Item* tree, SmartBody::SBPhysicsManager* phyService )
{
	SmartBody::SBPhysicsSim* phySim = phyService->getPhysicsEngine();
	Fl_Tree_Item* item = resourceTree->add(tree, phyService->getName().c_str());
	item->user_data((void*)ITEM_PHYSICS);
	resourceTree->sortorder(FL_TREE_SORT_NONE);

	for (SmartBody::SBPhysicsCharacterMap::iterator iter = phySim->getCharacterMap().begin();
		iter != phySim->getCharacterMap().end();
		iter++)
	{
		SmartBody::SBPhysicsCharacter* phyChar = (*iter).second;
		resourceTree->sortorder(FL_TREE_SORT_ASCENDING);
		updatePhysicsCharacter(item,phyChar);
	}

	for (SmartBody::SBPhysicsPawnMap::iterator iter = phySim->getPawnObjMap().begin();
		 iter != phySim->getPawnObjMap().end();
		 iter++)
	{
		SmartBody::SBPhysicsObj* phyObj = (*iter).second;
		Fl_Tree_Item* phyObjItem = resourceTree->add(item, phyObj->getName().c_str());
		phyObjItem->user_data((void*)ITEM_PHYSICS);
	}
}


void ResourceWindow::updateFaceMotion( Fl_Tree_Item* tree, SmartBody::SBFaceDefinition* faceDefinition )
{
	std::string neutralMotionName = "NA";
	if (faceDefinition->getFaceNeutral())
	{
		neutralMotionName = faceDefinition->getFaceNeutral()->getName();		
	}
	Fl_Tree_Item* neutralMotionTree = resourceTree->add(tree,"Neutral Expression");
	neutralMotionTree->close();
	neutralMotionTree->user_data((void*)ITEM_NETURAL_MOTION);

	Fl_Tree_Item* item = resourceTree->add(neutralMotionTree,neutralMotionName.c_str());
	item->user_data((void*)ITEM_NETURAL_MOTION);

	// update action unit tree
	Fl_Tree_Item* auTree = resourceTree->add(tree,"Action Units (AUs)");
	auTree->close();
	auTree->user_data((void*)ITEM_AU_MAP);
	int numAUs = faceDefinition->getNumAUs();
	for (int a = 0; a < numAUs; a++)
	{
		int auNUm = faceDefinition->getAUNum(a);
		ActionUnit* au = faceDefinition->getAU(auNUm);
		std::string auName = "Au " + boost::lexical_cast<std::string>(faceDefinition->getAUNum(a));
		
		Fl_Tree_Item* auItem = resourceTree->add(auTree,auName.c_str());
		auItem->close();
		auItem->user_data((void*)ITEM_AU_MAP);
		std::string auType = "bilateral:";
		if (au->is_bilateral())
		{	
			Fl_Tree_Item* item = NULL;
			if (au->left)
				item = resourceTree->add(auItem,(auType+ au->left->getName()).c_str());
			else
				item = resourceTree->add(auItem, auType.c_str());

			item->user_data((void*)ITEM_AU_MAP);
		}
		else 
		{
			if (au->is_left())
			{
				auType = "left:";
				Fl_Tree_Item* item = NULL;
				if (au->left)
				{
					item = resourceTree->add(auItem,(auType+au->left->getName()).c_str());
				}
				else
				{
					item = resourceTree->add(auItem, auType.c_str());
				}
				item->user_data((void*)ITEM_AU_MAP);
			}
			if (au->is_right())
			{
				auType = "right:";
				if (au->right)
				{
					item = resourceTree->add(auItem,(auType+au->right->getName()).c_str());
				}
				else
				{
					item = resourceTree->add(auItem, auType.c_str());
				}
				item->user_data((void*)ITEM_AU_MAP);
			}
		}		
	}

	// update viseme tree
	Fl_Tree_Item* visemeTree = resourceTree->add(tree,"Visemes");	
	visemeTree->close();
	visemeTree->user_data((void*)ITEM_VISEME_MAP);
	int numVisemes = faceDefinition->getNumVisemes();
	for (int v = 0; v < numVisemes; v++)
	{
		std::string visemeName = faceDefinition->getVisemeName(v);
		std::string motionName = "";
		if (faceDefinition->getVisemeMotion(visemeName))
			motionName = faceDefinition->getVisemeMotion(visemeName)->getName();
		Fl_Tree_Item* item = resourceTree->add(visemeTree,(visemeName+"-->"+motionName).c_str());
		item->user_data((void*)ITEM_VISEME_MAP);		
	}
}

void ResourceWindow::updatePath( Fl_Tree_Item* tree, const std::vector<std::string>& pathList )
{
	resourceTree->clear_children(tree);	
	for (size_t p = 0; p < pathList.size(); p++)
	{
		Fl_Tree_Item* item = resourceTree->add(tree, pathList[p].c_str());
		item->user_data(tree->user_data());
	}	
}

void ResourceWindow::updateSeqFiles( Fl_Tree_Item* tree, std::string pname )
{	
	using namespace boost::filesystem;
	path seqPath(pname);		
	if( is_directory( seqPath ) ) {

		directory_iterator end;
		for( directory_iterator i( seqPath ); i!=end; ++i ) {
			const path& cur = *i;
			if( !is_directory( cur ) ) {
				std::string ext = extension( cur );	
				if (_stricmp(ext.c_str(),".seq") == 0 ||
					_stricmp(ext.c_str(),".SEQ") == 0 )
				{
					std::string fileName = basename(cur);
					if (tree->find_child(fileName.c_str()) == -1) // if the seq name does not exist in the tree list
					{
						Fl_Tree_Item* item = resourceTree->add(tree,fileName.c_str());
						item->user_data((void*)ITEM_SEQ_FILES);
					}
				}
			} 
		}
	}
}


void ResourceWindow::updateJointMap( Fl_Tree_Item* tree, SmartBody::SBJointMap* jointMap )
{
	for (int i=0;i<jointMap->getNumMappings();i++)
	{
		std::string key = jointMap->getSource(i);
		std::string target = jointMap->getTarget(i);

		
		Fl_Tree_Item* item = resourceTree->add(tree,(key+"-->"+target).c_str());	
		item->user_data((void*)ITEM_JOINT_MAP);
		//resourceTree->add(item,target.c_str());
	}
}

void ResourceWindow::updateEventHandler( Fl_Tree_Item* tree, SmartBody::SBEventHandler* handler )
{
	//Fl_Tree_Item* item = resourceTree->add(tree,(handler->getType() + ":" + "\""+ handler->getAction() + "\"").c_str());
	//item->user_data((void*)ITEM_EVENT_HANDLERS);
}

void ResourceWindow::updateSkeleton( Fl_Tree_Item* tree, SmartBody::SBSkeleton* skel )
{
#if (BOOST_VERSION > 104400)
	std::string ext = boost::filesystem::extension( skel->skfilename() );
#else
	std::string ext = boost::filesystem2::extension( skel->skfilename() );
#endif
	std::string filebase = boost::filesystem::basename(skel->skfilename());
	Fl_Tree_Item* item = resourceTree->add(tree,(filebase+ext).c_str());
	item->user_data((void*)ITEM_SKELETON);
}

void ResourceWindow::updateMotion( Fl_Tree_Item* tree, SmartBody::SBMotion* motion )
{
	Fl_Tree_Item* item = resourceTree->add(tree, motion->getName().c_str());
	item->user_data((void*)ITEM_MOTION);
}

void ResourceWindow::updatePawn( Fl_Tree_Item* tree, SmartBody::SBPawn* pawn )
{
	if (dynamic_cast<SbmCharacter*>(pawn) != NULL)
		return; // this is actually a character

	Fl_Tree_Item* item = resourceTree->add(tree,pawn->getName().c_str());
	item->user_data((void*)ITEM_PAWN);
}


void ResourceWindow::updatePhysicsCharacter( Fl_Tree_Item* tree, SmartBody::SBPhysicsCharacter* phyChar )
{
	Fl_Tree_Item* item = resourceTree->add(tree,phyChar->getPhysicsCharacterName().c_str());
	item->user_data((void*)ITEM_PHYSICS);
	resourceTree->sortorder(FL_TREE_SORT_NONE);	
	std::vector<SmartBody::SBPhysicsJoint*> jointList = phyChar->getPhyJointList();
	for (unsigned int i=0;i<jointList.size();i++)
	{
		SmartBody::SBPhysicsJoint* phyJoint = jointList[i];
		Fl_Tree_Item* jointItem = resourceTree->add(item,phyJoint->getSBJoint()->getName().c_str());
		jointItem->user_data((void*)ITEM_PHYSICS);
		//Fl_Tree_Item* rigidBodyItem = resourceTree->add(jointItem,"body");
		//rigidBodyItem->user_data((void*)ITEM_PHYSICS);
	}

}

void ResourceWindow::updateCharacter( Fl_Tree_Item* tree, SmartBody::SBCharacter* character )
{
	SmartBody::SBCharacter* sbcharacter = dynamic_cast<SmartBody::SBCharacter*>(character);
	Fl_Tree_Item* item = resourceTree->add(tree,character->getName().c_str());
	item->user_data((void*)ITEM_CHARACTER);
	resourceTree->sortorder(FL_TREE_SORT_NONE);		
	SmartBody::SBSkeleton* sbSk = sbcharacter->getSkeleton();
	if (sbSk)
	{
		Fl_Tree_Item* charSkItem = resourceTree->add(item, sbSk->getName().c_str());
		charSkItem->user_data((void*)ITEM_SKELETON);
	}
	Fl_Tree_Item* controllerFolder = resourceTree->add(item,"controllers");	
	controllerFolder->user_data((void*)-1);
	controllerFolder->close();
	// add controllers
	MeControllerTreeRoot* ctTree = character->ct_tree_p ;
	if( ctTree )
	{
		int n = ctTree->count_controllers();
		for (int c = 0; c < n; c++)
		{
			//LOG( "%s", ctTree->controller(c)->name() );
			Fl_Tree_Item* ctrlItem = resourceTree->add(controllerFolder,ctTree->controller(c)->getName().c_str());
			ctrlItem->user_data((void*)ITEM_CONTROLLER);
		}
	}
	/*
	// add gesture map
	Fl_Tree_Item* gestureFolder = resourceTree->add(item,"gestures");	
	gestureFolder->user_data((void*)-1);
	gestureFolder->close();
	// add individual gesture mappings
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();

	SBGestureMap* gestureMap = scene->getGestureMapManager()->getGestureMap(sbcharacter->getName());
	if (gestureMap)
	{
		std::string lexeme;
		std::string type;
		std::string hand;
		std::string style;
		std::string posture;

		gestureMap->getGestureByInfo(lexeme, type, hand, style, posture);
		Fl_Tree_Item* gestureItem = resourceTree->add(gestureFolder, lexeme.c_str());
		gestureItem->user_data((void*)ITEM_GESTUREMAP);
	}
	*/

	// add NVBG
	SmartBody::Nvbg* nvbg = character->getNvbg();
	if (nvbg)
	{
		Fl_Tree_Item* ctrlItem = resourceTree->add(item, "NVBG");
		ctrlItem->user_data((void*)ITEM_NVBG);
	}
}


void ResourceWindow::updateService( Fl_Tree_Item* tree, SmartBody::SBService* service )
{
	Fl_Tree_Item* item = resourceTree->add(tree, service->getName().c_str());
	item->user_data((void*)ITEM_SERVICES);
	resourceTree->sortorder(FL_TREE_SORT_NONE);	
}

void ResourceWindow::refreshUI( Fl_Widget* widget, void* data )
{
	ResourceWindow* window = (ResourceWindow*)data;
	window->updateGUI();	
}

void ResourceWindow::treeCallBack( Fl_Widget* widget, void* data )
{
	//LOG("Tree call back....");

	Fl_Tree      *tree = (Fl_Tree*)widget;
	Fl_Tree_Item *item = (Fl_Tree_Item*)tree->callback_item();	// get selected item
	ResourceWindow* window = (ResourceWindow*)data;
	if (tree->callback_reason() == FL_TREE_REASON_SELECTED)
	{
		long itemType = (long)item->user_data();//window->findTreeItemType(item);
		if (itemType >= 0)
		{
			//LOG("Item Type =%s",ItemNameList[itemType].c_str());				
			window->updateTreeItemInfo(item,itemType);
			if (itemType == ITEM_PAWN || itemType == ITEM_CHARACTER)
			{
				if (item == window->treeItemList[ITEM_PAWN] ||
					item == window->treeItemList[ITEM_CHARACTER]) // clicked on the extent
					return;
				std::vector<SmartBody::SBSceneListener*>& listeners = SmartBody::SBScene::getScene()->getSceneListeners();
				for (size_t i = 0; i < listeners.size(); i++)
				{
					FLTKListener* fltkListener = dynamic_cast<FLTKListener*>(listeners[i]);
					if (fltkListener)
						fltkListener->OnObjectSelected( item->label() );
				}
			}
		}
	}	
	if (tree->callback_reason() == FL_TREE_REASON_DESELECTED)
	{
		//LOG("Item Deselect...\n");
	}
}

int ResourceWindow::findTreeItemType( Fl_Tree_Item* treeItem )
{
	Fl_Tree_Item* curItem = treeItem;
	while (curItem != resourceTree->root())
	{
		for (int i=0;i<ITEM_SIZE;i++)
		{
			if (curItem == treeItemList[i])
				return i;
		}
		curItem = curItem->parent();
	}
	return -1;
}

void ResourceWindow::updateTreeItemInfo( Fl_Tree_Item* treeItem, long itemType )
{
	if (!treeItem || itemType < 0) return;		

	char pathName[128];	
	resourceTree->item_pathname(pathName,128,treeItem);
	if (strcmp(pathName, lastClickedItemPath.c_str()) == 0)
		return;
	lastClickedItemPath = pathName;	
	TreeItemInfoWidget* lastWidget = itemInfoWidget;
	itemInfoWidget = createInfoWidget(resourceInfoGroup->x(),resourceInfoGroup->y(),resourceInfoGroup->w(),resourceInfoGroup->h(),ItemNameList[itemType].c_str(),treeItem,itemType);
	resourceInfoGroup->add(itemInfoWidget);
	clearInfoWidget(lastWidget);	
	resourceInfoGroup->show();
	itemInfoWidget->show();
	resourceInfoGroup->redraw();
}

void ResourceWindow::clearInfoWidget(TreeItemInfoWidget* lastWidget)
{
	if (lastWidget)
	{
		resourceInfoGroup->remove(lastWidget);
		widgetsToDelete.push_back(lastWidget); // need to delete these - causes memory leak
	}
}

TreeItemInfoWidget* ResourceWindow::createInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* treeItem, int itemType )
{
	SmartBody::SBScene* scene = SmartBody::SBScene::getScene();
	TreeItemInfoWidget* widget = NULL;
	if (itemType == ITEM_SKELETON)
	{
		widget = new SkeletonItemInfoWidget(x,y,w,h,name,treeItem,itemType, this);
	}
	else if (itemType == ITEM_SEQ_PATH || itemType == ITEM_ME_PATH || itemType == ITEM_AUDIO_PATH || itemType == ITEM_MESH_PATH)
	{
		widget = new PathItemInfoWidget(x,y,w,h,name,treeItem,itemType, this);
	}
	else if (itemType == ITEM_SEQ_FILES)
	{
		widget = new SeqItemInfoWidget(x,y,w,h,name,treeItem,itemType, this);
	}
	else if (itemType == ITEM_MOTION)
	{
		widget = new MotionItemInfoWidget(x,y,w,h,name,treeItem,itemType, this);
	}
	else if (itemType == ITEM_PAWN)
	{
		SmartBody::SBPawn* curPawn = scene->getPawn(treeItem->label());
		/*if (curPawn)
			widget = new PawnItemInfoWidget(x,y,w,h,name,treeItem,itemType,this);
		else 
			widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
		*/
		widget = new AttributeItemWidget(curPawn, x, y, w, h, strdup(name), treeItem, itemType, this);
	}
	else if (itemType == ITEM_CHARACTER)
	{
		SmartBody::SBCharacter* curChar = scene->getCharacter(treeItem->label());
		if (curChar)
			widget = new AttributeItemWidget(curChar,x,y,w,h,name,treeItem,itemType,this);
		else
			widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_PHYSICS)
	{
		SmartBody::SBPhysicsSim* phySim = SmartBody::SBPhysicsSim::getPhysicsEngine();
		std::string itemName = treeItem->label();
		std::string parentName = treeItem->parent()->label();
		SmartBody::SBPhysicsCharacter* phyChar = phySim->getPhysicsCharacter(itemName);
		SmartBody::SBPhysicsCharacter* phyParent = phySim->getPhysicsCharacter(parentName);
		SmartBody::SBPhysicsObj*    phyBody = phySim->getPhysicsPawn(itemName);
		SmartBody::SBObject* phyObj = phySim;		
		SmartBody::SBObject* phyObj2 = NULL;

		static std::string name1 = "PHYSICS JOINT";
		static std::string name2 = "RIGID BODY";	
		std::vector<std::string> phyObjNameList;
		std::vector<SmartBody::SBObject*> phyObjList;
		if (phyChar)
		{
			phyObj = phyChar;
		}
		else if (phyParent)
		{
			SmartBody::SBPhysicsJoint* phyJoint = phyParent->getPhyJoint(itemName);
			phyObj = phyJoint;
			phyObj2 = phyJoint->getChildObj();
			phyObjNameList.push_back(name1);
			phyObjNameList.push_back(name2);
			phyObjList.push_back(phyObj);
			phyObjList.push_back(phyObj2);
		}
		else if (phyBody)
		{
			phyObj = phyBody;			
		}

		if (phyObjList.size() > 0)
		{
			//widget = new DoubleAttributeItemWidget(phyObj,phyObj2,x,y,w,h,h/2,name1.c_str(),name2.c_str(),treeItem,itemType,this);
			widget = new MultiAttributeItemWidget(phyObjList,x,y,w,h,h/2,name,phyObjNameList,treeItem,itemType,this);
		}
		else if (phyObj)
			widget = new AttributeItemWidget(phyObj,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_SCENE)
	{
		widget = new AttributeItemWidget(scene,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_SERVICES)
	{
		SmartBody::SBService* service = scene->getServiceManager()->getService(treeItem->label());
		widget = new AttributeItemWidget(service,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_EVENT_HANDLERS)
	{
		widget = new EventItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_CONTROLLER)
	{
		SmartBody::SBCharacter* curChar = scene->getCharacter(treeItem->parent()->parent()->label()); // a controller's parent is its character name
		MeController* ctrl = NULL;
		if (curChar)
		{			
			MeControllerTreeRoot* ctTree = curChar->ct_tree_p ;
			for (unsigned int c = 0; c < ctTree->count_controllers(); c++)
			{
				if (ctTree->controller(c)->getName() == treeItem->label())
					ctrl = ctTree->controller(c);
			}
		}		
		if (ctrl)
			widget = new AttributeItemWidget(ctrl,x,y,w,h,name,treeItem,itemType,this);
		else
			widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_GESTUREMAP)
	{
		SmartBody::SBCharacter* curChar = scene->getCharacter(treeItem->parent()->parent()->label()); // a controller's parent is its character name
		widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_NVBG)
	{
		SmartBody::SBCharacter* curChar = scene->getCharacter(treeItem->parent()->label());
		widget = new AttributeItemWidget(curChar->getNvbg(),x,y,w,h,name,treeItem,itemType,this);
	}
	else
	{
		widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	return widget;
}

void ResourceWindow::selectPawn(const std::string& name)
{
	Fl_Tree_Item* tree = treeItemList[ITEM_PAWN];

	for (int c = 0; c < tree->children(); c++)
	{
		Fl_Tree_Item* child = tree->child(c);
		if (strcmp(child->label(), name.c_str()) == 0)
		{
			if (child->is_selected())
				return;
			this->updateTreeItemInfo(child, ITEM_PAWN);
			resourceTree->deselect_all();
			child->select();
			resourceTree->redraw();
			return;
		}
	}

	tree = treeItemList[ITEM_CHARACTER];
	for (int c = 0; c < tree->children(); c++)
	{
		Fl_Tree_Item* child = tree->child(c);
		if (strcmp(child->label(), name.c_str()) == 0)
		{
			if (child->is_selected())
				return;
			this->updateTreeItemInfo(child, ITEM_CHARACTER);
			resourceTree->deselect_all();
			child->select();
			resourceTree->redraw();
			return;
		}
	}

}


void ResourceWindow::notify( SmartBody::SBSubject* subject )
{
	this->refreshUI(this, this);
}
/************************************************************************/
/* Resource Viewer Factory                                              */
/************************************************************************/

GenericViewer* ResourceViewerFactory::create(int x, int y, int w, int h)
{
	ResourceWindow* resourceWindow = new ResourceWindow(x, y, w, h, (char*)"Resource Window");
	return resourceWindow;
}

void ResourceViewerFactory::destroy(GenericViewer* viewer)
{
	delete viewer;
}

ResourceViewerFactory::ResourceViewerFactory()
{

}

