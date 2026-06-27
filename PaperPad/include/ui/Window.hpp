#pragma once

#include "utils/Types.hpp"
#include "ui/Console.hpp"

namespace PaperPad {

class Window {
public:
    virtual ~Window() = default;

    virtual void setBounds(Rect bounds) { m_bounds = bounds; }
    Rect getBounds() const { return m_bounds; }

    virtual void setFocus(bool focused) { m_focused = focused; }
    bool isFocused() const { return m_focused; }

    virtual void draw(Console& console) = 0;
    virtual void handleInput(const UIEvent& ev) = 0;

protected:
    Rect m_bounds;
    bool m_focused = false;
};

} // namespace PaperPad
