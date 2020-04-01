#include <Pokitto.h>
#include <miloslav.h>
#include <Tilemap.hpp>
#include <SDFileSystem.h>
#include "sprites/Mareve.h"
#include "tilesets/TerminalTileSet.h"
#include "maps.h"
#include "ptui/TASTerminalTileMap.hpp"

int transition = 0;
bool renderTransparency = true;
bool renderCLUT = true;
bool renderColorOffset = true;

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

void TerminalTMFiller000(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<false, false, false>(line, y, skip);
}
void TerminalTMFiller001(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<false, false, true>(line, y, skip);
}
void TerminalTMFiller010(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<false, true, false>(line, y, skip);
}
void TerminalTMFiller011(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<false, true, true>(line, y, skip);
}
void TerminalTMFiller100(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<true, false, false>(line, y, skip);
}
void TerminalTMFiller101(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<true, false, true>(line, y, skip);
}
void TerminalTMFiller110(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<true, true, false>(line, y, skip);
}
void TerminalTMFiller111(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
{
    ptui::tasUITileMap.renderIntoLineBuffer<true, true, true>(line, y, skip);
}

TAS::LineFiller availableLineFillers[8]
{
    TerminalTMFiller000,
    TerminalTMFiller001,
    TerminalTMFiller010,
    TerminalTMFiller011,
    TerminalTMFiller100,
    TerminalTMFiller101,
    TerminalTMFiller110,
    TerminalTMFiller111,
};

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
    ptui::tasUITileMap.setCursorDelta(0);
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
        ptui::tasUITileMap.fillRectTiles(1, 1, 3, 1, 0);
        ptui::tasUITileMap.printInteger(PC::fps_counter);
        
        
        if ((ticks >= 60) && (ticks <= 120))
        {
            bool attackIsSelected = (ticks < 70) || (ticks >= 80);
            bool magickIsSelected = !attackIsSelected;
            bool itemsIsSelected = false;
                
            ptui::tasUITileMap.drawBox(2, 20, 10, 28);
            
            ptui::tasUITileMap.setCursor(3, 21);
            ptui::tasUITileMap.setCursorDelta(attackIsSelected ? 8 : 0);
            ptui::tasUITileMap.printChar(attackIsSelected ? '>' : ' ');
            ptui::tasUITileMap.printString("Attack");
            
            ptui::tasUITileMap.setCursor(3, 22);
            ptui::tasUITileMap.setCursorDelta(magickIsSelected ? 8 : 0);
            ptui::tasUITileMap.printChar(magickIsSelected ? '>' : ' ');
            ptui::tasUITileMap.printString("Magick");
            
            ptui::tasUITileMap.setCursor(3, 23);
            ptui::tasUITileMap.setCursorDelta(itemsIsSelected ? 8 : 0);
            ptui::tasUITileMap.printChar(itemsIsSelected ? '>' : ' ');
            ptui::tasUITileMap.printString("Items");
            if (ticks < 70)
            {
                ptui::tasUITileMap.setTileAndDelta(3, 21, '>', 8);
            }
            else if (ticks < 80)
            {
                ptui::tasUITileMap.setTileAndDelta(3, 21, ' ', 0);
                ptui::tasUITileMap.setTileAndDelta(3, 22, '>', 8);
            }
            else if (ticks < 90)
            {
                ptui::tasUITileMap.setTileAndDelta(3, 21, '>', 8);
                ptui::tasUITileMap.setTileAndDelta(3, 22, ' ', 0);
            }
            else
            {
                bool ratIsSelected = (ticks < 105);
                bool slimeIsSelected = !ratIsSelected;
                
                ptui::tasUITileMap.fillRectDeltas(9, 20, 16, 23, 0);
                ptui::tasUITileMap.drawBox(9, 20, 16, 23);
                
                ptui::tasUITileMap.setCursorDelta(ratIsSelected ? 8 : 0);
                ptui::tasUITileMap.setCursor(10, 21);
                ptui::tasUITileMap.printChar(ratIsSelected ? '>' : ' ');
                ptui::tasUITileMap.printString("Rat");
                
                ptui::tasUITileMap.setCursorDelta(slimeIsSelected ? 8 : 0);
                ptui::tasUITileMap.setCursor(10, 22);
                ptui::tasUITileMap.printChar(slimeIsSelected ? '>' : ' ');
                ptui::tasUITileMap.printString("Slime");
                
                ptui::tasUITileMap.setCursorDelta(0);
            }
        }
        else
        {
            ptui::tasUITileMap.fillRectTilesAndDeltas(-1, 20, 37, 30, 0, 0);
            
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
            
            ptui::tasUITileMap.fillRectDeltas(31, 22, 35, 28, ticks >= 59 ? 8 : 0);
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
            
            ptui::tasUITileMap.setCursorDelta(16);
            ptui::tasUITileMap.printString("Life... dreams... hope...\n    \n\nWhere do they come from?\nAnd where do they go?\n     \n\nSuch meaningless things...\nI'll destroy them all!    ", (ticks - 16) / 2);
            ptui::tasUITileMap.setCursorDelta(0);

            ptui::tasUITileMap.resetCursorBoundingBox();
        }
        else
            ptui::tasUITileMap.fillRectTiles(2, 2, 35, 6, 0);
    
        {
            ptui::tasUITileMap.drawGauge(1, 35, 8, ticks, 350);
            ptui::tasUITileMap.fillRectDeltas(1, 8, 35, 9, (ticks / 16 % 2) ? 40 : 0);
            ptui::tasUITileMap.setCursorBoundingBox(1, 9, 35, 9);
            ptui::tasUITileMap.setCursor(1, 9);
            ptui::tasUITileMap.setCursorDelta((ticks / 16 % 2) ? 40 : 0);
            ptui::tasUITileMap.printText("This is an interesting text!");
            ptui::tasUITileMap.resetCursorBoundingBox();
            ptui::tasUITileMap.setCursorDelta(0);
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
    ptui::tasUITileMap.setCursorDelta(0);
    
    ptui::tasUITileMap.drawBox(1, 1, 35, 28);
    ptui::tasUITileMap.setCursor(2, 2);
    ptui::tasUITileMap.setCursorBoundingBox(2, 2, 34, 27);
    ptui::tasUITileMap.fillRectDeltas(2, 2, 6, 2, 8);
    ptui::tasUITileMap.printText("Hello my good chap! Are we ready for the Punk Jam yet?!");
    ptui::tasUITileMap.resetCursorBoundingBox();
    
    ptui::tasUITileMap.drawGauge(2, 6, 4, 3, 6);
    ptui::tasUITileMap.fillRectDeltas(2, 4, 6, 4, 8);
    
    
    ptui::tasUITileMap.drawGauge(12, 16, 4, 3, 6);
    ptui::tasUITileMap.fillRectDeltas(12, 4, 16, 4, 16);
    
    ptui::tasUITileMap.drawGauge(22, 26, 4, 3, 6);
    ptui::tasUITileMap.fillRectDeltas(22, 4, 26, 4, 24);
    
    ptui::tasUITileMap.drawGauge(22, 29, 6, 6, 6);
    ptui::tasUITileMap.fillRectDeltas(22, 6, 24, 6, 8);
    ptui::tasUITileMap.fillRectDeltas(25, 6, 26, 6, 32);
    ptui::tasUITileMap.fillRectDeltas(27, 6, 29, 6, 16);
    
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
            ptui::tasUITileMap.fillRectTiles(2, 5, 3, 5, 0);
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
    ptui::tasUITileMap.setCursorDelta(0);
    
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
                ptui::tasUITileMap.fillRectTiles(i, i, i+2, i, 32);
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
    ptui::tasUITileMap.clear(32, 8);
    ptui::tasUITileMap.setOffset(0, 0);
    resetUIColors();
    
    PD::lineFillers[0] = TAS::NOPFiller;
    PD::lineFillers[1] = TAS::NOPFiller;
    
    // Drawing the UI.
    ptui::tasUITileMap.setCursorDelta(0);
    while (PC::isRunning() && PB::cBtn())
    {
        if (!PC::update()) 
            continue;
        
        if (PB::leftBtn()) renderTransparency = false;
        if (PB::rightBtn()) renderTransparency = true;
        if (PB::upBtn()) renderCLUT = true;
        if (PB::downBtn()) renderCLUT = false;
        if (PB::aBtn()) renderColorOffset = true;
        if (PB::bBtn()) renderColorOffset = false;
        
        ptui::tasUITileMap.drawBox(1, 1, 30, 3);
        ptui::tasUITileMap.setCursor(2, 2);
        ptui::tasUITileMap.printString("Next:");
        ptui::tasUITileMap.printString(nextScene);
        
        
        ptui::tasUITileMap.drawBox(1, 5, 36, 7);
        ptui::tasUITileMap.setCursor(2, 6);
        ptui::tasUITileMap.printString("Trans=");
        ptui::tasUITileMap.printString(renderTransparency ? "ON" : "OFF");
        ptui::tasUITileMap.printString(", CLUT=");
        ptui::tasUITileMap.printString(renderCLUT ? "ON" : "OFF");
        ptui::tasUITileMap.printString(", COff=");
        ptui::tasUITileMap.printString(renderColorOffset ? "ON" : "OFF");

        ticks++;
        if (ticks == 60)
        {
            printf("fps=%d\n", PC::fps_counter);
            ticks = 0;
            ptui::tasUITileMap.drawBox(30, 1, 35, 3);
            ptui::tasUITileMap.setCursor(32, 2);
            ptui::tasUITileMap.printInteger(PC::fps_counter);
        }
        
        int lineFillerIndex = (renderTransparency ? 4 : 0) + (renderCLUT ? 2 : 0) | (renderColorOffset ? 1 : 0);
        
        PD::lineFillers[2] = availableLineFillers[lineFillerIndex];
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
    PD::lineFillers[2] = availableLineFillers[7];
    
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
