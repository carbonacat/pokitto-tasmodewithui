#include <Pokitto.h>
#include <miloslav.h>
#include <Tilemap.hpp>
#include <SDFileSystem.h>
#include "sprites/Mareve.h"
#include "maps.h"

int main(){
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

    int characterX=32, characterY=32, c=0, speed=1;
    while (PC::isRunning())
    {
        if (!PC::update()) 
            continue;
        
        {
            int oldX = characterX;
            int oldY = characterY;
    
            if (PB::leftBtn()) characterX -= speed;
            if (PB::rightBtn()) characterX += speed;
            if (PB::downBtn()) characterY += speed;
            if (PB::upBtn()) characterY -= speed;
            
            int tileX = characterX / PROJ_TILE_W;
            int tileY = characterY / PROJ_TILE_H;
            auto tile = gardenPathEnum(tileX, tileY);
            if (tile == Collide)
            {
                characterX = oldX;
                characterY = oldY;
            }
        }

        PD::drawSprite(110 - mareveOriginX, 88 - mareveOriginY, Mareve);
        tilemap.draw(-(characterX - 110), -(characterY - 88));
    }
    
    return 0;
}
