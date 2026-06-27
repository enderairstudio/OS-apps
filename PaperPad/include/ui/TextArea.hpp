#pragma once

#include "ui/Window.hpp"
#include "core/Buffer.hpp"
#include "io/Config.hpp"

namespace PaperPad {

class TextArea : public Window {
public:
    TextArea(Buffer& buffer);
    ~TextArea() override = default;

    void draw(Console& console) override;
    void handleInput(const UIEvent& ev) override;

    void setShowLineNumbers(bool show) { m_showLineNumbers = show; }
    bool getShowLineNumbers() const { return m_showLineNumbers; }

    void setTabSize(int size) { m_tabSize = size; }

    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();
    void selectAll();

    Point getScrollOffset() const { return m_scroll; }
    void setScrollOffset(Point offset) { m_scroll = offset; }

    // Scroll viewport to make cursor visible
    void scrollToCursor();

private:
    Buffer& m_buffer;
    Point m_scroll;
    bool m_showLineNumbers = true;
    int m_tabSize = 4;

    // Syntax highlighting details
    struct HighlightToken {
        size_t start;
        size_t length;
        WORD attr;
    };
    std::vector<HighlightToken> getLineHighlights(const std::wstring& line, WORD defaultAttr, const Color::Scheme& theme);
    bool isKeyword(const std::wstring& word);
};

} // namespace PaperPad
