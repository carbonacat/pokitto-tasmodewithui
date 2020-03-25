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
    ptui::tasUITileMap.setOffset(-1, -4);
    
    // Drawing the UI.
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
        
        
        if ((ticks >= 60) && (ticks <= 120))
        {
            ptui::tasUITileMap.drawBox(2, 20, 10, 28);
            ptui::tasUITileMap.set(4, 21, 'A');
            ptui::tasUITileMap.set(5, 21, 't');
            ptui::tasUITileMap.set(6, 21, 't');
            ptui::tasUITileMap.set(7, 21, 'a');
            ptui::tasUITileMap.set(8, 21, 'c');
            ptui::tasUITileMap.set(9, 21, 'k');
            ptui::tasUITileMap.set(4, 22, 'M');
            ptui::tasUITileMap.set(5, 22, 'a');
            ptui::tasUITileMap.set(6, 22, 'g');
            ptui::tasUITileMap.set(7, 22, 'i');
            ptui::tasUITileMap.set(8, 22, 'c');
            ptui::tasUITileMap.set(9, 22, 'k');
            ptui::tasUITileMap.set(4, 23, 'I');
            ptui::tasUITileMap.set(5, 23, 't');
            ptui::tasUITileMap.set(6, 23, 'e');
            ptui::tasUITileMap.set(7, 23, 'm');
            ptui::tasUITileMap.set(8, 23, 's');
            if (ticks < 70)
            {
                ptui::tasUITileMap.set(3, 21, '>');
            }
            else if (ticks < 80)
            {
                ptui::tasUITileMap.set(3, 21, ' ');
                ptui::tasUITileMap.set(3, 22, '>');
            }
            else if (ticks < 90)
            {
                ptui::tasUITileMap.set(3, 21, '>');
                ptui::tasUITileMap.set(3, 22, ' ');
            }
            else if (ticks > 90)
            {
                ptui::tasUITileMap.drawBox(9, 20, 16, 23);
                ptui::tasUITileMap.set(10, 21, '>');
                ptui::tasUITileMap.set(11, 21, 'R');
                ptui::tasUITileMap.set(12, 21, 'a');
                ptui::tasUITileMap.set(13, 21, 't');
                ptui::tasUITileMap.set(11, 22, 'S');
                ptui::tasUITileMap.set(12, 22, 'l');
                ptui::tasUITileMap.set(13, 22, 'i');
                ptui::tasUITileMap.set(14, 22, 'm');
                ptui::tasUITileMap.set(15, 22, 'e');
                if (ticks > 105)
                {
                    ptui::tasUITileMap.set(10, 21, ' ');
                    ptui::tasUITileMap.set(10, 22, '>');
                }
            }
        }
        else
        {
            ptui::tasUITileMap.drawBox(-1, 20, 37, 30);
            ptui::tasUITileMap.drawGauge(29, 35, 21, ticks, 59);
            ptui::tasUITileMap.set(22, 21, 'M');
            ptui::tasUITileMap.set(23, 21, 'a');
            ptui::tasUITileMap.set(24, 21, 'r');
            ptui::tasUITileMap.set(25, 21, 'e');
            ptui::tasUITileMap.set(26, 21, 'v');
            ptui::tasUITileMap.set(27, 21, 'e');
        }
        if (ticks > 120)
        {
            ptui::tasUITileMap.drawGauge(29, 35, 21, 0, 59);
        }
        bool ready = (ticks > 60) && (ticks < 120);
        
        ptui::tasUITileMap.drawCheckbox(21, 21, ready);
        
        
        if (ticks > 120)
        {
            ptui::tasUITileMap.drawBox(13, 2, 22, 4);
            ptui::tasUITileMap.set(14, 3, 'C');
            ptui::tasUITileMap.set(15, 3, 'r');
            ptui::tasUITileMap.set(16, 3, 'i');
            ptui::tasUITileMap.set(17, 3, 't');
            ptui::tasUITileMap.set(18, 3, 'i');
            ptui::tasUITileMap.set(19, 3, 'c');
            ptui::tasUITileMap.set(20, 3, 'a');
            ptui::tasUITileMap.set(21, 3, 'l');
        }
        else
            ptui::tasUITileMap.clear(13, 2, 22, 4);
        
        
        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
        ticks++;
        if (ticks == 180)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            // ptui::tasUITileMap.shift(1, 1, ptui::TASUITileMap::columns - 2, ptui::TASUITileMap::rows - 2,
            //                          -2 + rand() % 5, -2 + rand() % 5);
        }
    }
    
    return 0;
}
