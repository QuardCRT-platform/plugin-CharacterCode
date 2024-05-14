
#include "charactercode.h"

#include <QApplication>
#include <QClipboard>
#include <QMap>
#include <QMessageBox>
#include <QDebug>

int CharacterCode::init(QMap<QString, QString> params, QWidget *parent)
{
    m_action = new QAction(tr("Character Code"), parent);
    connect(m_action, &QAction::triggered, [parent](){
        QMessageBox::information(parent, tr("Character Code"), tr("Character Code Plugin"));
    });
    Q_UNUSED(params);

    return 0;
}

QList<QAction *> CharacterCode::terminalContextAction(QString selectedText, QString workingDirectory, QMenu *parentMenu)
{
    Q_UNUSED(workingDirectory);
    if(selectedText.isEmpty()) {
        return QList<QAction *>();
    }

    QList<QAction *> actions;
    QAction *copyToASCIIAction = new QAction(tr("Copy to ASCII"), parentMenu);
    actions.append(copyToASCIIAction);
    connect(copyToASCIIAction, &QAction::triggered, [=](){
        QString asciiText;
        bool isAllASCII = true;
        for(int i = 0; i < selectedText.length(); i++)
        {
            uint32_t c = selectedText.at(i).unicode();
            if(c<=127) {
                asciiText += QString("0x%1 ").arg(c, 0, 16);
            } else {
                isAllASCII = false;
            }
        }
        if(!asciiText.isEmpty()) {
            QApplication::clipboard()->setText(asciiText);
            if(!isAllASCII) {
                QMessageBox::information(parentMenu, tr("Copy to ASCII"), tr("Some characters are not ASCII"));
            }
        } else {
            QMessageBox::information(parentMenu, tr("Copy to ASCII"), tr("No ASCII character found"));
        }
    });
    QAction *copyToUnicodeAction = new QAction(tr("Copy to Unicode"), parentMenu);
    actions.append(copyToUnicodeAction);
    connect(copyToUnicodeAction, &QAction::triggered, [=](){
        QString unicodeText;
        for(int i = 0; i < selectedText.length(); i++)
        {
            uint32_t c = selectedText.at(i).unicode();
            unicodeText += QString("\\u%1 ").arg(c, 0, 16);
        }
        if(!unicodeText.isEmpty()) {
            QApplication::clipboard()->setText(unicodeText);
        } else {
            QMessageBox::information(parentMenu, tr("Copy to Unicode"), tr("No character found"));
        }
    });
    QAction *copyToUTF8Action = new QAction(tr("Copy to UTF-8"), parentMenu);
    actions.append(copyToUTF8Action);
    connect(copyToUTF8Action, &QAction::triggered, [=](){
        QString utf8Text;
        for(int i = 0; i < selectedText.length(); i++)
        {
            uint32_t c = selectedText.at(i).unicode();
            if(c<0x80) {
                utf8Text += QString("0x%1 ").arg(c, 0, 16);
            } else if(c<0x800) {
                utf8Text += QString("0x%2%1 ").arg(0xC0 | (c >> 6), 2, 16, QChar('0')).arg(0x80 | (c & 0x3F), 2, 16, QChar('0'));
            } else if(c<0x10000) {
                utf8Text += QString("0x%3%2%1 ").arg(0xE0 | (c >> 12), 2, 16, QChar('0')).arg(0x80 | ((c >> 6) & 0x3F), 2, 16, QChar('0')).arg(0x80 | (c & 0x3F), 2, 16, QChar('0'));
            } else {
                utf8Text += QString("0x%4%3%2%1 ").arg(0xF0 | (c >> 18), 2, 16, QChar('0')).arg(0x80 | ((c >> 12) & 0x3F), 2, 16, QChar('0')).arg(0x80 | ((c >> 6) & 0x3F), 2, 16, QChar('0')).arg(0x80 | (c & 0x3F), 2, 16, QChar('0'));
            }
        }
        if(!utf8Text.isEmpty()) {
            QApplication::clipboard()->setText(utf8Text);
        } else {
            QMessageBox::information(parentMenu, tr("Copy to UTF-8"), tr("No character found"));
        }
    });

    // check selectedText is number or not
    bool isUIntNumber = false;
    uint32_t number = 0;
    if(selectedText.startsWith("0x")) {
        QString testText = selectedText.mid(2);
        number = testText.toUInt(&isUIntNumber,16);
    } else if(selectedText.startsWith("\\u")) {
        QString testText = selectedText.mid(2);
        number = testText.toUInt(&isUIntNumber,16);
    } else {
        number = selectedText.toUInt(&isUIntNumber,10);
    }
    if(isUIntNumber) {
        if(number<=127) {
            QAction *showASCIIAction = new QAction(tr("Show ASCII"), parentMenu);
            actions.append(showASCIIAction);
            connect(showASCIIAction, &QAction::triggered, [=](){
                QMessageBox::information(parentMenu, tr("Show ASCII"), QString("%1").arg(QChar(number).toLatin1()));
            });
        }
        if(number<=0xffff) {
            QAction *showUnicodeAction = new QAction(tr("Show Unicode"), parentMenu);
            actions.append(showUnicodeAction);
            connect(showUnicodeAction, &QAction::triggered, [=](){
                QMessageBox::information(parentMenu, tr("Show Unicode"), QString("%1").arg(QChar(number)));
            });
        }
        QAction *showUTF8Action = new QAction(tr("Show UTF-8"), parentMenu);
        actions.append(showUTF8Action);
        connect(showUTF8Action, &QAction::triggered, [=](){
            char utf8[5];
            memset(utf8,0,5);
            memcpy(utf8,&number,4);
            QMessageBox::information(parentMenu, tr("Show UTF-8"), QString::fromUtf8(utf8));
        });
    } else {
        bool isFloatNumber = false;
        float fnumber = selectedText.toFloat(&isFloatNumber);
        if(isFloatNumber) {
            QAction *showFloatHexAction = new QAction(tr("Show Float Hex"), parentMenu);
            actions.append(showFloatHexAction);
            connect(showFloatHexAction, &QAction::triggered, [=](){
                uint32_t *p = (uint32_t *)&fnumber;
                QMessageBox::information(parentMenu, tr("Show Float Hex"), QString("0x%1").arg(*p, 0, 16));
            });
        }
        bool isDoubleNumber = false;
        double dnumber = selectedText.toDouble(&isDoubleNumber);
        if(isDoubleNumber) {
            QAction *showDoubleHexAction = new QAction(tr("Show Double Hex"), parentMenu);
            actions.append(showDoubleHexAction);
            connect(showDoubleHexAction, &QAction::triggered, [=](){
                uint64_t *p = (uint64_t *)&dnumber;
                QMessageBox::information(parentMenu, tr("Show Double Hex"), QString("0x%1").arg(*p, 0, 16));
            });
        }
    }

    return actions;
}
