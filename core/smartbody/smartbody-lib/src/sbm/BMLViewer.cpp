#include "BMLViewer.h"

BMLViewer::BMLViewer(int x, int y, int w, int h)
{
}

void BMLViewer::label_viewer(std::string name)
{
}

void BMLViewer::show_bml_viewer()
{
}

void BMLViewer::hide_bml_viewer()
{
}

BMLViewerFactory::BMLViewerFactory()
{
}

BMLViewer* BMLViewerFactory::create(int x, int y, int w, int h)
{
	return new BMLViewer(x, y, w, h);
}


void BMLViewerFactory::destroy(BMLViewer* viewer)
{
	delete viewer;
}

