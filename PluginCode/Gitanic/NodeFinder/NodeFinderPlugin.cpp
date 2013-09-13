/*
Copyright (C) 2013 Chris Boyce <chris@gitanic.com>

See the LISCENSE.txt file at the top-level directory of 
this distribution and at http://opensource.org/licenses/MIT
*/

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
