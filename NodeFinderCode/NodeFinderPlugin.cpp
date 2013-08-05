#include "NodeFinderPlugin.h"

using namespace C4;



C4::Plugin *ConstructPlugin(void)
{
	return (new NodeFinderPlugin);
}

NodeFinderPlugin::NodeFinderPlugin() {

}
void NodeFinderPlugin::Initialize(EditorObject *editorObject){
	editorObject->AddEditorPage(new NodeFinderEditorPage());
}

NodeFinderPlugin::~NodeFinderPlugin(){}
