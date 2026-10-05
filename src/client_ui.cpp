#include "client.hpp"

namespace C4 {

Client::UI::UI(Client *client) {
  m_client = client;
  m_show_msgbox_username_check_failed = false;
}
Client::UI::~UI() {}

void Client::UI::imgui_username() {
  ImGui::Text("Please enter username");
  ImGui::Text("Username:");
  ImGui::SameLine();
  ImGui::InputText("##username", m_client->m_username,
                   sizeof(m_client->m_username) - 1);
  if (m_client->m_is_connected) {
    ImGui::Text("Server connected");
  } else {
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Server disconnected");
  }
  ImGui::BeginDisabled(!m_client->m_is_connected);
  if (ImGui::Button("Enter")) {
    m_client->enter_lobby();
  }
  ImGui::EndDisabled();

  if (m_show_msgbox_username_check_failed) {
    ImGui::OpenPopup("Username unvailable");
    m_show_msgbox_username_check_failed = false;
  }

  if (ImGui::BeginPopupModal("Username unvailable", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("The username is already taken, please try another.");
    ImGui::Separator();
    if (ImGui::Button("OK", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void Client::UI::imgui_lobby() {}

void Client::UI::imgui() {
  ImGui::Begin("Connect4", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  if (m_client->m_state == CLIENT_STATE_USERNAME) {
    imgui_username();
  } else if (m_client->m_state == CLIENT_STATE_LOBBY) {
    imgui_lobby();
  }

  ImGui::End();
}

}; // namespace C4