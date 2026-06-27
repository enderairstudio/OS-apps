#include "ui/Dialog.hpp"
#include "utils/Logger.hpp"

namespace PaperPad {

Dialog::Dialog() {
    m_bounds = {20, 8, 40, 8};
}

void Dialog::show(DialogType type, const std::wstring& title, const std::wstring& prompt, const std::wstring& defaultValue) {
    m_type = type;
    m_title = title;
    m_prompt = prompt;
    m_inputValue = defaultValue;
    m_active = true;
    m_result = DialogResult::Pending;
    m_confirmSelection = 0;
}

void Dialog::close() {
    m_active = false;
}

void Dialog::clampDialogBounds(int screenW, int screenH) {
    // Center the dialog based on current screen size
    m_bounds.w = 46;
    m_bounds.h = m_type == DialogType::Input ? 9 : 8;
    m_bounds.x = (screenW - m_bounds.w) / 2;
    m_bounds.y = (screenH - m_bounds.h) / 2;
}

void Dialog::draw(Console& console) {
    if (!m_active) return;

    int w = console.getWidth();
    int h = console.getHeight();
    clampDialogBounds(w, h);

    const auto& theme = console.getTheme();

    // Draw solid shadow or clear background
    console.fillRect(m_bounds, L' ', theme.menuBar);
    console.drawBorder(m_bounds, theme.activeBorder, true);

    // Draw Title
    std::wstring titleText = L" " + m_title + L" ";
    int titleX = m_bounds.x + (m_bounds.w - static_cast<int>(titleText.length())) / 2;
    console.writeText(titleX, m_bounds.y, titleText, theme.menuBarActive);

    // Draw Prompt
    int promptY = m_bounds.y + 2;
    int promptX = m_bounds.x + 3;
    console.writeText(promptX, promptY, m_prompt, theme.menuBarText);

    if (m_type == DialogType::Input) {
        // Draw input box
        int inputX = m_bounds.x + 3;
        int inputY = m_bounds.y + 4;
        int inputW = m_bounds.w - 6;

        console.fillRect({inputX, inputY, inputW, 1}, L' ', theme.background);
        console.writeText(inputX, inputY, m_inputValue, theme.text);

        // Position console cursor at end of input value
        int cursorX = inputX + static_cast<int>(m_inputValue.length());
        console.setCursorPos(cursorX, inputY);
        console.showCursor(true);

        // Cancel/OK hints
        std::wstring hint = L"Enter: Ok  ∙  Esc: Cancel";
        int hintX = m_bounds.x + (m_bounds.w - static_cast<int>(hint.length())) / 2;
        console.writeText(hintX, m_bounds.y + 6, hint, theme.menuBarText | FOREGROUND_INTENSITY);
    } 
    else if (m_type == DialogType::Confirm) {
        // Render Yes / No / Cancel Buttons
        int buttonY = m_bounds.y + 4;
        
        std::wstring btnYes = L"[ Yes ]";
        std::wstring btnNo  = L"[  No  ]";
        std::wstring btnCan = L"[ Cancel ]";

        int yesX = m_bounds.x + 4;
        int noX = yesX + 11;
        int canX = noX + 12;

        console.writeText(yesX, buttonY, btnYes, m_confirmSelection == 0 ? theme.menuBarActive : theme.menuBarText);
        console.writeText(noX, buttonY, btnNo, m_confirmSelection == 1 ? theme.menuBarActive : theme.menuBarText);
        console.writeText(canX, buttonY, btnCan, m_confirmSelection == 2 ? theme.menuBarActive : theme.menuBarText);

        console.showCursor(false);
    } 
    else if (m_type == DialogType::Alert) {
        int buttonY = m_bounds.y + 4;
        std::wstring btnOk = L"[  OK  ]";
        int okX = m_bounds.x + (m_bounds.w - static_cast<int>(btnOk.length())) / 2;
        
        console.writeText(okX, buttonY, btnOk, theme.menuBarActive);
        console.showCursor(false);
    }
}

void Dialog::handleInput(const UIEvent& ev) {
    if (!m_active) return;

    if (ev.type == UIEvent::Type::Key) {
        if (m_type == DialogType::Input) {
            if (ev.key.code == Key::Escape) {
                m_result = DialogResult::Cancel;
                close();
            } 
            else if (ev.key.code == Key::Enter) {
                m_result = DialogResult::Ok;
                close();
            } 
            else if (ev.key.code == Key::Backspace) {
                if (!m_inputValue.empty()) {
                    m_inputValue.pop_back();
                }
            } 
            else if (ev.key.code == Key::Char) {
                m_inputValue += ev.key.ch;
            }
        } 
        else if (m_type == DialogType::Confirm) {
            if (ev.key.code == Key::Escape) {
                m_result = DialogResult::Cancel;
                close();
            } 
            else if (ev.key.code == Key::Left) {
                m_confirmSelection = (m_confirmSelection - 1 + 3) % 3;
            } 
            else if (ev.key.code == Key::Right) {
                m_confirmSelection = (m_confirmSelection + 1) % 3;
            } 
            else if (ev.key.code == Key::Enter) {
                if (m_confirmSelection == 0) m_result = DialogResult::Yes;
                else if (m_confirmSelection == 1) m_result = DialogResult::No;
                else m_result = DialogResult::Cancel;
                close();
            }
        } 
        else if (m_type == DialogType::Alert) {
            if (ev.key.code == Key::Enter || ev.key.code == Key::Escape || ev.key.code == Key::Char) {
                m_result = DialogResult::Ok;
                close();
            }
        }
    } 
    else if (ev.type == UIEvent::Type::Mouse) {
        Point m = ev.mouse.pos;
        bool isClick = (ev.mouse.buttonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

        if (isClick) {
            if (m_type == DialogType::Confirm) {
                int buttonY = m_bounds.y + 4;
                if (m.y == buttonY) {
                    int yesX = m_bounds.x + 4;
                    int noX = yesX + 11;
                    int canX = noX + 12;

                    if (m.x >= yesX && m.x < yesX + 7) {
                        m_result = DialogResult::Yes;
                        close();
                    } 
                    else if (m.x >= noX && m.x < noX + 8) {
                        m_result = DialogResult::No;
                        close();
                    } 
                    else if (m.x >= canX && m.x < canX + 10) {
                        m_result = DialogResult::Cancel;
                        close();
                    }
                }
            } 
            else if (m_type == DialogType::Alert) {
                int buttonY = m_bounds.y + 4;
                int okX = m_bounds.x + m_bounds.w / 2 - 4;
                if (m.y == buttonY && m.x >= okX && m.x < okX + 8) {
                    m_result = DialogResult::Ok;
                    close();
                }
            }
        }
    }
}

} // namespace PaperPad
