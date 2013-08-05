#include "NodeFinderPage.h"

using namespace C4;

ResultListItemWidget::ResultListItemWidget(const char * label, Node * node) : TextWidget(label,"font/Gui"){
	this->node = node;
}

NodeFinderEditorPage::NodeFinderEditorPage(): EditorPage('SGPL',"NodeFinder/panels/editor_page"), 
	queryObserver(this,&NodeFinderEditorPage::HandleQueryEvent),
	listObserver(this,&NodeFinderEditorPage::HandleListEvent),
	focusCheckObserver(this,&NodeFinderEditorPage::HandleFocusCheckEvent),
	searchVisibleOnlyObserver(this,&NodeFinderEditorPage::HandleSearchOnlyVisibleEvent),
	orthoFilterObserver(this,&NodeFinderEditorPage::HandleSearchOnlyVisibleEvent),
	perspectiveFilterObserver(this,&NodeFinderEditorPage::HandleSearchOnlyVisibleEvent),
	editorObserver(this,&NodeFinderEditorPage::HandleEditorEvent),

	worldEditorStringTable("WorldEditor/strings"){


		for(int32 i = 0; i < kEditorViewportCount; i++){
			viewportTransformDeterminants[i] = Transform4D();
			previousViewportData[i] = Vector2D();
			visibilityCameras[i] = nullptr;
		}
		
		needsUpdate = false;

		searchVisibleOnly = false;
		showOrthoMatches = true;
		showPerspectiveMatches = true;

		
		focusSelection = false;
		touchedSelection = false;
}
bool NodeFinderEditorPage::ViewportCameraMoved(int32 viewportIndex){
	if(GetEditor()->GetViewport(viewportIndex)->GetEditorViewportType() == kEditorViewportOrtho){
		OrthoViewportWidget * viewportWidget = static_cast<OrthoViewportWidget*>(GetEditor()->GetViewport(viewportIndex)->GetViewportWidget());
		if(viewportWidget->GetOrthoScale() != previousViewportData[viewportIndex]){
			previousViewportData[viewportIndex] = viewportWidget->GetOrthoScale();
			return true;
		}
	}
		
	if(GetEditor()->GetViewport(viewportIndex)->GetViewportWidget()->GetViewportCamera()->GetWorldTransform() != viewportTransformDeterminants[viewportIndex]){
		viewportTransformDeterminants[viewportIndex] = GetEditor()->GetViewport(viewportIndex)->GetViewportWidget()->GetViewportCamera()->GetWorldTransform();
		return true;
	} else {
		return false;
	}
}
void NodeFinderEditorPage::Move(){
	EditorPage::Move();


	if(searchVisibleOnly){
		EditorViewport * viewport;
		//needsUpdate = ViewportCameraMoved(GetEditor()->GetEditorObject()->GetFullViewportIndex());
		for(int i = 0 ;i < kEditorViewportCount; i++){
			visibilityCameras[i] = nullptr;
			if(GetEditor()->GetViewport(i)->GetViewportWidget()->Visible()){
				viewport = GetEditor()->GetViewport(i);
				//TheEngine->Report("Visible:");
				//TheEngine->Report(Text::IntegerToString(i));
				if( 
					(viewport->GetEditorViewportType() == kEditorViewportOrtho && showOrthoMatches) ||
					(viewport->GetEditorViewportType() == kEditorViewportFrustum && showPerspectiveMatches))
				{
					if(needsUpdate){
						visibilityCameras[i] = viewport->GetViewportWidget()->GetViewportCamera();
					} else if(ViewportCameraMoved(i)){
						TheEngine->Report("Moved:");
						TheEngine->Report(Text::IntegerToString(i));
						visibilityCameras[i] = viewport->GetViewportWidget()->GetViewportCamera();
						needsUpdate = true;
					}
				}
			}
		}
	}
	
	if(needsUpdate){
		needsUpdate = false;
		Requery();
	}
}

void NodeFinderEditorPage::AddNodeToList(Node * node){

}
void NodeFinderEditorPage::SelectListNode(Node * node){
	
	for(int32 i = 0; i < resultListWidget->GetListItemCount() ; i++){
		if( (static_cast<ResultListItemWidget*>(resultListWidget->GetListItem(i)))->GetNode() == node){
			resultListWidget->SelectListItem(i);
		}
	}
}
void NodeFinderEditorPage::HandleEditorEvent(Editor *editor, const EditorEvent& event){
	if(event.GetEventType() == kEditorEventSelectionUpdated){
		if(!touchedSelection){
			if(!searchVisibleOnly){
				return;
			}
			resultListWidget->UnselectAllListItems();
			const List<NodeReference> * selectionList = editor->GetSelectionList();
			const NodeReference *reference = selectionList->First();
			while (reference != nullptr){
				SelectListNode(reference->GetNode());
				reference = reference->Next();
			}
		} else {
			touchedSelection = false;
		}
	} 
}

void NodeFinderEditorPage::UpdateFilterCheckStates(void){
	if(searchVisibleOnly){
		orthoFilterButton->Enable();
		perspectiveFilterButton->Enable();
	} else {
		orthoFilterButton->Disable();
		perspectiveFilterButton->Disable();
	}
	
}
void NodeFinderEditorPage::HandleSearchOnlyVisibleEvent(Widget * widget, const WidgetEventData * eventData){
	if(widget == searchVisibleOnlyCheckWidget){
		searchVisibleOnly = (static_cast<CheckWidget*>(widget))->GetValue() != 0;
		UpdateFilterCheckStates();
		if(searchVisibleOnly){
			focusSelection = false;
			frameSelectionButton->SetValue(0);
		}
	} else if(widget == orthoFilterButton){
		showOrthoMatches = orthoFilterButton->GetValue() != 1;
		orthoFilterButton->SetValue((showOrthoMatches) ? 1 : 0);
	} else if(widget == perspectiveFilterButton){
		showPerspectiveMatches = perspectiveFilterButton->GetValue() != 1;
		perspectiveFilterButton->SetValue((showPerspectiveMatches) ? 1 : 0);
	}
	needsUpdate = true;
}

void NodeFinderEditorPage::HandleFocusCheckEvent(Widget * widget, const WidgetEventData * eventData){
	focusSelection = (static_cast<CheckWidget*>(widget))->GetValue() != 0;
	if(focusSelection){
		if(searchVisibleOnly){
			searchVisibleOnly = false;
			searchVisibleOnlyCheckWidget->SetValue(0);
			UpdateFilterCheckStates();
		}
		GetEditor()->FrameSelectionAllViewports();
	}
}


void NodeFinderEditorPage::Requery(void){
	resultListWidget->PurgeListItems();
	const char * queryText = queryTextWidget->GetText();
	const char * nodeName = nullptr;
	
	Node * root = GetEditor()->GetRootNode();
	Node *node = root;
	
	/*
	Search by
		Node type (http://www.terathon.com/c4engine/doco/WorldMgr/Node_GetNodeType.html)
	*/
	
	do
	{
		

		nodeName = GetNodeDisplayName(node);
		if(nodeName != nullptr){
			if(Text::FindText(nodeName,queryText) >= 0){
				if(searchVisibleOnly){
					for(int i = 0; i < kEditorViewportCount; i++){
						if(!visibilityCameras[i]) { continue; }
						if(visibilityCameras[i]->SphereVisible(node->GetWorldPosition(),0.1f)){
							resultListWidget->AppendListItem(CreateListItemWidget(node,nodeName));
							break;
						}
					}
				} else {
					resultListWidget->AppendListItem(CreateListItemWidget(node,nodeName));	
				} 
			}
		}
		
		//Skip instance/world nodes
		if(node->GetNodeType() == kNodeInstance){
			node = root->GetNextLevelNode(node);
		} else {
			node = root->GetNextNode(node);
		}
	} while (node);
}
void NodeFinderEditorPage::HandleListEvent(Widget *widget, const WidgetEventData *eventData)
{
	Editor * editor = GetEditor();
	editor->UnselectAll();

	ResultListItemWidget * listItemWidget = static_cast<ResultListItemWidget*>(resultListWidget->GetFirstSelectedListItem());
	while(listItemWidget != nullptr){
		if(listItemWidget->GetNode() == nullptr){
			needsUpdate = true;
		} else {
			editor->SelectNode(listItemWidget->GetNode());
			listItemWidget = static_cast<ResultListItemWidget*>(resultListWidget->GetNextSelectedListItem(listItemWidget));
		}
	}
	if(focusSelection){
		editor->FrameSelectionAllViewports();
	}
	touchedSelection = true;
}

ResultListItemWidget * NodeFinderEditorPage::CreateListItemWidget(Node * node, const char * label){
	Vector2D widgetSize = resultListWidget->GetNaturalListItemSize();
	ResultListItemWidget * listItemWidget = new ResultListItemWidget(label,node);	
	listItemWidget->SetWidgetSize(widgetSize);
	return listItemWidget;
}
void NodeFinderEditorPage::HandleQueryEvent(Widget *widget, const WidgetEventData *eventData)
{
	needsUpdate = true;
}
const char * NodeFinderEditorPage::GetNodeDisplayName(const Node * node){
	/* The name as defined in the Node Info tab */
		const char * nodeName = node->GetNodeName();

		/* If no node name is set, it uses the best descriptor it can find, like "Landscape Light" */
		const char * specificName = nullptr;

		NodeType curNodeType = node->GetNodeType();
		if(nodeName != nullptr){
			specificName = nodeName;
		} else if(curNodeType == kNodeLight){
			const Light * light = static_cast<const Light*>(node);
			specificName = worldEditorStringTable.GetString(StringID('NAME','LITE',light->GetLightType()));
		} else if(curNodeType == kNodeModel){
			const Model * model = static_cast<const Model*>(node);
			const ModelType modelType = static_cast<const Model*>(node)->GetModelType();
			const ModelRegistration * registration = Model::FindRegistration(modelType);
			if (registration != nullptr){
				specificName = registration->GetModelName();
			} else {
				specificName = Text::TypeToString(modelType);
			}
		} else if(curNodeType == kNodeGeometry){
			const Geometry * geometry = static_cast<const Geometry*>(node);
			const GeometryType geometryType = geometry->GetGeometryType();
			if(geometryType == kGeometryPrimitive){
				const PrimitiveGeometry * primitiveGeometry = static_cast<const PrimitiveGeometry*>(geometry);
				specificName = worldEditorStringTable.GetString(StringID('NAME','GEOM',primitiveGeometry->GetPrimitiveType()));
			} else {
				specificName = worldEditorStringTable.GetString(StringID('NAME','GEOM'));	
			}
		} else if(curNodeType == kNodeImpostor){
			specificName = worldEditorStringTable.GetString(StringID('NAME','IPST','IPST'));
		} else if(curNodeType == kNodeInstance){
			const Instance *instance = static_cast<const Instance*>(node);
			specificName = instance->GetWorldName();
		} else if(curNodeType == kNodeZone){
			const Zone * zone = static_cast<const Zone*>(node);
			specificName = worldEditorStringTable.GetString(StringID('NAME','ZONE',zone->GetZoneType()));
		} else if(curNodeType == kNodePortal){
			const Portal * portal = static_cast<const Portal*>(node);
			specificName = worldEditorStringTable.GetString(StringID('NAME','PORT',portal->GetPortalType()));
		}
		return specificName;
		
}
void NodeFinderEditorPage::Preprocess(){
	EditorPage::Preprocess();

	RootWidget *root = GetRootWidget();
	if (root) root->AddMovingWidget(this);



	GetEditor()->AddObserver(&editorObserver);

	queryTextWidget = static_cast<TextWidget *>(FindWidget("query"));
	queryTextWidget->SetObserver(&queryObserver);

	resultListWidget = static_cast<ListWidget*>(FindWidget("resultList"));
	resultListWidget->SetObserver(&listObserver);

	frameSelectionButton = static_cast<CheckWidget*>(FindWidget("focusCheckbox"));
	frameSelectionButton->SetObserver(&focusCheckObserver);
	frameSelectionButton->SetValue( (focusSelection) ? 1 : 0);

	searchVisibleOnlyCheckWidget = static_cast<CheckWidget*>(FindWidget("srchVisibleChck"));
	searchVisibleOnlyCheckWidget->SetObserver(&searchVisibleOnlyObserver);
	searchVisibleOnlyCheckWidget->SetValue( (searchVisibleOnly) ? 1 : 0);

	orthoFilterButton = static_cast<IconButtonWidget*>(FindWidget("showOrthoBtn"));
	orthoFilterButton->SetObserver(&searchVisibleOnlyObserver);
	orthoFilterButton->SetValue( (showOrthoMatches) ? 1 : 0);

	perspectiveFilterButton = static_cast<IconButtonWidget*>(FindWidget("showPerspBtn"));
	perspectiveFilterButton->SetObserver(&searchVisibleOnlyObserver);
	perspectiveFilterButton->SetValue( (showPerspectiveMatches) ? 1 : 0);
	UpdateFilterCheckStates();

}
NodeFinderEditorPage::~NodeFinderEditorPage(){

}
