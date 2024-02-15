#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#if defined(__APPLE__)
#include <iostream>
#include <vector>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::pair;
#else
#include <bits/stdc++.h>
#endif

#include "../imageHandler/imageHandler.h"
#include "characterBuilder.h"

namespace ImGui { extern ImGuiKeyData* GetKeyData(ImGuiKey key); }

#include "../imageHandler/imageHandler.h"
#include "../imageHandler/imagePath.h"
#include "characterManager.h"

characterManager::characterManager() {}

void characterManager::createCharacter(string name, bool npc, bool fullMovement, imageHandler* imgHandler)
{
    character* newChar = new character(name, fullMovement, 32.f, 64.f); // create the character

    if(!npc)
        playerCharacters.emplace(name, newChar);
    else
        npcCharacters.emplace(name, newChar);
}

character* characterManager::getPlayerCharacter(string name)
{
    character* find = playerCharacters.find(name)->second;
    if(find == nullptr)
    { //todo - delete print statement
        std::cout << "FAIL TO FIND BOB" << std::endl;
    }
    return find;
}

character* characterManager::getNpcCharacter(string name)
{
    character* find = npcCharacters.find(name)->second;
    return find;
}

character *characterManager::getMainPlayer()
{
    return mainPlayer;
}

void characterManager::setMainPlayer(std::string name)
{
    mainPlayer = playerCharacters.find(name)->second;
}

void characterManager::moveMainCharacter(imageHandler* imgHandler, characterBuilder* charBuild,float frameTimer)
{
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
            const ImGuiKey key_first = ImGuiKey_NamedKey_BEGIN;
#else
    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } };
    // Hide Native<>ImGuiKey duplicates when both exists in the array
    const ImGuiKey key_first = 0;
    //ImGui::Text("Legacy raw:");       for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (io.KeysDown[key]) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
#endif

    int keyDown = 0; // used to identify which direction the character is moving
    for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++)
    {
        if (funcs::IsLegacyNativeDupe(key)) continue;
        if(ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_W)) { keyDown = 1; }
        else if(ImGui::IsKeyDown(ImGuiKey_DownArrow) || ImGui::IsKeyDown(ImGuiKey_S)) { keyDown = 2; }
        else if(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) { keyDown = 3; }
        else if(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)) { keyDown = 4; }
        else { keyDown = 0; }
    }

    float factor = 1.f;
    //Draw Order: Body -> Eyes -> Outfit -> Hair -> Accessories
    switch(keyDown)
    {
        case 1:
            charBuild->drawCharacterAnimation(imgHandler,drawPos,cordsWalkUp.at(frameCount_6),factor,mainPlayer->dynamicIndex);
            break;
        case 2:
            charBuild->drawCharacterAnimation(imgHandler,drawPos,cordsWalkDown.at(frameCount_6),factor,mainPlayer->dynamicIndex);
            break;
        case 3:
            charBuild->drawCharacterAnimation(imgHandler,drawPos,cordsWalkRight.at(frameCount_6),factor,mainPlayer->dynamicIndex);
            break;
        case 4:
            charBuild->drawCharacterAnimation(imgHandler,drawPos,cordsWalkLeft.at(frameCount_6),factor,mainPlayer->dynamicIndex);
            break;
        default:
            charBuild->drawCharacterAnimation(imgHandler,drawPos,cordsIdle.at(frameCount_6),factor,mainPlayer->dynamicIndex);
            break;
    }

    if (frameTimer <= 0.f)
    {
        frameCount_4 ++;
        frameCount_6 ++;
        if (frameCount_4 % 4 == 0) frameCount_4=0;
        if (frameCount_6 % 6 == 0) frameCount_6=0;
    }
}

void characterManager::selectMainCharacter(characterBuilder* charBuild)
{
    int* temp = charBuild->setAsMainCharacter();

    for(int i = 0; i < 8; i++)
    {
        mainPlayer->dynamicIndex[i] = temp[i];
    }
}