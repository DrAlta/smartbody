#ifndef _RESOURCEWINDOW_
#define _RESOURCEWINDOW_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <sb/SBFaceDefinition.h>
#include <sb/SBService.h>
#include <sb/SBPhysicsSim.h>
#include <sb/SBSceneListener.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBAnimationState.h>
#include <sb/SBAnimationTransition.h>
#include <sbm/GenericViewer.h>
#include <sbm/sbm_deformable_mesh.h>
#include <sb/SBEvent.h>
#include "Fl_Tree_Horizontal.h" // a custom tree widget with horizontal scroll bar
#include "TreeItemInfoWidget.h"
#include "TreeInfoObject.h"

class srPathList;
class BoneMap;
class EventHandler;

class ResourceWindow;

class ResourceWindowListener : public SmartBody::SBSceneListener
{
	public:
		ResourceWindowListener(ResourceWindow* window);

		virtual void OnCharacterCreate( const std::string & name, const std::string & objectClass );
		virtual void OnCharacterDelete( const std::string & name );
		virtual void OnCharacterUpdate( const std::string & name );
      
		virtual void OnPawnCreate( const std::string & name );
		virtual void OnPawnDelete( const std::string & name );

		virtual void OnReset();

		virtual void OnSimulationStart();
		virtual void OnSimulationEnd();
		virtual void OnSimulationUpdate();
	
	private:
		ResourceWindow* _window;

};

class ResourceWindow : public Fl_Double_Window, public GenericViewer, public SmartBody::SBObserver
{
	public:
		enum {
			   ITEM_SCENE = 0,
			   ITEM_SERVICES,			   
			   ITEM_SEQ_PATH, 
			   ITEM_ME_PATH, 
			   ITEM_AUDIO_PATH, 
			   ITEM_MESH_PATH, 
			   ITEM_SEQ_FILES,
			   ITEM_SKELETON, 
			   ITEM_JOINT_MAP,
			   ITEM_DIPHONES,
			   ITEM_MOTION,
			   ITEM_ANIMATION_BLEND,
			   ITEM_BLEND_TRANSITION,
			   ITEM_MESH,
			   ITEM_FACE_DEFINITION,
			   ITEM_EVENT_HANDLERS,
			   ITEM_PAWN, 
			   ITEM_CHARACTER,   
			   ITEM_CONTROLLER,
			   ITEM_GESTUREMAP,
			   ITEM_NVBG,
			   ITEM_PHYSICS,
			   ITEM_NETURAL_MOTION,
			   ITEM_AU_MAP,
			   ITEM_VISEME_MAP,			   			  
			   ITEM_DEFAULT,
			   ITEM_SIZE };
		std::vector<std::string> ItemNameList;

		ResourceWindow(int x, int y, int w, int h, char* name);
		~ResourceWindow();
		
		virtual void label_viewer(std::string name);
		virtual void show_viewer();
		virtual void hide_viewer();
		virtual	void update_viewer();

		virtual void notify(SmartBody::SBSubject* subject);
		int handle(int event);
        virtual void show();      
		virtual void hide(); 
        void draw();
		void resize(int x, int y, int w, int h);
		void update();
        
		void selectPawn(const std::string& name);
		void updateGUI();				
		void updateTreeItemInfo(Fl_Tree_Item* treeItem, long itemType);
		static void refreshUI(Fl_Widget* widget, void* data);
		static void treeCallBack(Fl_Widget* widget, void* data);
	protected:		
		Fl_Tree* resourceTree;
		Fl_Tree_Item *pathTree;//, *skeletonTree, *motionTree, *meshTree;		
		//Fl_Tree_Item *pathSeqTree, *pathMeTree, *pathAudioTree;
		//Fl_Tree_Item *pawnTree, *characterTree;
		Fl_Tree_Item* treeItemList[ITEM_SIZE];
		Fl_Button    *refreshButton;
		Fl_Group     *resourceInfoGroup;
		TreeItemInfoWidget* itemInfoWidget;
		std::string lastClickedItemPath;
		std::vector<TreeItemInfoWidget*> widgetsToDelete;

		
	protected:
		void updatePath(Fl_Tree_Item* tree, const std::vector<std::string>& pathList);
		void updateSeqFiles(Fl_Tree_Item* tree, std::string pathName);
		void updateSkeleton(Fl_Tree_Item* tree, SmartBody::SBSkeleton* skel);
		void updateJointMap(Fl_Tree_Item* tree, SmartBody::SBJointMap* jointMap);
		void updateMotion(Fl_Tree_Item* tree, SmartBody::SBMotion* motion);
		void updateAnimationBlend(Fl_Tree_Item* tree, SmartBody::SBAnimationBlend* blend);
		void updateBlendTransition(Fl_Tree_Item* tree, SmartBody::SBAnimationTransition* transition);
		void updateMesh(Fl_Tree_Item* tree, DeformableMesh* mesh);
		void updatePawn(Fl_Tree_Item* tree, SmartBody::SBPawn* pawn);
		void updateCharacter(Fl_Tree_Item* tree, SmartBody::SBCharacter* character);
		void updatePhysicsCharacter(Fl_Tree_Item* tree, SmartBody::SBPhysicsCharacter* phyChar);
		void updateService(Fl_Tree_Item* tree, SmartBody::SBService* service);
		void updatePhysicsManager(Fl_Tree_Item* tree, SmartBody::SBPhysicsManager* phyService);
		void updateFaceMotion(Fl_Tree_Item* tree, SmartBody::SBFaceDefinition* faceDefinition);
		void updateEventHandler(Fl_Tree_Item* tree, SmartBody::SBEventHandler* handler);
		bool processedDragAndDrop(std::string& dndText);
		int  findTreeItemType(Fl_Tree_Item* treeItem);
		void clearInfoWidget(TreeItemInfoWidget* lastWidget);
		TreeItemInfoWidget* createInfoWidget(int x, int y, int w, int h, const char* name, Fl_Tree_Item* treeItem, int itemType );
		ResourceWindowListener* _listener;
};

 class ResourceViewerFactory : public GenericViewerFactory
 {
	public:
		ResourceViewerFactory();

		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);
 };
#endif
