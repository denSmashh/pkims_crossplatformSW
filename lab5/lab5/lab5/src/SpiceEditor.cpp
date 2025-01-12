
#include <QFont>

#include "SpiceEditor.hpp"

SpiceEditor::SpiceEditor(QWidget* parent) : QTextEdit(parent) {    
    QFont font;
    font.setFamily("Consolas");
    font.setFixedPitch(true);
    font.setPointSize(15);
    setFont(font);

    moveCursor(QTextCursor::Start);
    ensureCursorVisible();
    
    syntaxHighlighter = new Highlighter(this->document());
}


Highlighter::Highlighter(QTextDocument* parent): QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Numbers
    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(128, 0, 0));
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?(n|p|u|m|k|K|meg|g)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Voltage type
    QTextCharFormat voltageTypeFormat;
    voltageTypeFormat.setForeground(QColor(238, 162, 30));
    rule.pattern = QRegularExpression("\\b(pulse|DC|sin)\\b");
    rule.format = voltageTypeFormat;
    highlightingRules.append(rule);

    // SPICE device names (e.g. R1, C2, etc.)
    QTextCharFormat deviceFormat;
    deviceFormat.setForeground(QColor(146, 255, 50));
    rule.pattern = QRegularExpression("\\b[RLCVDQrlcvdq]\\d+\\b");
    rule.format = deviceFormat;
    highlightingRules.append(rule);

    // SPICE commands
    QTextCharFormat spiceCommandFormat;
    spiceCommandFormat.setForeground(QColor(0, 102, 204));
    spiceCommandFormat.setFontWeight(QFont::Bold);
    const QString spiceCommandsPatterns[] = {
        QString("\\.tran\\b"), QString("\\.dc\\b"),
        QString("\\.ac\\b"), QString("\\.plot\\b"), 
        QString("\\.four\\b"), QString("\\.model\\b"),
        QString("\\.include\\b"), QString("\\.param\\b"),
        QString("\\.step\\b"), QString("\\.end\\b")
    };

    for (const QString& command : spiceCommandsPatterns) {
        rule.pattern = QRegularExpression(command);
        rule.format = spiceCommandFormat;
        highlightingRules.append(rule);
    }

    // SPICE comments
    QTextCharFormat spiceCommentFormat;
    spiceCommentFormat.setForeground(QColor(128, 128, 128));
    rule.pattern = QRegularExpression("^\\*.*$");
    rule.format = spiceCommentFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString& text) {
    for (int i = 0; i < highlightingRules.size(); ++i) {
        HighlightingRule& rule = highlightingRules[i];
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}