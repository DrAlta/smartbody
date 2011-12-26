
#include "pandaFramework.h"
#include "pandaSystem.h"

#include "PandaRenderer.h"

 
PandaFramework framework;
 
int main(int argc, char *argv[]) {
	PandaFramework framework;
    //open a new window framework
  framework.open_framework(argc, argv);
    //set the window title to My Panda3D Window
  framework.set_window_title("SmartBody Panda3D");
    //open the window
  WindowFramework* window = framework.open_window();
  window->setup_trackball();
 
  PandaRenderer panda(&framework, window);

  PT(AsyncTaskManager) taskMgr = AsyncTaskManager::get_global_ptr(); 
  taskMgr->add(&panda);
 
    //do the main loop, equal to run() in python
  framework.main_loop();
    //close the window framework
  framework.close_framework();
  return (0);
}