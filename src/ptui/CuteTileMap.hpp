#ifndef PTUI_CUTETILEMAP_HPP
#   define PTUI_CUTETILEMAP_HPP

#   include <array>


namespace ptui
{
    // Color lookUp, Transparency and color-offsEt TileMap.
    // A 8BPP TileMap implementation with optional color lookup, optional transparency and optional per-tile color-offset.
    // - `columnsP` and `rowsP` define how large is the tilemap in tiles.
    // - `tileWidthP` and `tileHeightP` define how large a single tile is in pixels.
    // - `lineWidthP` define how big is a scanline buffer in pixels.
    // - `tilesWithDeltasP`, if true, will provide the Delta API, which allows recoloration of single tiles.
    //   - If false, any call related to Deltas will be ignored.
    //
    // Provides the following services:
    // - Set/Get an arbitrary tile. [Tiles Access]
    // - Set/Get the Color Delta for an arbirtrary tile. [Tiles Access]
    // - Clear. [Mass Tiles Access]
    // - Shift. [Mass Tiles Access]
    // - Change the Tileset. [Configurations]
    // - Set/Get the offset to the Screen. [Configurations]
    // - Change the Color Lookup Table. [CLUT]
    // - Render a scanline using the provided Tileset and offset. [Rendering]
    //
    // - column and row are used for the grid coordinates' names.
    // - x and y are used as the pixel/display coordinates' name.
    // For a more interesting implementation, see UITileMap.
    template<unsigned columnsP, unsigned rowsP,
             unsigned tileWidthP, unsigned tileHeightP,
             unsigned lineWidthP,
             bool tilesWithDeltasP>
    class CuteTileMap
    {
        static_assert(lineWidthP > tileWidthP);
        
    public: // Types & Constants.
        using Tile = std::uint8_t;
        using Delta = std::uint8_t;
        using Color = std::uint8_t;
        using TilesetPixel = std::uint8_t;
        using BufferPixel = std::uint8_t;
        
        static constexpr auto columns = columnsP;
        static constexpr auto rows = rowsP;
        static constexpr auto tileWidth = tileWidthP;
        static constexpr auto tileHeight = tileHeightP;
        static constexpr auto tileSize = tileWidth * tileHeightP * 1; // 8bpp
        static constexpr auto width = columns * tileWidthP;
        static constexpr auto height = rows * tileHeightP;
        static constexpr auto lineWidth = lineWidthP;
        
        
    public: // Constructors.
        CuteTileMap() noexcept
        {
            resetCLUT();
        }
        
        
    public: // Configurations.
        // Changes the origin to the top-left of the display, in pixel units.
        // - 0, 0 means the top-left pixel of the map will be the same than the top-left pixel of the display.
        void setOffset(int offsetX, int offsetY) noexcept
        {
            _offsetX = offsetX;
            _offsetY = offsetY;
            if (offsetX >= 0)
            {
                _indexStart = offsetX;
                _indexEnd = std::min<int>(lineWidth, _indexStart + width);
                _tileXStart = 0;
                _tileSubXStart = 0;
            }
            else
            {
                _indexStart = 0;
                _indexEnd = std::min<int>(lineWidth, _indexStart + offsetX + width);
                _tileXStart = -offsetX / tileWidth;
                _tileSubXStart = -offsetX - _tileXStart * tileWidth;
            }
            if (offsetY >= 0)
            {
                _tileYStart = -offsetY / static_cast<int>(tileWidth);
                _tileSubYStart = -offsetY - _tileYStart * tileWidth;
            }
            else
            {
                _tileYStart = -offsetY / tileWidth;
                _tileSubYStart = -offsetY - _tileYStart * tileWidth;
            }
        }
        auto offsetX() const noexcept
        {
            return _offsetX;
        }
        auto offsetY() const noexcept
        {
            return _offsetY;
        }
        
        // Must be called before use!
        // - `tilesetImage` is a vertical-layout 8BPP tileset where each tile is `tileWidth` * `tileHeight` pixels. No size header.
        void setTilesetImage(const TilesetPixel* tilesetImage) noexcept
        {
            _tilesetImage = tilesetImage;
        }
        
        
    public: // Tiles Access.
        // Changes a given tile in this map.
        // - Safe - If `column` or `row` are outside the map, nothing will happen.
        void setTile(int column, int row, Tile tile) noexcept
        {
            if (areCoordsValid(column, row))
                _tiles[_tileIndex(column, row)] = tile;
        }
        
        // Changes the targeted tile's delta.
        // - Safe - If `column` or `row` are outside the map, nothing will happen.
        void setDelta(int column, int row, Delta delta) noexcept
        {
            if ((tilesWithDeltasP) && (areCoordsValid(column, row)))
                _tiles[_tileIndex(column, row) + deltaIndexOffset] = delta;
        }
        
        // Changes the targeted tile and its delta.
        // - Safe - If `column` or `row` are outside the map, nothing will happen.
        void setTileAndDelta(int column, int row, Tile tile, Delta delta) noexcept
        {
            if (areCoordsValid(column, row))
            {
                auto tileIndex = _tileIndex(column, row);
                
                _tiles[tileIndex] = tile;
                if (tilesWithDeltasP)
                    _tiles[tileIndex + deltaIndexOffset] = delta;
            }
        }
        
        // Returns a given tile in this map.
        // - Safe - If `column` or `row` are outside the map, `outsideTile` is returned.
        Tile getTile(int column, int row, Tile outsideTile = 0) const noexcept
        {
            if (areCoordsValid(column, row))
                return _tiles[_tileIndex(column, row)];
            return outsideTile;
        }
        
        // Returns the Delta for a given Tile.
        // - Safe - If `column` or `row` are outside the map, `outsideDelta` is returned.
        Delta getDelta(int column, int row, Delta outsideDelta = 0) const noexcept
        {
            if ((tilesWithDeltasP) && (areCoordsValid(column, row)))
                return _tiles[_tileIndex(column, row) + deltaIndexOffset];
            return outsideDelta;
        }
        
    
    public: // Mass Tiles Access.
    
        // Clears out the whole tilemap, tiles and deltas, .
        void clear(Tile tile = 0, Delta delta = 0) noexcept
        {
            clearTiles(tile);
            clearDeltas(delta);
        }
    
        // Sets the whole map with `tile`.
        void clearTiles(Tile tile) noexcept
        {
            std::fill(_tiles.begin(), _tiles.begin() + tilesSize, tile);
        }
        // Sets the whole map's deltas to `delta`.
        void clearDeltas(Delta delta) noexcept
        {
            if (tilesWithDeltasP)
                std::fill(_tiles.begin() + deltaIndexOffset, _tiles.end(), delta);
        }
        
        // Same than above, on a defined area.
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will be clamped.
        // - `lastColumn` and `lastRow` are included.
        // - Negative and reversed boxes are considered as empty ones.
        void fillRectTiles(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile) noexcept
        {
            fillRectTilesUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                                tile);
        }
        
        // Same than above, but for the tile deltas.
        void fillRectDeltas(int firstColumn, int firstRow, int lastColumn, int lastRow, Delta delta) noexcept
        {
            if (tilesWithDeltasP)
                fillRectDeltasUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                                     delta);
        }
        
        // Both fillRectTiles and fillRectDeltas.
        void fillRectTilesAndDeltas(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile, Delta delta) noexcept
        {
            firstColumn = clampColumn(firstColumn);
            firstRow = clampRow(firstRow);
            lastColumn = clampColumn(lastColumn);
            lastRow = clampRow(lastRow);
            fillRectTilesUnsafe(firstColumn, firstRow, lastColumn, lastRow,
                                tile);
            fillRectDeltasUnsafe(firstColumn, firstRow, lastColumn, lastRow,
                                 delta);
        }
        
        // Shift the whole map by `shiftedColumns` columns and `shiftedRows` rows.
        // - "Introduced" Tiles will be *left as is*.
        // - If Deltas are supported, they're shifted as well.
        void shift(int shiftedColumns, int shiftedRows) noexcept
        {
            shiftUnsafe(0, 0, columns - 1, rows - 1,
                        shiftedColumns, shiftedRows);
        }
        
        // Same than above, on a given area.
        // - `firstColumn`, `firstRow`, `lastColumn` and `lastRow` will be clamped.
        // - `lastColumn` and `lastRow` are included.
        // - Negative and reversed boxes are considered as empty ones.
        void shift(int firstColumn, int firstRow, int lastColumn, int lastRow,
                   int shiftedColumns, int shiftedRows) noexcept
        {
            shiftUnsafe(clampColumn(firstColumn), clampRow(firstRow), clampColumn(lastColumn), clampRow(lastRow),
                        shiftedColumns, shiftedRows);
        }
        
        
    public: // Rendering.
        // Renders a single line.
        // - If `transparentZeroColor` is true, any color resolved (that is, after offset and lookup) to 0 will leave the current color in place.
        // - If `colorLookUp` is true, the tileset's color will be looked up using the embedded Color LookUp Table.
        //   - Will produce a compilation error if this instance doesn't support Color LookUp.
        // - If `colorOffset` is true, each tile's color will be offset by the tile's offset.
        //   - Happens before the Color LookUp step if any.
        //   - Will produce a compilation error if this instance doesn't support Tile Color Offset.
        template<bool transparentZeroColor, bool colorLookUp, bool colorOffset>
        void renderIntoLineBuffer(BufferPixel* lineBuffer, int y, bool skip) noexcept;
        
        
    public: // Coords manipulation.
        // Updates the given grid coordinates so they're clamped inside the box (e.g. negative will be zero'd).
        template<typename CoordsType = int>
        CoordsType clampColumn(CoordsType column) const noexcept
        {
            return std::max<CoordsType>(0, std::min<CoordsType>(column, columns - 1));
        }
        template<typename CoordsType = int>
        CoordsType clampRow(CoordsType row) const noexcept
        {
            return std::max<CoordsType>(0, std::min<CoordsType>(row, rows - 1));
        }
        template<typename CoordsType = int>
        bool areCoordsValid(CoordsType column, CoordsType row) const noexcept
        {
            return (column >= 0) && (column < columns) && (row >= 0) && (row < rows);
        }
        
        
    public: // CLUT.
        // Remaps a color to a new one.
        // - That is, when `color` is encountered in the Tileset, `newColor` will be returned instead.
        // - Using `0` as `newColor` will make the color transparent!
        void mapColor(Color color, Color newColor) noexcept
        {
            _colorLUT[color] = newColor;
        }
        
        // Resets the mapping to a default x => x setting.
        void resetCLUT() noexcept
        {
            Color color = 0;
            
            // Fills the default color lookup table with the same colors.
            for (auto& mappedColor: _colorLUT)
                mappedColor = color++;
        }
        
        
    protected: // Unsafe implementations.
        // `fillRectTiles`, but columns & rows are considered valid within the grid.
        void fillRectTilesUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow, Tile tile) noexcept
        {
            // TODO: Optimizable with cached indexes calculation.
            for (int row = firstRow; row <= lastRow; row++)
                for (int column = firstColumn; column <= lastColumn; column++)
                    setTile(column, row, tile);
        }
        
        // `fillRectDeltas`, but columns & rows are considered valid within the grid.
        void fillRectDeltasUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow, Delta delta) noexcept
        {
            if (tilesWithDeltasP)
                // TODO: Optimizable with cached indexes calculation.
                for (int row = firstRow; row <= lastRow; row++)
                    for (int column = firstColumn; column <= lastColumn; column++)
                        setDelta(column, row, delta);
        }
        
        // `shift`, but columns & rows are considered valid within the grid.
        void shiftUnsafe(int firstColumn, int firstRow, int lastColumn, int lastRow,
                         int shiftedColumns, int shiftedRows) noexcept
        {
            int shiftFirstColumn;
            int shiftEndColumn;
            int shiftColumnIncrement;
            int shiftFirstRow;
            int shiftEndRow;
            int shiftRowIncrement;
            
            if (shiftedColumns < 0)
            {
                // We start from the first column.
                shiftFirstColumn = firstColumn;
                shiftEndColumn = lastColumn + 1 + shiftedColumns;
                shiftColumnIncrement = 1;
            }
            else
            {
                // We start from the last column.
                shiftFirstColumn = lastColumn;
                shiftEndColumn = firstColumn + shiftedColumns - 1;
                shiftColumnIncrement = -1;
            }
            if (shiftedRows < 0)
            {
                // We start from the first column.
                shiftFirstRow = firstRow;
                shiftEndRow = lastRow + 1 + shiftedRows;
                shiftRowIncrement = 1;
            }
            else
            {
                // We start from the last column.
                shiftFirstRow = lastRow;
                shiftEndRow = firstRow + shiftedRows - 1;
                shiftRowIncrement = -1;
            }
            for (int row = shiftFirstRow; row != shiftEndRow; row += shiftRowIncrement)
            {
                // TODO: Can be sped up with MemOps.
                for (int column = shiftFirstColumn; column != shiftEndColumn; column += shiftColumnIncrement)
                {
                    auto sourceIndex = _tileIndex(column - shiftedColumns, row - shiftedRows);
                    auto destIndex = _tileIndex(column, row);
                    
                    _tiles[destIndex] = _tiles[sourceIndex];
                    if (tilesWithDeltasP)
                        _tiles[destIndex + deltaIndexOffset] = _tiles[sourceIndex + deltaIndexOffset];
                }
            }
        }
    
    private:
        // All the tiles, optionally with their Delta.
        static constexpr auto tilesSize = columns * rows;
        static constexpr auto deltasSize = tilesWithDeltasP ? (columns * rows) : 0;
        using Tiles = std::array<Tile, tilesSize + deltasSize>;
        
        static constexpr auto deltaIndexOffset = tilesSize;
    
        static constexpr auto _tileIndex(short tileX, short tileY) noexcept
        {
            return tileX + tileY * columns;
        }
    
        short _offsetX = 0;
        short _offsetY = 0;
    
        // Changes only when an offset is introduced.
        short _indexStart = 0;
        short _indexEnd = std::min(lineWidth, 0 + width);
        
        // The y coordinate within the current tile.
        short _tileSubY = 0;
        short _tileY = 0;
        
        short _tileXStart = 0;
        short _tileSubXStart = 0;
        short _tileYStart = 0;
        short _tileSubYStart = 0;
        
        // Derived from _tilesetImage
        const TilesetPixel* _tileImageRowBase = nullptr;

        const TilesetPixel* _tilesetImage = nullptr;
        Tiles _tiles = {};
        
        Color _colorLUT[256];
    };
}


#   include "ptui/CuteTileMap.Rendering.hpp"

#endif // PTUI_PALETTETILEMAP_HPP