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
        ptui::tasUITileMap.setCursor(1, 1);
        ptui::tasUITileMap.printChar('0' + PC::fps_counter / 10);
        ptui::tasUITileMap.printChar('0' + PC::fps_counter % 10);
        
        
        if ((ticks >= 60) && (ticks <= 120))
        {
            ptui::tasUITileMap.drawBox(2, 20, 10, 28);
            
            ptui::tasUITileMap.setCursor(4, 21);
            ptui::tasUITileMap.printString("Attack");
            
            ptui::tasUITileMap.setCursor(4, 22);
            ptui::tasUITileMap.printString("Magick");
            
            ptui::tasUITileMap.setCursor(4, 23);
            ptui::tasUITileMap.printString("Items");
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
                ptui::tasUITileMap.setCursor(11, 21);
                ptui::tasUITileMap.printString("Rat");
                ptui::tasUITileMap.setCursor(11, 22);
                ptui::tasUITileMap.printString("Slime");
                
                if (ticks > 105)
                {
                    ptui::tasUITileMap.set(10, 21, ' ');
                    ptui::tasUITileMap.set(10, 22, '>');
                }
                else
                    ptui::tasUITileMap.set(10, 21, '>');
            }
        }
        else
        {
            ptui::tasUITileMap.drawBox(-1, 20, 37, 30);
            ptui::tasUITileMap.drawGauge(29, 35, 21, ticks, 59);
            ptui::tasUITileMap.setCursor(22, 21);
            ptui::tasUITileMap.printString("Mareve");
        }
        if (ticks > 120)
        {
            ptui::tasUITileMap.drawGauge(29, 35, 21, 0, 59);
        }
        bool ready = (ticks > 60) && (ticks < 120);
        
        ptui::tasUITileMap.drawCheckbox(21, 21, ready);
        
        
        if (ticks > 16)
        {
            ptui::tasUITileMap.drawBox(13, 2, 22, 6);
            
            ptui::tasUITileMap.setCursorBoundingBox(14, 3, 21, 6);
            ptui::tasUITileMap.setCursor(14, 3);
            
            ptui::tasUITileMap.printString("i\tii\tiii\tiiii\t\n----");
            
            ptui::tasUITileMap.resetCursorBoundingBox();
        }
        else
            ptui::tasUITileMap.clear(13, 2, 22, 6);
        
        
        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
        ticks++;
        if (ticks == 180)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
        }
    }
    
    return 0;
}
