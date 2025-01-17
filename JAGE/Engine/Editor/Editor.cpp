#include <Input/KeyboardInput.h>
#include <Input/MouseInput.h>
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
	MouseInput::Get().SetMouseCursorVisibility(mIsEditorActive);
}

void Editor::HandleInput()
{
	if (KeyboardInput::Get().IsKeyPressed(GLFW_KEY_F1)) {
		ToggleEditorMode();
	}
}

void Editor::RenderEditorWindows()
{
	if (mShowEntityViewer) {
		mEntityViewerWindow.Render();
	}
}
