#include <Core/Core.h>
#include <Core/Window.h>
#include "Editor.h"


void Editor::Update(float dt)
{
	HandleInput();
	if (mIsEditorActive) {
		RenderEditorWindows();
	}
}

void Editor::ToggleEditorMode()
{
	mIsEditorActive = !mIsEditorActive;
	Window::Get().SetMouseCursorVisibility(mIsEditorActive);
}

void Editor::HandleInput()
{
	if (Input::Get().GetKeyWasPressed(GLFW_KEY_F1)) {
		ToggleEditorMode();
	}
}

void Editor::RenderEditorWindows()
{
	if (mShowEntityViewer) {
		mEntityViewerWindow.Render();
	}
}
