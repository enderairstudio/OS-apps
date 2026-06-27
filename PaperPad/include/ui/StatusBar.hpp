#pragma once

#include "ui/Window.hpp"
#include "core/Buffer.hpp"

namespace PaperPad {

class StatusBar : public Window {
public:
    StatusBar();
    ~StatusBar() override = default;

    void update(const Buffer& buffer);

    void draw(Console& console) override;
    void handleInput(const UIEvent& ev) override;

private:
    std::wstring m_filename;
    bool m_dirty = false;
    Point m_cursorPos;
    size_t m_lineCount = 0;
};

} // namespace PaperPad
