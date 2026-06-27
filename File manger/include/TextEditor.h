#pragma once
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>

class TextEditor : public QDialog {
    Q_OBJECT
public:
    TextEditor(const QString& filePath, QWidget* parent = nullptr);
private slots:
    void onSave();
private:
    QString m_filePath;
    QTextEdit* m_textEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
};
