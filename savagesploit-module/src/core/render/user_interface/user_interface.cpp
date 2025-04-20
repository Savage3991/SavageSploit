//
// Created by savage on 18.04.2025.
//

#include "user_interface.h"

#include "entry.h"
#include "globals.h"
#include "imgui.h"
#include "TextEditor.h"
#include "src/rbx/taskscheduler/taskscheduler.h"

void user_interface::render() {

    ImGui::SetNextWindowSize(ImVec2(450,230), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(450, 230), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::Begin("SavageSploit | 100% Real.")) {

        ImVec2 content_avail = ImGui::GetContentRegionAvail();
        content_avail.y -= ImGui::GetFrameHeightWithSpacing();

        static std::string buffer(16384, '\0');

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
        if (ImGui::InputTextMultiline("##editor", buffer.data(), buffer.size() + 1, content_avail, flags,
            [](ImGuiInputTextCallbackData* data) {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                    auto* str = static_cast<std::string*>(data->UserData);
                    str->resize(data->BufTextLen);
                    data->Buf = str->data();
                }
                return 0;
            }, &buffer)) {}


        if (ImGui::Button("Execute :P")) {
            g_taskscheduler->queue_script(buffer);
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear da Editor :P")) {
            buffer = "";
        }

        ImGui::SameLine();

        if (ImGui::Button("Reload SavageSploit")) {
            g_entry_point->entry(globals::dll_module);
        }
    }

    ImGui::End();
}
