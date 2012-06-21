#ifndef _GENERICVIEWER_
#define _GENERICVIEWER_

#include <string>

class GenericViewer
{
	public:
		GenericViewer();
		GenericViewer(int x, int y, int w, int h);
		virtual ~GenericViewer();

		virtual void label_viewer(std::string name);
		virtual void show_viewer();
		virtual void hide_viewer();
		virtual void update_viewer();

};

class GenericViewerFactory
{
	public:
		GenericViewerFactory();
		
		virtual GenericViewer* create(int x, int y, int w, int h);
		virtual void destroy(GenericViewer* viewer);

};

#endif