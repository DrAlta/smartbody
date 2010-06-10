#ifndef _BMLVIEWER_
#define _BMLVIEWER_

#include <string>

class BMLViewer
{
	public:
		BMLViewer(int x, int y, int w, int h);

		virtual void label_viewer(std::string name);
		virtual void show_bml_viewer();
		virtual void hide_bml_viewer();

};

class BMLViewerFactory
{
	public:
		BMLViewerFactory();
		
		virtual BMLViewer* create(int x, int y, int w, int h);
		virtual void destroy(BMLViewer* viewer);

};

#endif