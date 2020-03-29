#ifndef PTUI_CUTETILEMAP_RENDERING_HPP
#   define PTUI_CUTETILEMAP_RENDERING_HPP

#   include "ptui/CuteTileMap.hpp"


namespace ptui
{
    // CuteTileMap - Rendering.
    
    template<bool transparentZeroColor>
    struct PixelOutput
    {
        using Color8 = std::uint8_t;
        
        static void output(Color8& output, Color8 source) noexcept
        {
            output = source;
        }
    };
    
    template<>
    struct PixelOutput<true>
    {
        using Color8 = std::uint8_t;
        
        static void output(Color8& output, Color8 source) noexcept
        {
            if (source != 0)
                output = source;
        }
    };
    
    
    
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP>
    template<bool transparentZeroColor, bool colorLookUp, bool colorOffset>
    void CuteTileMap<columnsP, rowsP, tileWidthP, tileHeightP, lineWidthP>::renderIntoLineBuffer(Color8* lineBuffer, int y, bool skip) noexcept
    {
        // Row initialization / change.
        
        if (y == 0)
        {
            _tileY = _tileYStart;
            _tileSubY = _tileSubYStart;
            _tileImageRowBase = _tilesetImage + _tileSubY * tileWidth;
        }
        else
        {
            if (_tileSubY == tileHeight - 1)
            {
                // Onto the next row!
                _tileSubY = 0;
                _tileY++;
                _tileImageRowBase = _tilesetImage;
            }
            else
            {
                // Onto the next pixel line of the same tile row!
                _tileSubY++;
                _tileImageRowBase += tileWidth;
            }
        }
        if ((skip) || (y < _offsetY) || (_tileY >= static_cast<int>(rows)) || (_indexStart >= _indexEnd))
            return ;
        
        // Scanline rendition.
        
        // Local access is faster than field access.
        auto tileImageRowBase = _tileImageRowBase;
        // Current pixel pointer.
        Color8* pixelP = lineBuffer + _indexStart;
        // Last pixel pointer.
        Color8* pixelPEnd = lineBuffer + _indexEnd;
        // Current tile pointer.
        const Tile* tileP = &_tiles[_tileIndex(_tileXStart, _tileY)];
        
        // Is the first tile cut in half?
        if (_tileSubXStart != 0)
        {
            // Is the tile empty?
            if (*tileP == 0)
                pixelP += tileWidth - _tileSubXStart;
            else
            {
                auto tileImagePStart = tileImageRowBase + *tileP * tileSize;
                // Let's render it.
                // Points to the current pixel in the tile.
                const Color8* tileImageP = tileImagePStart + _tileSubXStart;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const Color8* tileImagePEnd = tileImagePStart + tileWidth;
                
                if (colorLookUp)
                {
                    if (colorOffset)
                    {
                        // Which palette to use.
                        auto colorLUT = _colorLUT + tileP[colorOffsetIndexOffset];
                        
                        for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                        {
                            auto tilePixel = colorLUT[*tileImageP];
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                    else
                    {
                        // Which palette to use.
                        auto colorLUT = _colorLUT;
                        
                        for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                        {
                            auto tilePixel = colorLookUp ? (colorLUT[*tileImageP]) : (*tileImageP);
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                }
                else
                {
                    if (colorOffset)
                    {
                        auto tileColorOffset = tileP[colorOffsetIndexOffset];
                        
                        for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                        {
                            auto tilePixel = *tileImageP + tileColorOffset;
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                    else
                    {
                        for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                        {
                            auto tilePixel = *tileImageP;
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                }
            }
            tileP++;
        }
        
        // Let's render the middle tiles.
        while (pixelP + tileWidth <= pixelPEnd)
        {
            // Let's skip any empty middle tiles.
            if (*tileP != 0)
            {
                // Middle tile!
                // Let's render it.
                // Points to the current pixel in the tile.
                const Color8* tileImageP = tileImageRowBase + *tileP * tileSize;
                // Points right after the last pixel in the tile's row.
                // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
                const Color8* tileImagePEnd = tileImageP + tileWidth;
                
                
                if (colorLookUp)
                {
                    if (colorOffset)
                    {
                        // Which palette to use.
                        auto colorLUT = _colorLUT + tileP[colorOffsetIndexOffset];
                        
                        for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                        {
                            auto tilePixel = colorLUT[*tileImageP];
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                    else
                    {
                        // Which palette to use.
                        auto colorLUT = _colorLUT;
                        
                        for (int i = 0; i < tileWidth; i++, pixelP++, tileImageP++)
                        {
                            auto tilePixel = colorLookUp ? (colorLUT[*tileImageP]) : (*tileImageP);
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                }
                else
                {
                    if (colorOffset)
                    {
                        auto tileColorOffset = tileP[colorOffsetIndexOffset];
                        
                        for (int i = 0; i < tileWidth; i++, pixelP++, tileImageP++)
                        {
                            auto tilePixel = *tileImageP + tileColorOffset;
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < tileWidth; i++, pixelP++, tileImageP++)
                        {
                            auto tilePixel = *tileImageP;
                            
                            PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                        }
                    }
                }
            }
            else
                pixelP += tileWidth;
            tileP++;
        }
        
        // Let's render the last tile, if there is one.
        if ((pixelP < pixelPEnd) && (*tileP != 0))
        {
            // Last tile!
            // Let's render it.
            // Points to the current pixel in the tile.
            const Color8* tileImageP = tileImageRowBase + *tileP * tileSize;
            // Points right after the last pixel in the tile's row.
            // The line buffer is guaranteed to be greater than a single tile's width by the static_assert on top of the class.
            const Color8* tileImagePEnd = tileImageP + (pixelPEnd - pixelP);
            
            if (colorLookUp)
            {
                if (colorOffset)
                {
                    // Which palette to use.
                    auto colorLUT = _colorLUT + tileP[colorOffsetIndexOffset];
                    
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = colorLUT[*tileImageP];
                        
                        PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                    }
                }
                else
                {
                    // Which palette to use.
                    auto colorLUT = _colorLUT;
                    
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = colorLookUp ? (colorLUT[*tileImageP]) : (*tileImageP);
                        
                        PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                    }
                }
            }
            else
            {
                if (colorOffset)
                {
                    auto tileColorOffset = tileP[colorOffsetIndexOffset];
                    
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = *tileImageP + tileColorOffset;
                        
                        PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                    }
                }
                else
                {
                    for (; tileImageP != tileImagePEnd; pixelP++, tileImageP++)
                    {
                        auto tilePixel = *tileImageP;
                        
                        PixelOutput<transparentZeroColor>::output(*pixelP, tilePixel);
                    }
                }
            }
            tileP++;
        }
    }
}



#endif // PTUI_CUTETILEMAP_RENDERING_HPP