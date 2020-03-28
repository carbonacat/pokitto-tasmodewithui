#include "ptui/TASLineFiller.hpp"

#include "ptui/TASTerminalTileMap.hpp"


namespace ptui
{
    void TerminalTMFiller(std::uint8_t* line, std::uint32_t y, bool skip) noexcept
    {
        tasUITileMap.renderIntoLineBuffer(line, y, skip);
    }
}