/*
Copyright (C) 2013 Chris Boyce <chris@gitanic.com>

See the LISCENSE.txt file at the top-level directory of 
this distribution and at http://opensource.org/licenses/MIT
*/
#ifndef NodeFinderEditorPage_h
#define NodeFinderEditorPage_h

#include "C4EditorPages.h"
#include "C4EditorSupport.h"

namespace C4
{
	/**
	 * ListWidget container item for search results
	 *
	 * Any Widget can be displayed as an item in the ListWidget. 
	 * The ResultListItemWidget simply extends a TextWidget to
	 * allow a text result to be displayed, as well as storing
	 * a reference to the matching node.
	 */
	class ResultListItemWidget : public TextWidget{
		private:
			Node * node;
		public:
			ResultListItemWidget(const char * label, Node * node);
			Node * GetNode(){return node;}
	};


	class NodeFinderEditorPage : public EditorPage{

		private:
			void Requery(void);
			void Move(void);
			ListWidget * resultListWidget;		
			TextWidget * queryTextWidget;
			TextButtonWidget * settingsButtonWidget;
		
			/* When the user has opted to only search visible nodes,
			we have to determine when the viewports have changed to 
			requery the list. To detect this change, we store the
			determinants of each viewport each frame. 
			*/
			bool searchVisibleOnly;
			Transform4D viewportTransformDeterminants[kEditorViewportCount];


			Vector2D previousViewportData[kEditorViewportCount];
			Camera * visibilityCameras[kEditorViewportCount];

			IconButtonWidget * orthoFilterButton;
			IconButtonWidget * perspectiveFilterButton;

			CheckWidget * frameSelectionButton;
			CheckWidget * searchVisibleOnlyCheckWidget;
		

			const char * GetNodeDisplayName(const Node * node);
			StringTable							worldEditorStringTable;
		
			bool needsUpdate;

			
			bool showOrthoMatches;
			bool showPerspectiveMatches;

			bool focusSelection;
		
			bool touchedSelection;
			void SelectListNode(Node * node);
			bool ViewportCameraMoved(int32 viewportIndex);
			void UpdateFilterCheckStates(void);

			EditorObserver<NodeFinderEditorPage>		editorObserver;

			WidgetObserver<NodeFinderEditorPage>		queryObserver;
			WidgetObserver<NodeFinderEditorPage>		listObserver;
			WidgetObserver<NodeFinderEditorPage>		focusCheckObserver;
			WidgetObserver<NodeFinderEditorPage>		searchVisibleOnlyObserver;
			WidgetObserver<NodeFinderEditorPage>		orthoFilterObserver;
			WidgetObserver<NodeFinderEditorPage>		perspectiveFilterObserver;

			void HandleEditorEvent(Editor *editor, const EditorEvent& event);
			void HandleListEvent(Widget * widget, const WidgetEventData * eventData);
			void HandleQueryEvent(Widget *widget, const WidgetEventData *eventData);
			void HandleFocusCheckEvent(Widget * widget, const WidgetEventData *eventData);
			void HandleSearchOnlyVisibleEvent(Widget * widget, const WidgetEventData * eventData);
			void Preprocess(void);

		public:
			NodeFinderEditorPage();
			~NodeFinderEditorPage();
		
			ResultListItemWidget * CreateListItemWidget(Node * node, const char * defaultLabel);

			void AddNodeToList(Node * node);
	};
}

#endif NodeFinderEditorPage_h