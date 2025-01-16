#pragma once

#include <imgui/imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

#define LOG_DISPLAY(text, duration) LogDisplayWindow::Get().AddLog(text, duration, "")
#define LOG_DISPLAY_KEYED(text, key) LogDisplayWindow::Get().AddLog(text, 0.0f, key)

struct LogDisplayEntry {
	std::string text;
	float ttl = 2.0f;   // time to live of text
};

class LogDisplayWindow {
private:
	std::deque<LogDisplayEntry> unkeyedLogs;
	std::unordered_map<std::string, LogDisplayEntry> keyedLogs;
	LogDisplayWindow() = default;

public:
	// prevents copying
	LogDisplayWindow(const LogDisplayWindow&) = delete;
	LogDisplayWindow& operator=(const LogDisplayWindow&) = delete;

	static LogDisplayWindow& Get() {
		static LogDisplayWindow instance;
		return instance;
	}

	void AddLog(const std::string& text, const float duration = 2.0f, const std::string& key = "") {
		LogDisplayEntry newLogEntry;
		newLogEntry.text = text;
		newLogEntry.ttl = duration;

		if (key.empty())
			unkeyedLogs.emplace_back(LogDisplayEntry{text, duration});
		else
			keyedLogs[key] = LogDisplayEntry{ text, 0.0f };
	}

	void Update(float dt) {
		for (auto it = unkeyedLogs.begin(); it != unkeyedLogs.end();) {
			it->ttl -= dt;
			if (it->ttl <= 0.0f) {
				it = unkeyedLogs.erase(it);
			}
			else {
				++it;
			}
		}

		Render();
	}

	void Render() {
		const ImGuiIO& io = ImGui::GetIO();
		ImVec2 window_pos = ImVec2(io.DisplaySize.x - 10.0f, 10.0f);
		ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);

		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.1f);

		ImGui::Begin("LogDisplay", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings);

		for (const auto& it : keyedLogs) {
			ImGui::Text("%s: %s", it.first.c_str(), it.second.text.c_str());
		}

		for (const auto& it : unkeyedLogs) {
			ImGui::Text("%s", it.text.c_str());
		}


		ImGui::End();
	}
};