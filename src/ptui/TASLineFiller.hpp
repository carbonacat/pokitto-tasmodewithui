#ifndef PTUI_TASLINEFILLER_HPP
#   define PTUI_TASLINEFILLER_HPP

#   include <cstdint>


namespace ptui
{
    // A filler which renders the Terminal.
    void TerminalTMFiller(std::uint8_t* line, std::uint32_t y, bool skip) noexcept;
};


#endif // PTUI_TASLINEFILLER_HPP