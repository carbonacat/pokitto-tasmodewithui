#include <Pokitto.h>
#include <miloslav.h>
#include <Tilemap.hpp>
#include <SDFileSystem.h>
#include "sprites/Mareve.h"
#include "tilesets/TerminalTileSet.h"
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
    int ticks = 0;
    
    // Configuration.
    ptui::tasUITileMap.setTileset(TerminalTileSet);
    
    // Drawing the UI.
    {
        const auto tile = 24;
    
        const auto boxX = 0;
        const auto boxY = 0;
        const auto boxXLast = ptui::TASUITileMap::columns - 1;
        const auto boxYLast = ptui::TASUITileMap::rows - 1;
    
        ptui::tasUITileMap.set(boxX, boxY, 24);
        ptui::tasUITileMap.set(boxX, boxYLast, 30);
        ptui::tasUITileMap.set(boxXLast, boxY, 26);
        ptui::tasUITileMap.set(boxXLast, boxYLast, 28);
        for (int i = boxX + 1; i <= boxXLast - 1; i++)
        {
            ptui::tasUITileMap.set(i, boxY, 25);
            ptui::tasUITileMap.set(i, boxYLast, 29);
        }
        for (int j = boxY + 1; j <= boxYLast - 1; j++)
        {
            ptui::tasUITileMap.set(boxX, j, 31);
            ptui::tasUITileMap.set(boxXLast, j, 27);
        }
        for (int i = boxX + 1; i <= boxXLast - 1; i++)
            for (int j = boxY + 1; j <= boxYLast - 1; j++)
            {
                ptui::tasUITileMap.set(i, j, 32);
            }
        ptui::tasUITileMap.set(boxX + 1, boxY + 1, '1');
        ptui::tasUITileMap.set(boxXLast - 1, boxY + 1, '1');
        ptui::tasUITileMap.set(boxX + 1, boxYLast - 1, '1');
        ptui::tasUITileMap.set(boxXLast - 1, boxYLast - 1, '1');
        
        
        ptui::tasUITileMap.set((boxX + boxXLast)/2 - 1, (boxY + boxYLast)/2, 'O');
        ptui::tasUITileMap.set((boxX + boxXLast)/2, (boxY + boxYLast)/2, 'N');
        ptui::tasUITileMap.set((boxX + boxXLast)/2 + 1, (boxY + boxYLast)/2, 'E');
    }
    while (PC::isRunning())
    {
        if (!PC::update()) 
            continue;
        
        {
            int oldX = characterX;
            int oldY = characterY;
    
            if (PB::aBtn())
            {
                auto offsetX = ptui::tasUITileMap.offsetX();
                auto offsetY = ptui::tasUITileMap.offsetY();
        
                if (PB::leftBtn()) offsetX--;
                if (PB::rightBtn()) offsetX++;
                if (PB::downBtn()) offsetY++;
                if (PB::upBtn()) offsetY--;
                ptui::tasUITileMap.setOffset(offsetX, offsetY);
            }
            else
            {
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
        }
        ptui::tasUITileMap.set(1, 1, '0' + PC::fps_counter / 10);
        ptui::tasUITileMap.set(2, 1, '0' + PC::fps_counter % 10);
        
        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
        ticks++;
        if (ticks == 15)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            ptui::tasUITileMap.shift(-2 + rand() % 5, -2 + rand() % 5);
        }
    }
    
    return 0;
}
