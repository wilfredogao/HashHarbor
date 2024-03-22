#include "../../imgui/imgui.h"
#include "../../backends/imgui_impl_sdl.h"
#include "../../backends/imgui_impl_opengl3.h"
#include <stdio.h>

#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else

#include <SDL2/SDL_opengl.h>
#endif

#if defined(__APPLE__)
#include <iostream>
#include <vector>
#include <fstream>
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::pair;
using std::ofstream;
#else
#include <bits/stdc++.h>
#endif

#include "graphic.h"
#include "textEditor/TextEditor.h"
#include "../imageHandler/imageHandler.h"
#include "../imageHandler/imagePath.h"
#include "../character/characterManager.h"
#include "../character/characterBuilder.h"
#include "../movement/movementHandler.h"
#include "../assets/font/IconsFontAwesome6.h"
#include "../login/login.h"
#include "database/database.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"


void graphic::setup(){
    TextEditor editor;
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

	// set your own known preprocessor symbols...
	static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
	// ... and their corresponding values
	static const char* ppvalues[] = { 
		"#define NULL ((void*)0)", 
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ", 
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0", 
		"enum D3D_FEATURE_LEVEL", 
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
        "    (!!(expression)) ||                                                              \n"
        "    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
        " )"
		};

	for (long unsigned int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] = 
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (long unsigned int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);
	//editor.SetPalette(TextEditor::GetLightPalette());

	// error markers
	TextEditor::ErrorMarkers markers;
	markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	editor.SetErrorMarkers(markers);

    vector<string> cppStart;
    cppStart.push_back("#include <iostream>");
    cppStart.push_back("int main() {");
    cppStart.push_back("\tstd::cout << \"Hello World!\";");
    cppStart.push_back("\treturn 0;");
    cppStart.push_back("}");
    editor.SetTextLines(cppStart);

    static const char* fileToEdit = "solution.cpp";

    //texteditor setup ends here

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version;
#if defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    glsl_version = "#version 130";

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif


    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->width_px, this->height_px, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    imagePath imgPth = imagePath();
    // Font Icon set up
    io.Fonts->AddFontDefault();
    float baseFontSize = 25.0f; // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
#if defined(__APPLE__)
    string font_1 = imgPth.currentPath.string() + FONT_ICON_FILE_NAME_FAR;
    string font_2 = imgPth.currentPath.string() + FONT_ICON_FILE_NAME_FAS;
#else
    string font_1 = string("..") + FONT_ICON_FILE_NAME_FAR;
    string font_2 = string("..") + FONT_ICON_FILE_NAME_FAS;
#endif
    io.Fonts->AddFontFromFileTTF(font_1.c_str(), iconFontSize, &icons_config, icons_ranges );
    io.Fonts->AddFontFromFileTTF( font_2.c_str(), iconFontSize, &icons_config, icons_ranges );


    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    imageHandler image = imageHandler();
    characterManager character = characterManager();
    characterBuilder builder = characterBuilder(&image);

    database& db = database::getInstance();
    //db.connect();
    login Login = login(width_px, height_px, &image);

    string pathMap;
#if defined(__APPLE__)
    pathMap = imgPth.currentPath.string() + "/assets/map/abc.png";
#else
    pathMap = "../src/abc.png";
#endif
    imageHandler background = imageHandler(pathMap.c_str());
    background.loadTexture(background.filepath, &background);

    character.createCharacter("Bob", false, true, &image);
    character.setMainPlayer("Bob");

    movementHandler move = movementHandler(pathMap);
    // auto gr = obs.getGrid();
    // for(uint i = 0; i < gr.size(); i++){
    //     for(uint j = 0; j < gr[0].size(); j++){
    //         cout << gr[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    float mapGridX = 15.0f;
    float mapGridY = 8.0f;
    
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        if(changeResolution)
        {
            SDL_SetWindowSize(window, width_px, height_px);
            changeResolution = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if(ImGui::IsKeyPressedEx(ImGuiKey_Escape, false))
        {
            if(characterCreated && !show_login)
            {
                show_settings = !show_settings;
                show_display = !show_display;
                show_process = !show_process;
                show_config = !show_config;

                resetPauseScreen = true;
            }
        }

        if(show_login)
        {
            makeLogIn(Login, image);
        }

        if(show_settings)
        {
            makeSettings(Login, image, character, builder, done);
        }

        if(show_display){
            makeBackground(background, move, mapGridX, mapGridY);
            makeDisplay(image, character, builder);
        }
        
        if(show_process){
            makeProcess(editor, fileToEdit); 
        }

        if(show_config){
            makeConfig();
        }

        if(show_charSelector)
        {
            makeCharacterSelector(image, character, builder);
        }
        
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup

    builder.cleanUp();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void graphic::makeDisplay(imageHandler& image, characterManager &character, characterBuilder& charBuild)
{
    // Graphics window calculation
    ImGui::SetNextWindowSize({(float)width_px /2, (float)height_px / 2});
    ImGui::SetNextWindowPos({0, 0});

    const float frameLength = 1.f / 10.f; // In seconds, so  FPS
    static float frameTimer = frameLength;

    // Window - Graphics
    ImGui::Begin("Graphics", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    {
        if(characterCreated && !show_settings)
        {
            frameTimer -= ImGui::GetIO().DeltaTime;

            ImVec2 characterPos = ImVec2((ImGui::GetContentRegionAvail() - ImVec2(32, 64)) * 0.5f);
            character.drawPos = characterPos;

            ImGui::SetCursorPos(characterPos);
            character.moveMainCharacter(&image, &charBuild, frameTimer);

            if (frameTimer <= 0.f)
            {
                frameTimer = 2.5f / 10.f;
            }
        }
    }
    ImGui::End();
}

string result = "";

void graphic::makeProcess(TextEditor &editor, const char* fileToEdit){
    // Processor information window calculation
    ImGui::SetNextWindowSize({(float)width_px / 2,(float)height_px / 4 * 3});
    ImGui::SetNextWindowPos({(float)width_px / 2, 0});

    auto cpos = editor.GetCursorPosition();
    editor.GetText();

    ImGui::Begin("Sandbox", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    {
        // if (ImGui::BeginMenu("File")) {
        //     if (ImGui::MenuItem("New", "Ctrl+N")) {
        //         // Handle New action
        //     }
        //     if (ImGui::MenuItem("Open", "Ctrl+O")) {
        //         // Handle Open action
        //     }
        //     if (ImGui::MenuItem("Save", "Ctrl+S")) {
        //         // Handle Save action
        //     }
        //     ImGui::Separator();
        //     if (ImGui::MenuItem("Exit", "Alt+F4")) {
        //         // Handle Exit action
        //     }
        //     ImGui::EndMenu();
        // }

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), fileToEdit);

		editor.Render("TextEditor");
    }
    ImGui::End();

    ImGui::SetNextWindowSize({(float)width_px / 2, (float)height_px / 4});
    ImGui::SetNextWindowPos({(float)width_px / 2, (float)height_px / 4 * 3});

    // Window - Processor Information
    ImGui::Begin("Submission details", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    {
        if(ImGui::Button("Run Code")){
            string textToSave = editor.GetText();

            result = executeCPP(textToSave);
            cout << result << endl;
        }

        ImGui::InputTextMultiline("Result", const_cast<char*>(result.c_str()), result.size() + 1, ImVec2(500, 200), ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();
}

void graphic::makeCharacterSelector(imageHandler& image, characterManager &character, characterBuilder& charBuild)
{
    ImGui::SetNextWindowSize({ImGui::GetIO().DisplaySize.x-550.f, ImGui::GetIO().DisplaySize.y-200.f});
    ImGui::SetNextWindowPos({275.f,100.f});

    float factor = 4.f;
    const float frameLength = 5.f / 10.f; // In seconds, so  FPS
    static float frameTimer = frameLength;

    // Window - Config
    ImGui::Begin("Character Selector", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav);
    {
        frameTimer -= ImGui::GetIO().DeltaTime;
        ImVec2 characterPos = ImVec2((ImGui::GetContentRegionAvail() - ImVec2((32.f * factor), (64.f * factor))) * 0.25f);
        charBuild.drawPos = ImVec2(characterPos.x * 3.f, characterPos.y * 2.f);
        charBuild.drawCharacterBuilder(&image, frameTimer);

        if (frameTimer <= 0.f)
        {
            frameTimer = 5.f / 10.f;
        }

        //---setchar
        ImGui::SetCursorPos(ImVec2(290.f,390.f));
        ImGui::PushID(8);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
        if(ImGui::Button("Select Character", ImVec2(150.f, 40.f)))
        {
            character.selectMainCharacter(&charBuild);
            characterCreated = true;
            show_charSelector = false;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::PopID();
    }
    ImGui::End();
}

void graphic::makeBackground(imageHandler background, movementHandler move, float &gridX, float &gridY){
    ImGui::SetNextWindowSize({(float)width_px /2, (float)height_px / 2});
    ImGui::SetNextWindowPos({0, 0});

    static auto lastKeyEventTime = std::chrono::steady_clock::now();

    // Get the current time
    auto currentTime = std::chrono::steady_clock::now();

    // Calculate the time elapsed since the last key event
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastKeyEventTime).count();

    // Define the cooldown duration between key events in milliseconds
    const int cooldownMilliseconds = 1000; // 1 second cooldown


    #ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
            const ImGuiKey key_first = ImGuiKey_NamedKey_BEGIN;
    #else
        struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } };
        const ImGuiKey key_first = 0;
    #endif

    int keyDown = 0; // used to identify which direction the character is moving
    for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++)
    {
        if (elapsedTime >= cooldownMilliseconds) {
            if (funcs::IsLegacyNativeDupe(key)) continue;

            if(ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_W)) { keyDown = 1; }
            else if(ImGui::IsKeyDown(ImGuiKey_DownArrow) || ImGui::IsKeyDown(ImGuiKey_S)) { keyDown = 2; }
            else if(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) { keyDown = 3; }
            else if(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)) { keyDown = 4; }
            
        }
    }

    ImGui::Begin("Background", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    {
        move.mapMovement(keyDown, background, gridX, gridY);
    }

    ImGui::End();
}

void graphic::makeConfig(){
    // Config window calculation
    ImGui::SetNextWindowSize({(float)width_px / 2, (float)height_px / 2});

    ImGui::SetNextWindowPos({0, (float)height_px/2});

    // Window - Config
    ImGui::Begin("Config", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
    }
    ImGui::End();

}

string graphic::executeCPP(string code){
    // Step 1: Write code to a temporary file
    std::ofstream file("temp.cpp");
    file << code;
    file.close();

    // Step 2: Invoke the compiler and capture output
    std::string compileCommand = "g++ -o temp temp.cpp 2>&1"; // Redirect stderr to stdout
    FILE* pipe = popen(compileCommand.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error invoking compiler command." << std::endl;
        return "Error invoking compiler command.";
    }

    char buffer[128];
    std::string compileOutput = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            compileOutput += buffer;
    }
    pclose(pipe);

    if (!compileOutput.empty()) {  //has error
        remove("temp.cpp");
        return compileOutput;
    }

    FILE* executePipe = popen("./temp 2>&1", "r");
    if (!executePipe) {
        std::cerr << "Error invoking execute command." << std::endl;
        remove("temp.cpp");
        remove("temp");
        return "Error invoking execute command.";
    }

    char executeBuffer[128];
    std::string executeOutput = "";
    while (!feof(executePipe)) {
        if (fgets(executeBuffer, 128, executePipe) != NULL)
            executeOutput += executeBuffer;
    }
    pclose(executePipe);

    // Clean up temporary files
    remove("temp.cpp");
    remove("temp");

    return executeOutput;
}

void graphic::makeLogIn(login& Login, imageHandler& image)
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 7.5f;
    style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    Login.drawLoginScreen(&image);
    if(Login.checkAuth())
    {
        show_display = true;
        show_process = true;
        show_config = true;
        show_charSelector = true;

        style.FrameRounding = 0.f;
        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

        show_login = false;
    }
}

void graphic::makeSettings(login &Login, imageHandler& image, characterManager &character, characterBuilder& charBuild, bool& done)
{
    // todo - change background color
    float windowWidth = 320.f;
    float windowHeight = 620.f; // allow for 50px padding on a 1280x720 window
    float padding = ((float)height_px - windowHeight) / 2.f;

    static bool settingsWindow = false;
    static bool userProfileWindow = false;
    static bool characterWindow = false;
    static bool logOutWindow = false;
    static bool quitWindow = false;

    static bool usr_Username = false;
    static bool usr_Password = false;

    if(resetPauseScreen)
    {
        settingsWindow = false;
        userProfileWindow = false;
        characterWindow = false;
        logOutWindow = false;
        quitWindow = false;

        usr_Username = false;
        usr_Password = false;

        resetPauseScreen = false;
    }

    ImGui::SetNextWindowSize({windowWidth, windowHeight});
    ImGui::SetNextWindowPos({padding, padding});

    ImGui::Begin("Pause", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    {
        ImGui::SetCursorPos(ImVec2(85.f, (620.f / 6.f) * 1.f - 20.f));
        ImGui::PushID(1111);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
        if(ImGui::Button("Settings", ImVec2(150.f, 40.f)))
        {
            settingsWindow = true;
            userProfileWindow = false;
            characterWindow = false;
            logOutWindow = false;
            quitWindow = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SetCursorPos(ImVec2(85.f, (620.f / 6.f) * 2.f - 20.f));
        ImGui::PushID(2222);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
        if(ImGui::Button("User Profile", ImVec2(150.f, 40.f)))
        {
            settingsWindow = false;
            userProfileWindow = true;
            characterWindow = false;
            logOutWindow = false;
            quitWindow = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SetCursorPos(ImVec2(85.f, (620.f / 6.f) * 3.f - 20.f));
        ImGui::PushID(3333);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
        if(ImGui::Button("Change Character", ImVec2(150.f, 40.f)))
        {
            settingsWindow = false;
            userProfileWindow = false;
            characterWindow = true;
            logOutWindow = false;
            quitWindow = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SetCursorPos(ImVec2(85.f, (620.f / 6.f) * 4.f - 20.f));
        ImGui::PushID(4444);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
        if(ImGui::Button("Log Out", ImVec2(150.f, 40.f)))
        {
            settingsWindow = false;
            userProfileWindow = false;
            characterWindow = false;
            logOutWindow = true;
            quitWindow = false;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        ImGui::SetCursorPos(ImVec2(85.f, (620.f / 6.f) * 5.f - 20.f));
        ImGui::PushID(5555);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f / 360.f, 1.0f, 0.76f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 0.384f));
        if(ImGui::Button("Quit", ImVec2(150.f, 40.f)))
        {
            settingsWindow = false;
            userProfileWindow = false;
            characterWindow = false;
            logOutWindow = false;
            quitWindow = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    ImGui::End();


    if(settingsWindow)
    {
        // todo - change to real audio controls from audio class
        static int tempMain = 20;
        static int tempMusic = 20;
        static int tempSoundEffect = 20;

        ImGui::SetNextWindowSize({(float)width_px - windowWidth - (padding * 2.f) - 10.f, 620.f});
        ImGui::SetNextWindowPos({windowWidth + padding + 10.f, padding});

        ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::SetCursorPos(ImVec2(20.f, 40.f));
            ImGui::Text("Change Screen Resolution");

            ImGui::SetCursorPos(ImVec2(30.f, 60.f));
            static int e = 0;
            static int k = 0;
#if defined(__APPLE__)
            ImGui::RadioButton("2480 x 1440", &e, 0);
            ImGui::SameLine();
            ImGui::RadioButton("2560 x 1800", &e, 1);
#else
            ImGui::RadioButton("1280 x 720", &e, 0);
            ImGui::SameLine();
            ImGui::RadioButton("1440 x 900", &e, 1);
            ImGui::SameLine();
            ImGui::RadioButton("1920 x 1080", &e, 2);
#endif
            // Disabled due to issues with SDL and MAC Retina Displays
            /*
            if(k != e)
            {
                switch(e)
                {
                    case 0:
                        width_px = 1280;
                        height_px = 720;
                        break;
                    case 1:
                        width_px = 1440;
                        height_px = 900;
                        break;
                    case 2:
                        width_px = 1920;
                        height_px = 1080;
                        break;
                }
                changeResolution = true;
                k = e;
            }
             */

            ImGui::SetCursorPos(ImVec2(20.f, 100.f));
            ImGui::Text("Volume Controls: ");

            ImGui::SetCursorPos(ImVec2(30.f, 120.f));
            ImGui::SliderInt("Main", &tempMain, 0, 20);
            ImGui::SetCursorPos(ImVec2(30.f, 145.f));
            ImGui::SliderInt("Music", &tempMusic, 0, 20);
            ImGui::SetCursorPos(ImVec2(30.f, 170.f));
            ImGui::SliderInt("Sound Effects", &tempSoundEffect, 0, 20);
            // Change resolution
                // give warning on mac that resolution exceeds display size on anything over 1440x900 or just double the number displayed
        }
        ImGui::End();
    }
    else if(userProfileWindow)
    {
        const float profileWidth = windowWidth + padding + 10.f;
        const float profileHeight = padding;
        ImGui::SetNextWindowSize({(float)width_px - windowWidth - (padding * 2.f) - 10.f, 620.f});
        ImGui::SetNextWindowPos({profileWidth, profileHeight});

        static char createUsername[64] = "";

        static char currentPasswd[64] = "";
        static char createPasswd[64] = "";
        static char confirmPasswd[64] = "";

        ImGui::Begin("User Profile", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImVec2(profileWidth + 20.f, profileHeight + 70.f), ImVec2(profileWidth + 300.f, profileHeight + 230.f), ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), 20.0f);
            draw_list->AddRectFilled(ImVec2(profileWidth + 35.f, profileHeight + 85.f), ImVec2(profileWidth + 109.f, profileHeight + 218.f), ImColor(ImVec4(0.9f, 0.9f, 0.9f, 1.0f)), 20.0f);

            charBuild.drawCharacterAnimation(&image, ImVec2(40.f, 85.f), {ImVec2(0.1f / 192.f, 0.1f/320.f),ImVec2(31.99f/192.f, 64.f/320.f)}, 2.f, character.getMainPlayer()->dynamicIndex);
            // show username
            ImGui::SetCursorPos(ImVec2(124.f, 110.f));
            ImGui::Text("Username: ");
            ImGui::SameLine();
            ImGui::Text(Login._username.c_str());

            ImGui::SetCursorPos(ImVec2(124.f, 130.f));
            ImGui::Text("Date Joined: ");
            ImGui::SameLine();
            ImGui::Text("mm/dd/yyyy"); // todo - get from database

            ImGui::SetCursorPos(ImVec2(124.f, 150.f));
            ImGui::Text("Level: "); // todo - change to whatever gets implemented
            ImGui::SameLine();
            ImGui::Text("0");

            ImGui::SetCursorPos(ImVec2(124.f, 170.f));
            ImGui::Text("Problems Solved: "); // todo - get from database
            ImGui::SameLine();
            ImGui::Text("0");

            // change username

            ImGui::SetCursorPos(ImVec2(360.f, 40.f));
            ImGui::PushID(11);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Change Username", ImVec2(130.f, 20.f)))
            {
                usr_Username = true;
                usr_Password = false;

                createUsername[64] = '\0';
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
            // change password
            ImGui::SameLine();
            ImGui::PushID(22);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Change Password", ImVec2(130.f, 20.f)))
            {
                usr_Username = false;
                usr_Password = true;

                currentPasswd[64] = '\0';
                createPasswd[64] = '\0';
                confirmPasswd[64] = '\0';
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::PushID(33);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Change Character", ImVec2(130.f, 20.f)))
            {
                usr_Username = false;
                usr_Password = false;
                userProfileWindow = false;
                characterWindow = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            if(usr_Username)
            {
                draw_list->AddRectFilled(ImVec2(profileWidth + 350.f, profileHeight + 70.f), ImVec2(profileWidth + 800.f, profileHeight + 200.f), ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), 20.0f);

                ImGui::SetCursorPos(ImVec2(380.f, 80.f));
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
                ImGui::Text("Enter New Username");
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(380.f, 105.f));
                ImGui::PushItemWidth(300);
                ImGui::InputText(" ",createUsername, IM_ARRAYSIZE(createUsername), ImGuiInputTextFlags_None);
                ImGui::PopItemWidth();

                ImGui::SetCursorPos(ImVec2(380.f, 140.f));
                ImGui::PushID(44);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
                if(ImGui::Button("Update Username", ImVec2(130.f, 30.f)))
                {
                    // todo - Connect to login and database to update username
                        // this needs to check to not have overlapping usernames
                        // use stored db id to access user
                        // if successful change on other display, clear input filed, and close this setting window
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();

            }
            else if(usr_Password)
            {
                draw_list->AddRectFilled(ImVec2(profileWidth + 350.f, profileHeight + 70.f), ImVec2(profileWidth + 800.f, profileHeight + 255.f), ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), 20.0f);

                ImGui::SetCursorPos(ImVec2(380.f, 75.f));
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
                ImGui::Text("Enter Current Password");
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(380.f, 95.f));
                ImGui::PushItemWidth(300);
                ImGui::InputText(" ",currentPasswd, IM_ARRAYSIZE(currentPasswd), ImGuiInputTextFlags_None);
                ImGui::PopItemWidth();

                ImGui::SetCursorPos(ImVec2(380.f, 120.f));
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
                ImGui::Text("Enter New Password");
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(380.f, 140.f));
                ImGui::PushItemWidth(300);
                ImGui::InputText("  ",createPasswd, IM_ARRAYSIZE(createPasswd), ImGuiInputTextFlags_None);
                ImGui::PopItemWidth();

                ImGui::SetCursorPos(ImVec2(380.f, 170.f));
                ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
                ImGui::Text("Confirm New Password");
                ImGui::PopStyleColor();

                ImGui::SetCursorPos(ImVec2(380.f, 190.f));
                ImGui::PushItemWidth(300);
                ImGui::InputText("   ",confirmPasswd, IM_ARRAYSIZE(confirmPasswd), ImGuiInputTextFlags_None);
                ImGui::PopItemWidth();

                ImGui::SetCursorPos(ImVec2(380.f, 220.f));
                ImGui::PushID(44);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
                if(ImGui::Button("Update Password", ImVec2(130.f, 30.f)))
                {
                    // todo - Connect to login and database to change password
                    // add function in auth and database to handle this
                    // use stored db id to access user
                    // if successful change display a success message, clear input filed
                }
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }

            // see progress
            // have a child window to list and selectg the problems to see how the user did
            draw_list->AddRectFilled(ImVec2(profileWidth + 20.f, profileHeight + 260.f), ImVec2(profileWidth + 405.f, profileHeight + 600.f), ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), 20.0f);
            ImGui::SetCursorPos(ImVec2(30.f, 270.f));
            ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
            ImGui::Text("Solved Problems");
            ImGui::PopStyleColor();

            draw_list->AddRectFilled(ImVec2(profileWidth + 425.f, profileHeight + 260.f), ImVec2(profileWidth + 830.f, profileHeight + 600.f), ImColor(ImVec4(0.6f, 0.6f, 0.6f, 1.0f)), 20.0f);
            ImGui::SetCursorPos(ImVec2(435.f, 270.f));
            ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.f / 360.f,0.0f,0.0f));
            ImGui::Text("Inventory");
            ImGui::PopStyleColor();
            // possibly see inventory
                // use a child window to view the items
        }
        ImGui::End();
    }
    else if(characterWindow)
    {
        float factor = 4.f;
        const float frameLength = 5.f / 10.f; // In seconds, so  FPS
        static float frameTimer = frameLength;

        ImGui::Begin("Change Character", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            frameTimer -= ImGui::GetIO().DeltaTime;
            ImVec2 characterPos = ImVec2((ImGui::GetContentRegionAvail() - ImVec2((32.f * factor), (64.f * factor))) * 0.25f);
            charBuild.drawPos = ImVec2(characterPos.x * 3.f, characterPos.y * 2.f);
            charBuild.drawCharacterBuilder(&image, frameTimer);

            if (frameTimer <= 0.f)
            {
                frameTimer = 5.f / 10.f;
            }

            ImGui::SetCursorPos(ImVec2(290.f, padding + 380.f));
            ImGui::PushID(8);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Select Character", ImVec2(150.f, 40.f)))
            {
                character.selectMainCharacter(&charBuild);
                show_settings = !show_settings;
                show_display = !show_display;
                show_process = !show_process;
                show_config = !show_config;

                resetPauseScreen = true;

                // todo - change to show message that character changed instead of closing
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
        ImGui::End();
    }
    else if (logOutWindow)
    {
        ImGui::SetNextWindowSize({320.f, 110.f});
        ImGui::SetNextWindowPos({windowWidth + padding + 10.f, padding});
        ImGui::Begin("Log Out", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::SetCursorPos(ImVec2(20.f, 20.f));
            ImGui::Text("Are You Sure You Want to Log Out?");

            ImGui::SetCursorPos(ImVec2( 20.f,50.f));
            ImGui::PushID(77);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Keep Playing", ImVec2(120.f, 40.f)))
            {
                settingsWindow = false;
                userProfileWindow = false;
                characterWindow = false;
                logOutWindow = false;
                quitWindow = false;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::SetCursorPos(ImVec2( 160.f,50.f));
            ImGui::PushID(66);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f / 360.f, 1.0f, 0.76f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 0.384f));
            if(ImGui::Button("Log Out", ImVec2(120.f, 40.f)))
            {
                // todo - login function to clear saved data
                show_display = false;
                show_process = false;
                show_config = false;
                show_charSelector = false;
                show_settings = false;

                characterCreated = false;

                show_login = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
        ImGui::End();
    }
    else if(quitWindow)
    {
        ImGui::SetNextWindowSize({320.f, 110.f});
        ImGui::SetNextWindowPos({windowWidth + padding + 10.f, padding});
        ImGui::Begin("Quit", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        {
            ImGui::SetCursorPos(ImVec2(20.f, 20.f));
            ImGui::Text("Are You Sure You Want to Quit?");

            ImGui::SetCursorPos(ImVec2( 20.f,50.f));
            ImGui::PushID(77);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(219.f / 360.f, 0.289f, 0.475f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(211.f / 360.f, 0.346f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(228.f / 360.f, 0.153f, 0.384f));
            if(ImGui::Button("Keep Playing", ImVec2(120.f, 40.f)))
            {
                settingsWindow = false;
                userProfileWindow = false;
                characterWindow = false;
                logOutWindow = false;
                quitWindow = false;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::SetCursorPos(ImVec2( 160.f,50.f));
            ImGui::PushID(66);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f / 360.f, 1.0f, 0.76f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 360.f, 1.f, 0.384f));
            if(ImGui::Button("Quit", ImVec2(120.f, 40.f)))
            {
                done = true;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
        ImGui::End();
    }

    // todo - remove backgroind color
}