#include <Pokitto.h>
#include <miloslav.h>
#include <Tilemap.hpp>
#include <SDFileSystem.h>
#include "sprites/Mareve.h"
#include "maps.h"
#include "ptui/TASTerminalTileMap.hpp"


int main()
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
    PC::begin();
    PD::loadRGBPalette(miloslav);
    
    auto mareveOriginX = Mareve[0] / 2;
    auto mareveOriginY = Mareve[1] / 2;
    Tilemap tilemap;
    tilemap.set(gardenPath[0], gardenPath[1], gardenPath+2);
    
    for (int i = 0; i < sizeof(tiles)/(POK_TILE_W*POK_TILE_H); i++)
        tilemap.setTile(i, POK_TILE_W, POK_TILE_H, tiles+i*POK_TILE_W*POK_TILE_H);

    int characterX = 32;
    int characterY = 32;
    int speed=1;
    
    // Drawing the UI.
    {
        int shift = 0;
        const auto tile = 128;
        
        while ((shift < ptui::TASUITileMap::columns) && (shift < ptui::TASUITileMap::rows))
        {
            for (int i = shift; i < ptui::TASUITileMap::columns - shift; i++)
            {
                ptui::tasUITileMap.setTile(i, shift, tile);
                ptui::tasUITileMap.setTile(i, ptui::TASUITileMap::rows - 1 - shift, tile);
            }
            for (int j = shift; j < ptui::TASUITileMap::rows - shift; j++)
            {
                ptui::tasUITileMap.setTile(shift, j, tile);
                ptui::tasUITileMap.setTile(ptui::TASUITileMap::columns - 1 - shift, j, tile);
            }
            shift += 2;
        }
    }
    while (PC::isRunning())
    {
        if (!PC::update()) 
            continue;
        
        {
            int oldX = characterX;
            int oldY = characterY;
    
            speed = PB::bBtn() ? 4 : 1;
            while (speed--)
            {
                {
                    if (PB::leftBtn()) characterX--;
                    if (PB::rightBtn()) characterX++;
                    
                    int tileX = characterX / PROJ_TILE_W;
                    int tileY = characterY / PROJ_TILE_H;
                    auto tile = gardenPathEnum(tileX, tileY);
                    if (tile == Collide)
                        characterX = oldX;
                }
                {
                    if (PB::downBtn()) characterY++;
                    if (PB::upBtn()) characterY--;
                    
                    int tileX = characterX / PROJ_TILE_W;
                    int tileY = characterY / PROJ_TILE_H;
                    auto tile = gardenPathEnum(tileX, tileY);
                    if (tile == Collide)
                        characterY = oldY;
                }
            }
        }

        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
    }
    
    return 0;
}
