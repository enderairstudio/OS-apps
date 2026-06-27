#pragma once

#include "ui/Window.hpp"
#include <string>

namespace PaperPad {

enum class DialogType {
    Alert,
    Confirm,
    Input
};

enum class DialogResult {
    Pending,
    Yes,
    No,
    Cancel,
    Ok
};

class Dialog : public Window {
public:
    Dialog();
    ~Dialog() override = default;

    void show(DialogType type, const std::wstring& title, const std::wstring& prompt, const std::wstring& defaultValue = L"");
    void close();

    void draw(Console& console) override;
    void handleInput(const UIEvent& ev) override;

    bool isActive() const { return m_active; }
    DialogResult getResult() const { return m_result; }
    const std::wstring& getInputValue() const { return m_inputValue; }

private:
    DialogType m_type;
    std::wstring m_title;
    std::wstring m_prompt;
    std::wstring m_inputValue;
    bool m_active = false;
    DialogResult m_result = DialogResult::Pending;

    int m_confirmSelection = 0; // 0 for Yes, 1 for No, 2 for Cancel (if Confirm)

    void clampDialogBounds(int screenW, int screenH);
};

} // namespace PaperPad
