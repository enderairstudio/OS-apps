#include "TextEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>

TextEditor::TextEditor(const QString& filePath, QWidget* parent)
    : QDialog(parent), m_filePath(filePath) {
    
    setWindowTitle("Editor - " + QFileInfo(filePath).fileName());
    resize(700, 500);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setFontFamily("monospace");
    m_textEdit->setStyleSheet(
        "QTextEdit { background-color: #131124; color: #f8f8f2; border: 1px solid #331f4d; border-radius: 8px; padding: 10px; font-size: 14px; }"
    );
    layout->addWidget(m_textEdit);

    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_textEdit->setPlainText(in.readAll());
        file.close();
    }

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton("Save Changes", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    
    btnLayout->addStretch();
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);
    layout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &TextEditor::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void TextEditor::onSave() {
    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_textEdit->toPlainText();
        file.close();
        accept();
    } else {
        QMessageBox::critical(this, "Save Error", "Could not save file contents!");
    }
}
