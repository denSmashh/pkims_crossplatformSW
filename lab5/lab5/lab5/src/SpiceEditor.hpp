#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

class SpiceEditor;

class Highlighter : public QSyntaxHighlighter {
public:
    Highlighter(QTextDocument* parent);
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression  pattern;
        QTextCharFormat     format;
    };
    QVector<HighlightingRule> highlightingRules;

};


class SpiceEditor : public QTextEdit {
public:
    SpiceEditor(QWidget* parent);

private:
    Highlighter* syntaxHighlighter;

private slots:

};
