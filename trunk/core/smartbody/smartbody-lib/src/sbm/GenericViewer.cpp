#include "GenericViewer.h"

GenericViewer::GenericViewer(int x, int y, int w, int h)
{
}

void GenericViewer::label_viewer(std::string name)
{
}

void GenericViewer::show_viewer()
{
}

void GenericViewer::hide_viewer()
{
}

GenericViewerFactory::GenericViewerFactory()
{
}

GenericViewer* GenericViewerFactory::create(int x, int y, int w, int h)
{
	return new GenericViewer(x, y, w, h);
}


void GenericViewerFactory::destroy(GenericViewer* viewer)
{
	delete viewer;
}

