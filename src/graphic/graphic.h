#pragma once

#include "imgui.h"
#include "../imageHandler/imageHandler.h"
#include "../character/characterManager.h"
#include "../character/characterBuilder.h"
#include "../movement/movementHandler.h"
#include "textEditor/TextEditor.h"
#include "../login/login.h"
#include "database/database.h"
#include "../pause/pauseMenu.h"

class graphic {
public:
    int width_px = 1760;
    int height_px = 1024;

    // int width_px = 1320;
    // int height_px = 768;

    bool allowMovement = false;
    bool show_login = true;
    bool show_display = false;
    bool show_codeEditor = false;
    bool show_config = false;
    bool show_charSelector = false;
    bool show_settings = false;
    bool show_blur = false;
    bool show_userProfile = false;

    // bool allowMovement = false;
    // bool show_login = false;
    // bool show_display = true;
    // bool show_codeEditor = false;
    // bool show_config = true;
    // bool show_charSelector = true;
    // bool show_blur = true;
    // bool show_userProfile = true;

    int codeEditorFont = 0;

    bool characterCreated = false;

    string result = "";
    database::questionData qes;

    vector<TextEditor::LanguageDefinition> languages;
    vector<string> codeStarter;
    
    int selectedLanguageIndex = 0;
    int interact = 0;

    string pathMap;
    string intMap;
    string obsMap;
    string overlapMap;

    string world = "", room = "";

    imageHandler background;
    imageHandler overlap;
    imageHandler interactions;

    //graphic functions
    void setup();

    void makeBackground(imageHandler background, vector<vector<int>> grid, double gridX, double gridY, imageHandler& image, characterManager &character, characterBuilder& charBuild, int &lastAction);
    void makeConfig(ImFont* font);
    void makeCodeEditor(TextEditor &editor, const char* fileToEdit, ImFont* font_15, ImFont* font_18, ImFont* font_21);
    void makeCharacter(imageHandler& image, TextEditor& editor, double &gridX, double &gridY, movementHandler& move, int &lastAction, characterManager &character, characterBuilder& charBuild);
    void makeCharacterSelector(imageHandler& image, characterManager &character, characterBuilder& charBuild);
    void makeBackground(imageHandler background, movementHandler move, float &gridX, float &gridY);
    void makeLogIn(login& Login, imageHandler& image, characterManager &character, characterBuilder& charBuild);

    void makeBlur();

    void makeUserProfile(ImFont* fontLarge, ImFont* fontSmall, ImFont* fontLogo, imageHandler& image, characterManager& character, characterBuilder& charBuild);

    void triggerQuestion(int question);
    void loadMapUpdate(movementHandler &move);
    string executeCPP(string code);
    void makeSettings(pauseMenu& Pause, imageHandler& image, characterManager& character, characterBuilder& charBuild, movementHandler& movement, login& Login, bool& done);

private:
    bool resetPauseScreen = false;
    bool reset = false;
    bool changeScreenRes = false;
    pair<int,int> res;

    bool overlapCharacter = false;
    ImVec2 characterPos;

    bool playTutorial = true;
};