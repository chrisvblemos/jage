#pragma once

#include <Core/Core.h>
#include "Viewers/EntityViewer.h"

class Editor {
public:
	Editor() {
		mEntityViewerWindow = EntityViewerWindow();
	};

	void Update(float dt);
	void ToggleEditorMode();

private:
	void HandleInput();
	void RenderEditorWindows();

	EntityViewerWindow mEntityViewerWindow;

	bool mIsEditorActive = false;
	bool mShowEntityViewer = true;
};