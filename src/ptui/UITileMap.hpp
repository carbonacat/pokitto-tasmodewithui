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
        
        static constexpr auto tabColumns = 4;
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
        
        
    public: // Terminal.
        // Sets where to draw the next character.
        // - Clamped to the Cursor Bounding Box.
        void setCursor(int cursorColumn, int cursorRow) noexcept
        {
            _cursorColumn = std::max<int>(_cursorFirstColumn, std::min<int>(cursorColumn, _cursorLastColumn));
            _cursorRow = std::max<int>(_cursorFirstRow, std::min<int>(cursorRow, _cursorLastRow));
        }
        
        // Changes the cursor's bounding box.
        // - Cursor's position will be updated to fit inside.
        // - Negative boxes
        void setCursorBoundingBox(int cursorFirstColumn, int cursorFirstRow, int cursorLastColumn, int cursorLastRow) noexcept
        {
            _cursorFirstColumn = cursorFirstColumn;
            _cursorFirstRow = cursorFirstRow;
            _cursorLastColumn = std::max<int>(cursorFirstColumn, cursorLastColumn);
            _cursorLastRow = std::max<int>(cursorFirstRow, cursorLastRow);
            setCursor(_cursorColumn, _cursorRow);
        }
        
        // Changes the Cursor Bounding Box back to the default.
        // - Cursor's position will be updated to fit inside.
        void resetCursorBoundingBox() noexcept
        {
            _cursorFirstColumn = 0;
            _cursorFirstRow = 0;
            _cursorLastColumn = this->columns - 1;
            _cursorLastRow = this->rows - 1;
            setCursor(_cursorColumn, _cursorRow);
        }
    
        // Prints a single character.
        // - All the Control characters are ignored, besides newlines and tabs.
        // - Tabs are based on the TilesetDefinition's `tabColumns` characters wide columns, starting from the bound's firstColumn.
        // - If printing such a char would go above the last column, then a newline is generated before.
        //   - If `c` was a space, it won't be printed and transformed instead into the said newline.
        // - Note: This is not suitable for setting a tile to a given value. Use `set()` instead.
        void printChar(char c) noexcept
        {
            if (c == '\n')
            {
                // Newlines gets the cursor back to the first column, and to the next row.
                _cursorColumn = _cursorFirstColumn;
                _cursorRow++;
                if (_cursorRow > _cursorLastRow)
                {
                    // Autoscroll.
                    this->shift(_cursorFirstColumn, _cursorFirstRow, _cursorLastColumn, _cursorLastRow,
                          0, -1);
                    this->clear(_cursorFirstColumn, _cursorLastRow, _cursorLastColumn, _cursorLastRow, TilesetDefinition::boxMiddle);
                    _cursorRow = _cursorLastRow;
                }
            }
            else if (c == '\t')
            {
                int nextColumn = _cursorFirstColumn + ((_cursorColumn - _cursorFirstColumn) / TilesetDefinition::tabColumns + 1) * TilesetDefinition::tabColumns;
                
                if (nextColumn > _cursorLastColumn)
                    printChar('\n');
                else while (_cursorColumn < nextColumn)
                    printChar(' ');
            }
            else if (c >= ' ')
            {
                if (_cursorColumn > _cursorLastColumn)
                {
                    printChar('\n');
                    // Spaces are transformed into the generated newline.
                    if (c == ' ')
                        return ;
                }
                this->set(_cursorColumn, _cursorRow, c - ' ' + TilesetDefinition::asciiSpace);
                _cursorColumn++;
            }
        }
        
        // Prints a string.
        void printString(const char* string) noexcept
        {
            for (auto charP = string; *charP != 0; charP++)
                printChar(*charP);
        }
        
        // Prints a string.
        void printString(const char* string, int limit) noexcept
        {
            for (auto charP = string; (*charP != 0) && (limit > 0); charP++, limit--)
                printChar(*charP);
        }
        
    private:
        short _cursorColumn = 0;
        short _cursorRow = 0;
        short _cursorFirstColumn = 0;
        short _cursorFirstRow = 0;
        short _cursorLastColumn = this->columns - 1;
        short _cursorLastRow = this->rows - 1;
    };
}


#endif // PTUI_UITILEMAP_HPP