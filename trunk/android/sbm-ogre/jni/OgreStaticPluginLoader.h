/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    StaticPluginLoader.h
Description: Utility class to load plugins statically
-----------------------------------------------------------------------------
*/

#ifndef __StaticPluginLoader_H__
#define __StaticPluginLoader_H__

#include "Ogre.h"
#include "OgreGLES2Plugin.h"

namespace Ogre
{
	/** Utility class for loading some plugins statically.
	@remarks
		When loading plugins statically, you are limited to loading plugins 
		that are known about at compile time. You should define preprocessor
		symbols depending on which plugins you want to load - the symbol being
		OGRE_STATIC_<pluginname>, with pluginname being the usual name of the
		plugin DLL (no file extension, no debug suffix, and without the Plugin_ 
		or RenderSystem_ prefix.)
	*/
	class StaticPluginLoader
	{
	protected:
		GLES2Plugin* mGLES2Plugin;
	public:
		StaticPluginLoader() {}

		/** Load all the enabled plugins against the passed in root object. */
		void load()
		{
			Root& root  = Root::getSingleton();

			mGLES2Plugin = OGRE_NEW GLES2Plugin();
			root.installPlugin(mGLES2Plugin);

		}

		void unload()
		{
			// don't unload plugins, since Root will have done that. Destroy here.

			OGRE_DELETE mGLES2Plugin;
		}

	};

}

#endif
