//
// Created by Matq on 19/03/2023.
//

#include "TextManager.h"

TextManager TextHandler;

TextManager::TextManager() {

}

TextManager::~TextManager() {
    for (const auto& Pair : sFonts)
        TTF_CloseFont(Pair.second);  // problem
}

TTF_Font* TextManager::LoadFont(const std::string& key, const char* filepath, int ptsize) {
    TTF_Font* NewFont = TTF_OpenFont(filepath, ptsize);
    if (!NewFont)
        std::printf("TTF: There was an error while loading font '%s' %s", key.c_str(), TTF_GetError());

    std::pair<std::string, TTF_Font*> NewPair = { key, NewFont };
    sFonts.push_back(NewPair);
    return NewFont;
}
