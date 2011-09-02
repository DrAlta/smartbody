
#include "vhcl.h"

#include "ResourceWindow.h"

#include <iostream>
#include <vector>
#include <math.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <sbm/mcontrol_util.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>


// enum {
// 	ITEM_PHYSICS = 0,
// 	ITEM_STEERING,
// 	ITEM_SEQ_PATH, 
// 	ITEM_ME_PATH, 
// 	ITEM_AUDIO_PATH, 
// 	ITEM_MESH_PATH, 
// 	ITEM_SEQ_FILES,
// 	ITEM_SKELETON, 
// 	ITEM_BONE_MAP,
// 	ITEM_MOTION, 			   
// 	ITEM_FACE_DEFINITION,
// 	ITEM_EVENT_HANDLERS,
// 	ITEM_PAWN, 
// 	ITEM_CHARACTER,   
// 	ITEM_NETURAL_MOTION,
// 	ITEM_AU_MAP,
// 	ITEM_VISEME_MAP,
// 	ITEM_SIZE };

std::string ResourceWindow::ItemNameList[ITEM_SIZE] = { "SCENE", "PHYSICS", "STEERING", 
														"SEQ_PATH", "ME_PATH", "AUDIO_PATH", "MESH_PATH", "SEQ_FILES", 
														"SKELETON", "BONE MAP", "MOTION",  
														"FACE DEFINITION", "EVENT HANDLERS",
														"PAWN", "CHARACTER", "CONTROLLER",
														"NEUTRAL MOTION", "AU MAP", "VISEME MAP", "DEFAULT"} ;

ResourceWindow::ResourceWindow(int x, int y, int w, int h, char* name) : Fl_Double_Window(w, h, name), GenericViewer(x, y, w, h)
{
	int rightPanelWidth = 450;
	itemInfoWidget = NULL;
	lastClickedItemPath = " ";

	for (int i=0;i<ITEM_SIZE;i++)
		treeItemList[i] = NULL;

	// create Tree Info Object
	this->begin();
	resourceTree = new Fl_TreeHorizontal(10,10,w-rightPanelWidth,h-30);//new Fl_Tree(10,10,w - 300, h - 30);			
	resourceTree->showroot(0);
	pathTree = resourceTree->add("Paths");	
	pathTree->user_data((void*)ITEM_DEFAULT);
	treeItemList[ITEM_SEQ_PATH] = resourceTree->add(pathTree,"Sequence Paths");
	treeItemList[ITEM_ME_PATH] = resourceTree->add(pathTree,"ME Paths");
	treeItemList[ITEM_AUDIO_PATH] = resourceTree->add(pathTree,"Audio Paths");
	treeItemList[ITEM_MESH_PATH] = resourceTree->add(pathTree,"Mesh Paths");

	treeItemList[ITEM_SEQ_FILES] = resourceTree->add("Seq Files");

	treeItemList[ITEM_SKELETON] = resourceTree->add("Skeletons");
	treeItemList[ITEM_BONE_MAP] = resourceTree->add("Bone Map");
	treeItemList[ITEM_MOTION] =  resourceTree->add("Motions");	

	treeItemList[ITEM_FACE_DEFINITION] = resourceTree->add("Face Definition");
	treeItemList[ITEM_EVENT_HANDLERS] = resourceTree->add("Event Handlers");

	treeItemList[ITEM_PAWN] = resourceTree->add("Pawns");
	treeItemList[ITEM_CHARACTER] = resourceTree->add("Characters");

	treeItemList[ITEM_SCENE] = resourceTree->add("Scene");
	treeItemList[ITEM_PHYSICS] = resourceTree->add("Physics");
	treeItemList[ITEM_STEERING] = resourceTree->add("Steering");


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
}

ResourceWindow::~ResourceWindow()
{
	
}

void ResourceWindow::label_viewer( std::string name )
{
	this->label(strdup(name.c_str()));
}

void ResourceWindow::show_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	this->show();
}

void ResourceWindow::hide_viewer()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.bml_processor.registerRequestCallback(NULL, NULL);
	this->hide();
}

void ResourceWindow::update_viewer()
{

}

int ResourceWindow::handle( int event )
{
	return Fl_Double_Window::handle(event);
}

void ResourceWindow::show()
{
	Fl_Double_Window::show();
}

void ResourceWindow::update()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	if (mcu.resourceDataChanged) // don't update the tree if nothing happens
	{		
		updateGUI();
		mcu.resourceDataChanged = false;
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	resourceTree->sortorder(FL_TREE_SORT_ASCENDING);	
	// update path tree	
	updatePath(treeItemList[ITEM_SEQ_PATH],mcu.seq_paths);	
	updatePath(treeItemList[ITEM_ME_PATH],mcu.me_paths);
	updatePath(treeItemList[ITEM_AUDIO_PATH],mcu.audio_paths);	
	updatePath(treeItemList[ITEM_MESH_PATH],mcu.mesh_paths);	
	

	// update sequence file list
	mcu.seq_paths.reset();
	std::string pathName = mcu.seq_paths.next_path();
	resourceTree->clear_children(treeItemList[ITEM_SEQ_FILES]);
	while (pathName != "")
	{
		updateSeqFiles(treeItemList[ITEM_SEQ_FILES],pathName);
		pathName = mcu.seq_paths.next_path();
	}	

	// update skeleton
	SkSkeletonMap::iterator ski;
	resourceTree->clear_children(treeItemList[ITEM_SKELETON]);	
	for ( ski  = mcu.skeleton_map.begin();
		  ski != mcu.skeleton_map.end();
		  ski++)
	{
		updateSkeleton(treeItemList[ITEM_SKELETON],ski->second);
	}

	// update bone map
	std::map<std::string, BoneMap*>::iterator bi;
	resourceTree->clear_children(treeItemList[ITEM_BONE_MAP]);	
	for ( bi  = mcu.boneMaps.begin();
		bi != mcu.boneMaps.end();
		bi++)
	{
		Fl_Tree_Item* boneMapItem = resourceTree->add(treeItemList[ITEM_BONE_MAP],bi->first.c_str());
		updateBoneMap(boneMapItem,bi->second);
	}

	// update motion map
	SkMotionMap::iterator mi;
	resourceTree->clear_children(treeItemList[ITEM_MOTION]);
	for ( mi  = mcu.motion_map.begin();
		  mi != mcu.motion_map.end();
		  mi++)
	{
		//resourceTree->add(treeItemList[ITEM_MOTION],mi->first.c_str());
		updateMotion(treeItemList[ITEM_MOTION],mi->second);
	}

	// update face definition map
	std::map<std::string, FaceDefinition*>::iterator fi;
	resourceTree->clear_children(treeItemList[ITEM_FACE_DEFINITION]);
	for ( fi  = mcu.face_map.begin();
		  fi != mcu.face_map.end();
		  fi++)
	{
		//resourceTree->add(treeItemList[ITEM_MOTION],mi->first.c_str());
		Fl_Tree_Item* faceTree = resourceTree->add(treeItemList[ITEM_FACE_DEFINITION],fi->first.c_str());
		faceTree->user_data((void*)ITEM_FACE_DEFINITION);
		updateFaceMotion(faceTree,fi->second);
	}

	// update event handler list
	EventManager* eventManager = EventManager::getEventManager();
	EventHandlerMap& eventMap = eventManager->getEventHandlers();
	EventHandlerMap::iterator ei;
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
	SbmPawn* pawn_p = NULL;
	resourceTree->clear_children(treeItemList[ITEM_PAWN]);
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		updatePawn(treeItemList[ITEM_PAWN], pawn);
	}

	// update characters
	resourceTree->clear_children(treeItemList[ITEM_CHARACTER]);
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		resourceTree->sortorder(FL_TREE_SORT_ASCENDING);	
		updateCharacter(treeItemList[ITEM_CHARACTER],character);
	}			
	Fl_Tree_Item* lastItem = resourceTree->find_item(lastClickedItemPath.c_str());	
	if (lastItem)
	{		
		long itemType = (long)lastItem->user_data();
		updateTreeItemInfo(lastItem,itemType);
	}
	else
	{
		clearInfoWidget();
	}
}

void ResourceWindow::updateFaceMotion( Fl_Tree_Item* tree, FaceDefinition* faceDefinition )
{
	std::string neutralMotionName = "NA";
	if (faceDefinition->getFaceNeutral())
	{
		neutralMotionName = faceDefinition->getFaceNeutral()->name();		
	}
	Fl_Tree_Item* neutralMotionTree = resourceTree->add(tree,"Neutral Motion");
	neutralMotionTree->user_data((void*)ITEM_NETURAL_MOTION);

	Fl_Tree_Item* item = resourceTree->add(neutralMotionTree,neutralMotionName.c_str());
	item->user_data((void*)ITEM_NETURAL_MOTION);

	// update action unit tree
	Fl_Tree_Item* auTree = resourceTree->add(tree,"AU Map");
	auTree->user_data((void*)ITEM_AU_MAP);
	int numAUs = faceDefinition->getNumAUs();
	for (int a = 0; a < numAUs; a++)
	{
		int auNUm = faceDefinition->getAUNum(a);
		ActionUnit* au = faceDefinition->getAU(auNUm);
		std::string auName = "Au " + boost::lexical_cast<std::string>(faceDefinition->getAUNum(a));
		
		Fl_Tree_Item* auItem = resourceTree->add(auTree,auName.c_str());
		auItem->user_data((void*)ITEM_AU_MAP);
		std::string auType = "bilateral:";
		if (au->is_bilateral())
		{	
			Fl_Tree_Item* item = NULL;
			if (au->left)
				item = resourceTree->add(auItem,(auType+ au->left->name()).c_str());
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
					item = resourceTree->add(auItem,(auType+au->left->name()).c_str());
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
					item = resourceTree->add(auItem,(auType+au->right->name()).c_str());
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
	Fl_Tree_Item* visemeTree = resourceTree->add(tree,"Viseme Map");	
	visemeTree->user_data((void*)ITEM_VISEME_MAP);
	int numVisemes = faceDefinition->getNumVisemes();
	for (int v = 0; v < numVisemes; v++)
	{
		std::string visemeName = faceDefinition->getVisemeName(v);
		std::string motionName = "";
		if (faceDefinition->getVisemeMotion(visemeName))
			motionName = faceDefinition->getVisemeMotion(visemeName)->name();
		Fl_Tree_Item* item = resourceTree->add(visemeTree,(visemeName+"-->"+motionName).c_str());
		item->user_data((void*)ITEM_VISEME_MAP);		
	}
}

void ResourceWindow::updatePath( Fl_Tree_Item* tree, srPathList& pathList )
{
	resourceTree->clear_children(tree);	
	pathList.reset();
	std::string pathName = pathList.next_path();
	while (pathName != "")
	{
		Fl_Tree_Item* item = resourceTree->add(tree,pathName.c_str());
		item->user_data(tree->user_data());
		pathName = pathList.next_path();
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
				string ext = extension( cur );	
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


void ResourceWindow::updateBoneMap( Fl_Tree_Item* tree, BoneMap* boneMap )
{
	for (unsigned int i=0;i<boneMap->map.size();i++)
	{
		std::string key = boneMap->map[i].first;
		std::string target = boneMap->map[i].second;

		
		Fl_Tree_Item* item = resourceTree->add(tree,(key+"-->"+target).c_str());	
		item->user_data((void*)ITEM_BONE_MAP);
		//resourceTree->add(item,target.c_str());
	}
}

void ResourceWindow::updateEventHandler( Fl_Tree_Item* tree, EventHandler* handler )
{
	//Fl_Tree_Item* item = resourceTree->add(tree,(handler->getType() + ":" + "\""+ handler->getAction() + "\"").c_str());
	//item->user_data((void*)ITEM_EVENT_HANDLERS);
}

void ResourceWindow::updateSkeleton( Fl_Tree_Item* tree, SkSkeleton* skel )
{
	std::string ext = boost::filesystem2::extension( skel->skfilename() );
	std::string filebase = boost::filesystem::basename(skel->skfilename());
	Fl_Tree_Item* item = resourceTree->add(tree,(filebase+ext).c_str());	
	item->user_data((void*)ITEM_SKELETON);	
}

void ResourceWindow::updateMotion( Fl_Tree_Item* tree, SkMotion* motion )
{
	Fl_Tree_Item* item = resourceTree->add(tree,motion->name().c_str());
	item->user_data((void*)ITEM_MOTION);
}

void ResourceWindow::updatePawn( Fl_Tree_Item* tree, SbmPawn* pawn )
{
	if (dynamic_cast<SbmCharacter*>(pawn) != NULL)
		return; // this is actually a character

	Fl_Tree_Item* item = resourceTree->add(tree,pawn->getName().c_str());
	item->user_data((void*)ITEM_PAWN);
}

void ResourceWindow::updateCharacter( Fl_Tree_Item* tree, SbmCharacter* character )
{
	Fl_Tree_Item* item = resourceTree->add(tree,character->getName().c_str());
	item->user_data((void*)ITEM_CHARACTER);
	resourceTree->sortorder(FL_TREE_SORT_NONE);	

	// add controllers
	MeControllerTreeRoot* ctTree = character->ct_tree_p ;
	if( ctTree )
	{
		int n = ctTree->count_controllers();
		for (int c = 0; c < n; c++)
		{
			//LOG( "%s", ctTree->controller(c)->name() );
			Fl_Tree_Item* ctrlItem = resourceTree->add(item,ctTree->controller(c)->getName().c_str());
			ctrlItem->user_data((void*)ITEM_CONTROLLER);
		}
	}
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
	lastClickedItemPath = pathName;	
	clearInfoWidget();	
	itemInfoWidget = createInfoWidget(resourceInfoGroup->x(),resourceInfoGroup->y(),resourceInfoGroup->w(),resourceInfoGroup->h(),ItemNameList[itemType].c_str(),treeItem,itemType);
	resourceInfoGroup->add(itemInfoWidget);
	resourceInfoGroup->show();
	itemInfoWidget->show();
	resourceInfoGroup->redraw();
}

void ResourceWindow::clearInfoWidget()
{
	if (itemInfoWidget)
	{
		resourceInfoGroup->remove(itemInfoWidget);
		delete itemInfoWidget;
		itemInfoWidget = NULL;
	}
}

TreeItemInfoWidget* ResourceWindow::createInfoWidget( int x, int y, int w, int h, const char* name, Fl_Tree_Item* treeItem, int itemType )
{
	TreeItemInfoWidget* widget = NULL;
	if (itemType == ITEM_SKELETON)
	{
		widget = new SkeletonItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_SEQ_PATH || itemType == ITEM_ME_PATH || itemType == ITEM_AUDIO_PATH || itemType == ITEM_MESH_PATH)
	{
		widget = new PathItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_SEQ_FILES)
	{
		widget = new SeqItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_MOTION)
	{
		widget = new MotionItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_PAWN)
	{
		widget = new PawnItemInfoWidget(x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_CHARACTER)
	{
		SbmCharacter* curChar = mcuCBHandle::singleton().getCharacter(treeItem->label());
		if (curChar)
			widget = new AttributeItemWidget(curChar,x,y,w,h,name,treeItem,itemType,this);
		else
			widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_PHYSICS)
	{
		widget = new AttributeItemWidget(mcuCBHandle::singleton().physicsEngine,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_SCENE)
	{
		widget = new AttributeItemWidget(&mcuCBHandle::singleton().scene,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_STEERING)
	{
		widget = new AttributeItemWidget(&mcuCBHandle::singleton().steerEngine,x,y,w,h,name,treeItem,itemType,this);
	}
	else if (itemType == ITEM_EVENT_HANDLERS)
	{
		widget = new EventItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	else if (itemType == ITEM_CONTROLLER)
	{
		SbmCharacter* curChar = mcuCBHandle::singleton().getCharacter(treeItem->parent()->label()); // a controller's parent is its character name
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
	else
	{
		widget = new TreeItemInfoWidget(x,y,w,h,name,treeItem,itemType);
	}
	return widget;
}

void ResourceWindow::notify( DSubject* subject )
{

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

