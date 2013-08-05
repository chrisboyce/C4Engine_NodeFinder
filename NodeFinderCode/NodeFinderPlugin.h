#ifndef SceneGraphPlugin_h
#define SceneGraphPlugin_h

#include "C4EditorPlugins.h"
#include "C4EditorSupport.h"
#include "NodeFinderPage.h"

extern "C"
{
	C4MODULEEXPORT C4::Plugin *ConstructPlugin(void);
}

namespace C4
{
	class NodeFinderPlugin : public EditorPlugin
	{
	public:
		NodeFinderPlugin();
		~NodeFinderPlugin();

		C4MODULEEXPORT void Initialize(EditorObject *editorObject);

		const char *GetPluginName(void) const {
			return "NodeFinder";
		}
	};
}
#endif