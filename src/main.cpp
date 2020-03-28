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

void TerminalTMFillerCLUT(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBufferCLUT(line, y, skip);
}
void TerminalTMFillerBase(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBufferBase(line, y, skip);
}

int battleMockup()
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
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
    ptui::tasUITileMap.setTilesetImage(TerminalTileSet);
    ptui::tasUITileMap.setOffset(-1, -4);
    ptui::tasUITileMap.clear();
    
    PD::lineFillers[0] = TAS::BGTileFiller;
    PD::lineFillers[1] = TAS::SpriteFiller;
    
    // Drawing the UI.
    while (PC::isRunning() && !PB::cBtn())
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
        ptui::tasUITileMap.fillRect(1, 1, 3, 1);
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
            ptui::tasUITileMap.fillRect(2, 20, 20, 20);
            
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
            
            ptui::tasUITileMap.fillRectColorOffset(31, 22, 35, 28, ticks >= 59 ? 8 : 0);
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
            ptui::tasUITileMap.fillRect(2, 2, 35, 6);
    
        {
            ptui::tasUITileMap.drawGauge(1, 35, 8, ticks, 350);
            ptui::tasUITileMap.fillRectColorOffset(1, 8, 35, 9, (ticks / 16 % 2) ? 40 : 0);
            ptui::tasUITileMap.setCursorBoundingBox(1, 9, 35, 9);
            ptui::tasUITileMap.setCursor(1, 9);
            ptui::tasUITileMap.printText("This is an interesting text!");
            ptui::tasUITileMap.resetCursorBoundingBox();
        }
        
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

int testPerfsFull(bool cropped)
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
    int ticks = 0;
    
    // Configuration.
    ptui::tasUITileMap.setTilesetImage(TerminalTileSet);
    ptui::tasUITileMap.clear(32);
    ptui::tasUITileMap.setOffset(-1, cropped ? 135: 0);
    
    ptui::tasUITileMap.drawBox(1, 1, 35, 28);
    ptui::tasUITileMap.setCursor(2, 2);
    ptui::tasUITileMap.setCursorBoundingBox(2, 2, 34, 27);
    ptui::tasUITileMap.fillRectColorOffset(2, 2, 6, 2, 8);
    ptui::tasUITileMap.printText("Hello my good chap! Are we ready for the Punk Jam yet?!");
    ptui::tasUITileMap.resetCursorBoundingBox();
    
    ptui::tasUITileMap.drawGauge(2, 6, 4, 3, 6);
    ptui::tasUITileMap.fillRectColorOffset(2, 4, 6, 4, 8);
    
    
    ptui::tasUITileMap.drawGauge(12, 16, 4, 3, 6);
    ptui::tasUITileMap.fillRectColorOffset(12, 4, 16, 4, 16);
    
    ptui::tasUITileMap.drawGauge(22, 26, 4, 3, 6);
    ptui::tasUITileMap.fillRectColorOffset(22, 4, 26, 4, 24);
    
    ptui::tasUITileMap.drawGauge(22, 29, 6, 6, 6);
    ptui::tasUITileMap.fillRectColorOffset(22, 6, 24, 6, 8);
    ptui::tasUITileMap.fillRectColorOffset(25, 6, 26, 6, 32);
    ptui::tasUITileMap.fillRectColorOffset(27, 6, 29, 6, 16);
    
    PD::lineFillers[0] = TAS::NOPFiller;
    PD::lineFillers[1] = TAS::NOPFiller;
    
    // Drawing the UI.
    while (PC::isRunning() && !PB::cBtn())
    {
        if (!PC::update()) 
            continue;
        
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
        
        ticks++;
        if (ticks == 60)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            ptui::tasUITileMap.setCursor(2, 5);
            ptui::tasUITileMap.fillRect(2, 5, 3, 5);
            ptui::tasUITileMap.printInteger(PC::fps_counter);
        }
        ptui::tasUITileMap.mapColor(0+5, ticks);
        ptui::tasUITileMap.mapColor(0+6, ticks + 1);
    }
    
    return 0;
}

int testPerfsStairs()
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
    int ticks = 0;
    
    // Configuration.
    ptui::tasUITileMap.setTilesetImage(TerminalTileSet);
    ptui::tasUITileMap.clear();
    ptui::tasUITileMap.setOffset(0, 0);
    
    PD::lineFillers[0] = TAS::NOPFiller;
    PD::lineFillers[1] = TAS::NOPFiller;
    
    // Drawing the UI.
    while (PC::isRunning() && !PB::cBtn())
    {
        if (!PC::update()) 
            continue;
        
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
        
        ticks++;
        if (ticks == 60)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            ptui::tasUITileMap.clear();
            for (int i = 0; i < 30; i++)
            {
                ptui::tasUITileMap.setCursor(i, i);
                ptui::tasUITileMap.fillRect(i, i, i+2, i, 32);
                ptui::tasUITileMap.printInteger(PC::fps_counter);
            }
        }
        ptui::tasUITileMap.mapColor(0+5, ticks);
        ptui::tasUITileMap.mapColor(0+6, ticks + 1);
    }
    
    return 0;
}

void resetUIColors() noexcept
{
    // Configuring UI's Colors.
    ptui::tasUITileMap.resetCLUT();
    // Makes the first 8 subpalettes the same.
    for (int i = 0; i < 8; i++)
    {
        for (int p = 0; p < 64; p += 8)
            ptui::tasUITileMap.mapColor(p + i, i);
    }
    // Remaps the light UI colors to red for subpalette 8.
    ptui::tasUITileMap.mapColor(8+5, 88+5);
    ptui::tasUITileMap.mapColor(8+6, 88+6);
    // Remaps the light UI colors to blue for subpalette 16.
    ptui::tasUITileMap.mapColor(16+5, 136+5);
    ptui::tasUITileMap.mapColor(16+6, 136+6);
    // Remaps the light UI colors to green for subpalette 24.
    ptui::tasUITileMap.mapColor(24+5, 112+5);
    ptui::tasUITileMap.mapColor(24+6, 112+6);
    // Transparent background for subpalette 40.
    ptui::tasUITileMap.mapColor(40+1, 0);
}

int intermission(const char* nextScene)
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
    int ticks = 0;
    
    // Configuration.
    ptui::tasUITileMap.setTilesetImage(TerminalTileSet);
    ptui::tasUITileMap.clear(32);
    ptui::tasUITileMap.setOffset(0, 0);
    resetUIColors();
    
    PD::lineFillers[0] = TAS::NOPFiller;
    PD::lineFillers[1] = TAS::NOPFiller;
    
    // Drawing the UI.
    ptui::tasUITileMap.fillColorOffset(8);
    while (PC::isRunning() && PB::cBtn())
    {
        if (!PC::update()) 
            continue;
        
        if (PB::rightBtn()) PD::lineFillers[2] = TerminalTMFillerBase;
        if (PB::upBtn()) PD::lineFillers[2] = TerminalTMFillerCLUT;
        
        ptui::tasUITileMap.drawBox(1, 1, 30, 3);
        ptui::tasUITileMap.setCursor(2, 2);
        ptui::tasUITileMap.printString("Next:");
        ptui::tasUITileMap.printString(nextScene);
        
        
        ptui::tasUITileMap.drawBox(1, 5, 36, 7);
        ptui::tasUITileMap.setCursor(2, 6);
        ptui::tasUITileMap.printString("UI rend:");
        if (PD::lineFillers[2] == TerminalTMFillerBase)
            ptui::tasUITileMap.printString("TerminalTMFillerBase");
        if (PD::lineFillers[2] == TerminalTMFillerCLUT)
            ptui::tasUITileMap.printString("TerminalTMFillerCLUT");

        ticks++;
        if (ticks == 60)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            ptui::tasUITileMap.drawBox(30, 1, 35, 3);
            ptui::tasUITileMap.setCursor(32, 2);
            ptui::tasUITileMap.printInteger(PC::fps_counter);
        }
    }
    
    return 0;
}

int main() noexcept
{
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    
    PC::begin();
    PD::loadRGBPalette(miloslav);
    PD::lineFillers[2] = TerminalTMFillerBase;
    
    while (PC::isRunning())
    {
        intermission("Test - Perfs Full");
        testPerfsFull(false);
        
        intermission("Test - Perfs Cropped");
        testPerfsFull(true);
        
        intermission("Test - Perfs Stairs");
        testPerfsStairs();
        
        intermission("Battle Mockup");
        battleMockup();
    }
    return 0;
}
