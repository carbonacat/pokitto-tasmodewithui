#ifndef PTUI_UITILEMAP_HPP
#   define PTUI_UITILEMAP_HPP

#   include "ptui/BaseTileMap.hpp"


namespace ptui
{
    // Defines which indexes to use to get tileset's special tiles (texts, widgets, etc).
    // - You can actually use a completely different tileset layout by changing UITileMap's TilesetDefinition parameter to a similar class!
    // - However there are some restrictions:
    //    - The 0 tile is ALWAYS used as the null tile.
    //    - Gauge's parts' Empty and Full defines a contiguous range of filling - i.e. everything between them will be considered as "partly filled".
    //    - Text is always contiguous as well, starting from `asciiSpace`.
    struct UITilesetDefinition
    {
        static constexpr auto gaugeLeftEmpty = 5;
        static constexpr auto gaugeLeftFull = 9;
        static constexpr auto gaugeMiddleEmpty = 10;
        static constexpr auto gaugeMiddleFull = 16;
        static constexpr auto gaugeRightEmpty = 17;
        static constexpr auto gaugeRightFull = 21;
        
        static constexpr auto checkboxUnchecked = 22;
        static constexpr auto checkboxChecked = 23;
        
        static constexpr auto asciiSpace = 32;
        
        static constexpr auto boxTopLeft = 24;
        static constexpr auto boxTop = 25;
        static constexpr auto boxTopRight = 26;
        static constexpr auto boxRight = 27;
        static constexpr auto boxBottomRight = 28;
        static constexpr auto boxBottom = 29;
        static constexpr auto boxBottomLeft = 30;
        static constexpr auto boxLeft = 31;
        static constexpr auto boxMiddle = asciiSpace;
    };
    
    // An extension of BaseTileMap revolving around a UI TileSet.
    // - TilesetDefinition is used to know where to get the relevant tiles in the underlying Tileset.
    // Provides the following services:
    // - Text printing with bounds and bounded autoscroll. [Terminal]
    // - Box, checkbox, gauge drawing. [Widgets]
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP,
             typename TilesetDefinition = UITilesetDefinition>
    class UITileMap:
        public BaseTileMap<columnsP, rowsP,
                           tileWidthP, tileHeightP,
                           lineWidthP>
    {
    public: // Types.
        TilesetDefinition definition;
        
        
    public: // Widgets.
        // Draws a gauge filled with the given amount, extending from `firstColumn` to `lastColumn` at the given row.
        // - If `current` is negative or 0, the gauge will be empty.
        // - If `current` is `max`, the gauge will be full.
        // - If the gauge is reversed or if `max` is negative, nothing happens.
        // - The drawing will be clipped as needed.
        void drawGauge(int firstColumn, int lastColumn, int row,
                       int current, int max) noexcept
        {
            constexpr auto gaugeLeftCapacity = TilesetDefinition::gaugeLeftFull - TilesetDefinition::gaugeLeftEmpty;
            constexpr auto gaugeMiddleCapacity = TilesetDefinition::gaugeMiddleFull - TilesetDefinition::gaugeMiddleEmpty;
            constexpr auto gaugeRightCapacity = TilesetDefinition::gaugeRightFull - TilesetDefinition::gaugeRightEmpty;
            
            static_assert(gaugeLeftCapacity >= 0, "Gauge Left Part is invalid.");
            static_assert(gaugeMiddleCapacity >= 0, "Gauge Middle Part is invalid.");
            static_assert(gaugeRightCapacity >= 0, "Gauge Right Part is invalid.");
            
            current = std::max(0, std::min(current, max));
            if (firstColumn == lastColumn)
            {
                // It's an edge case. We'll use the Middle part.
                if (gaugeMiddleCapacity == 0)
                    this->set(firstColumn, row, TilesetDefinition::gaugeMiddleFull); // No distinction between empty or full = no value.
                else
                {
                    int filling = (current * gaugeMiddleCapacity + max / 2) / max;
                    
                    this->set(firstColumn, row, TilesetDefinition::gaugeMiddleEmpty + filling);
                }
            }
            else if (firstColumn < lastColumn)
            {
                int gaugeWidth = lastColumn - firstColumn + 1;
                int gaugeMiddleWidth = gaugeWidth - 2;
                int gaugeCapacity = gaugeLeftCapacity + gaugeMiddleWidth * gaugeMiddleCapacity + gaugeRightCapacity;
                int filling = (current * gaugeCapacity + max / 2) / max;
                
                // Left part.
                this->set(firstColumn, row, TilesetDefinition::gaugeLeftEmpty + std::min(filling, gaugeLeftCapacity));
                filling -= gaugeLeftCapacity;
                
                // Middle parts.
                for (int middleColumn = firstColumn + 1; middleColumn < lastColumn; middleColumn++)
                {
                    this->set(middleColumn, row, TilesetDefinition::gaugeMiddleEmpty + std::max(0, std::min(filling, gaugeMiddleCapacity)));
                    filling -= gaugeMiddleCapacity;
                }
                
                // Right parts.
                this->set(lastColumn, row, TilesetDefinition::gaugeRightEmpty + std::max(0, std::min(filling, gaugeRightCapacity)));
            }
        }
        
        // Draws a checkbox at `column`, `row` with the given checked state.
        void drawCheckbox(int column, int row, bool checked) noexcept
        {
            this->set(column, row, checked ? TilesetDefinition::checkboxChecked : TilesetDefinition::checkboxUnchecked);
        }
        
        // Draws a Box.
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will not be clamped, but clipped instead.
        // - `lastColumn` and `lastRow` are included.
        // - Empty or negative boxes won't be drawn.
        // - Due to the limitation of tileset, 1-column and 1-row boxes will be rendered as Spaces.
        void drawBox(int firstColumn, int firstRow, int lastColumn, int lastRow) noexcept
        {
            if ((firstColumn > lastColumn) || (firstRow > firstRow))
                return ;
            if ((firstColumn == lastColumn) || (firstRow == lastRow))
            {
                this->clear(firstColumn, firstRow, lastColumn, lastRow, TilesetDefinition::boxMiddle);
                return ;
            }
            
            // Corners.
            this->set(firstColumn, firstRow, TilesetDefinition::boxTopLeft);
            this->set(firstColumn, lastRow, TilesetDefinition::boxBottomLeft);
            this->set(lastColumn, firstRow, TilesetDefinition::boxTopRight);
            this->set(lastColumn, lastRow, TilesetDefinition::boxBottomRight);
            
            // Borders.
            this->clear(firstColumn + 1, firstRow, lastColumn - 1, firstRow, TilesetDefinition::boxTop);
            this->clear(firstColumn + 1, lastRow, lastColumn - 1, lastRow, TilesetDefinition::boxBottom);
            
            this->clear(firstColumn, firstRow + 1, firstColumn, lastRow - 1, TilesetDefinition::boxLeft);
            this->clear(lastColumn, firstRow + 1, lastColumn, lastRow - 1, TilesetDefinition::boxRight);
            
            // Inside.
            this->clear(firstColumn + 1, firstRow + 1, lastColumn - 1, lastRow - 1, TilesetDefinition::boxMiddle);
        }
    };
}


#endif // PTUI_UITILEMAP_HPP