#include "client.hpp"

namespace C4 {

int Client::UI::run() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    printf("Error: SDL_Init(): %s\n", SDL_GetError());
    return -1;
  }
  m_glsl_version = nullptr;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  m_sdl_window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                       SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  m_sdl_window = SDL_CreateWindow("Connect4", (int)(480 * main_scale),
                                  (int)(240 * main_scale), m_sdl_window_flags);
  if (m_sdl_window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return -1;
  }
  m_sdl_gl_context = SDL_GL_CreateContext(m_sdl_window);
  if (m_sdl_gl_context == nullptr) {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
    return -1;
  }

  SDL_GL_MakeCurrent(m_sdl_window, m_sdl_gl_context);
  SDL_GL_SetSwapInterval(1);
  SDL_SetWindowPosition(m_sdl_window, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(m_sdl_window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui::StyleColorsLight();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  ImGui_ImplSDL3_InitForOpenGL(m_sdl_window, m_sdl_gl_context);
  ImGui_ImplOpenGL3_Init(m_glsl_version);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  m_done = false;
  while (!m_done) {
    while (SDL_PollEvent(&m_sdl_event)) {
      ImGui_ImplSDL3_ProcessEvent(&m_sdl_event);
      if (m_sdl_event.type == SDL_EVENT_QUIT)
        m_done = true;
      if (m_sdl_event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          m_sdl_event.window.windowID == SDL_GetWindowID(m_sdl_window))
        m_done = true;
    }

    if (SDL_GetWindowFlags(m_sdl_window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    int window_width, window_height;
    SDL_GetWindowSizeInPixels(m_sdl_window, &window_width, &window_height);
    ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));

    imgui();

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_sdl_window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DestroyContext(m_sdl_gl_context);
  SDL_DestroyWindow(m_sdl_window);
  SDL_Quit();

  return 0;
}

} // namespace C4