#include <Pokitto.h>
#include <miloslav.h>
#include <Tilemap.hpp>
#include <SDFileSystem.h>
#include "sprites/Mareve.h"
#include "tilesets/TerminalTileSet.h"
#include "maps.h"
#include "ptui/TASTerminalTileMap.hpp"

int transition = 0;

void GameFiller(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    int yMin = 88 - transition * 2;
    int yMax = 88 + transition * 2;
    bool transitioning = (yMin > (int)y) || ((int)y > yMax);
    
    TAS::BGTileFiller(line, y, skip);
    TAS::SpriteFiller(line, y, skip || transitioning);
    ptui::TerminalTMFiller(line, y, skip);
    if (transitioning)
        std::fill(line, line + PROJ_LCDWIDTH, 0);
}

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
        ptui::tasUITileMap.clear(1, 1, 1, 3);
        ptui::tasUITileMap.printInteger(PC::fps_counter);
        
        
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
            ptui::tasUITileMap.clear(2, 20, 20, 20);
            
            ptui::tasUITileMap.drawBox(-1, 21, 37, 30);
            
            ptui::tasUITileMap.setCursor(17, 22);
            ptui::tasUITileMap.printString("Mareve");
            
            ptui::tasUITileMap.setCursor(26, 22);
            ptui::tasUITileMap.printInteger(133, 4);
            
            ptui::tasUITileMap.drawGauge(31, 35, 22, ticks, 59);
            
            ptui::tasUITileMap.setCursor(17, 24);
            ptui::tasUITileMap.printString("Delirio");
            
            ptui::tasUITileMap.setCursor(26, 24);
            ptui::tasUITileMap.printInteger(6894, 4);
            
            ptui::tasUITileMap.drawGauge(31, 35, 24, ticks, 59);
            
            ptui::tasUITileMap.setCursor(17, 26);
            ptui::tasUITileMap.printString("Matti");
            
            ptui::tasUITileMap.setCursor(26, 26);
            ptui::tasUITileMap.printInteger(9999, 4);
            
            ptui::tasUITileMap.drawGauge(31, 35, 26, ticks, 59);
            
            ptui::tasUITileMap.setCursor(17, 28);
            ptui::tasUITileMap.printString("???");
            
            ptui::tasUITileMap.setCursor(26, 28);
            ptui::tasUITileMap.printInteger(543, 4);
            
            ptui::tasUITileMap.drawGauge(31, 35, 28, ticks, 59);
        }
        if (ticks > 120)
        {
            ptui::tasUITileMap.drawGauge(31, 35, 22, 0, 59);
            ptui::tasUITileMap.drawGauge(31, 35, 24, 0, 59);
            ptui::tasUITileMap.drawGauge(31, 35, 26, 0, 59);
            ptui::tasUITileMap.drawGauge(31, 35, 28, 0, 59);
        }

        if (ticks > 16)
        {
            ptui::tasUITileMap.drawBox(2, 2, 35, 6);
            
            ptui::tasUITileMap.setCursorBoundingBox(3, 3, 34, 5);
            ptui::tasUITileMap.setCursor(3, 3);
            
            //ptui::tasUITileMap.printString("Life... dreams... hope...\n    \n\nWhere do they come from?\nAnd where do they go?\n     \n\nSuch meaningless things...\nI'll destroy them all!    ", (ticks - 16) / 2);
            ptui::tasUITileMap.printText("Interesting,Interesting Interesting\tInteresting, Interesting-Interesting, Interesting InterestingInteresting, Interesting, InterestingInterestingInteresting", (ticks - 16) / 2);
            
            ptui::tasUITileMap.resetCursorBoundingBox();
        }
        else
            ptui::tasUITileMap.clear(2, 2, 35, 6);
        
        
        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
        ticks++;
        if (ticks == 350)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
        }
        transition++;
    }
    
    return 0;
}
