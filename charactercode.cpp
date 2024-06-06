
#include "charactercode.h"

#include <QApplication>
#include <QTranslator>
#include <QClipboard>
#include <QMap>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLocale>
#include <QTimer>
#include <QDebug>

int CharacterCode::init(QMap<QString, QString> params, QWidget *parent)
{
    m_action = new QAction(tr("Character Code"), parent);
    connect(m_action, &QAction::triggered, [&,parent](){
        QDialog dialog(parent);
        dialog.setWindowTitle(tr("Character Code"));
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QRadioButton *msbRadio = new QRadioButton(tr("Most Significant Byte First"), &dialog);
        QRadioButton *lsbRadio = new QRadioButton(tr("Least Significant Byte First"), &dialog);
        if(utf8BytesOrderMsB) {
            msbRadio->setChecked(true);
        } else {
            lsbRadio->setChecked(true);
        }
        layout->addWidget(msbRadio);
        layout->addWidget(lsbRadio);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if(dialog.exec() == QDialog::Accepted) {
            utf8BytesOrderMsB = msbRadio->isChecked();
            emit writeSettings("CharacterCode","BytesOrderMsB", QVariant::fromValue(utf8BytesOrderMsB));
        }
    });

    QTimer::singleShot(10,this,[&](){
        QVariant bytesOrderMsBVariant;
        emit readSettings("CharacterCode","BytesOrderMsB", bytesOrderMsBVariant);
        if(bytesOrderMsBVariant.isValid()) {
            utf8BytesOrderMsB = bytesOrderMsBVariant.toBool();
        } else {
            utf8BytesOrderMsB = true;
            emit writeSettings("CharacterCode","BytesOrderMsB", QVariant::fromValue(utf8BytesOrderMsB));
        }
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
            QString utf8(selectedText.at(i));
            QByteArray utf8Bytes = utf8.toUtf8();
            QString utf8BytesStr;
            if(utf8BytesOrderMsB) {
                for(int j = 0; j < utf8Bytes.length(); j++) {
                    utf8BytesStr += QString("%1").arg((uint8_t)utf8Bytes.at(j), 0, 16);
                }
            } else {
                for(int j = utf8Bytes.length()-1; j >= 0; j--) {
                    utf8BytesStr += QString("%1").arg((uint8_t)utf8Bytes.at(j), 0, 16);
                }
            }
            utf8Text += QString("0x%1 ").arg(utf8BytesStr);
        }
        if(!utf8Text.isEmpty()) {
            QApplication::clipboard()->setText(utf8Text);
        } else {
            QMessageBox::information(parentMenu, tr("Copy to UTF-8"), tr("No character found"));
        }
    });

    QLocale locale;
    bool isUInt64Number = false;
    uint64_t number = 0;
    if(selectedText.startsWith("0x")) {
        QString testText = selectedText.mid(2);
        number = testText.toULongLong(&isUInt64Number,16);
    } else if(selectedText.startsWith("\\u")) {
        QString testText = selectedText.mid(2);
        number = testText.toULongLong(&isUInt64Number,16);
    } else if(selectedText.startsWith("u")) {
        QString testText = selectedText.mid(1);
        number = testText.toULongLong(&isUInt64Number,16);
    } else {
        number = selectedText.toULongLong(&isUInt64Number,10);
        if(!isUInt64Number) {
            number = locale.toULongLong(selectedText,&isUInt64Number);
        }
    }
    if(isUInt64Number) {
        if(number<=127) {
            QAction *showASCIIAction = new QAction(tr("Show ASCII"), parentMenu);
            actions.append(showASCIIAction);
            connect(showASCIIAction, &QAction::triggered, [=](){
                QMessageBox::information(parentMenu, tr("Show ASCII"), QString("%1").arg(QChar((uint8_t)number).toLatin1()));
            });
        }
        if(number<=0xffff) {
            QAction *showUnicodeAction = new QAction(tr("Show Unicode"), parentMenu);
            actions.append(showUnicodeAction);
            connect(showUnicodeAction, &QAction::triggered, [=](){
                QMessageBox::information(parentMenu, tr("Show Unicode"), QString("%1").arg(QChar((uint16_t)number)));
            });
        }
        QAction *showUTF8Action = new QAction(tr("Show UTF-8"), parentMenu);
        actions.append(showUTF8Action);
        connect(showUTF8Action, &QAction::triggered, [=](){
            QByteArray utf8Bytes;
            if(utf8BytesOrderMsB) {
                for(int i = 0; i < 8; i++) {
                    uint8_t byte = (uint8_t)(number>>(8*(7-i)));
                    if(byte == 0) {
                        continue;
                    }
                    utf8Bytes.append(byte);
                }
            } else {
                for(int i = 0; i < 8; i++) {
                    uint8_t byte = (uint8_t)(number>>(8*i));
                    if(byte == 0) {
                        continue;
                    }
                    utf8Bytes.append(byte);
                }
            }
            QMessageBox::information(parentMenu, tr("Show UTF-8"), QString::fromUtf8(utf8Bytes));
        });
    } else {
        bool isFloatNumber = false;
        float fnumber = selectedText.toFloat(&isFloatNumber);
        if(!isFloatNumber) {
            fnumber = locale.toFloat(selectedText,&isFloatNumber);
        }
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
        if(!isDoubleNumber) {
            dnumber = locale.toFloat(selectedText,&isDoubleNumber);
        }
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

void CharacterCode::setLanguage(const QLocale &language,QApplication *app) {
    static QTranslator *qtTranslator = nullptr;
    if(qtTranslator == nullptr) {
        qtTranslator = new QTranslator(app);
    } else {
        app->removeTranslator(qtTranslator);
        delete qtTranslator;
        qtTranslator = new QTranslator(app);
    }
    switch(language.language()) {
    case QLocale::Chinese:
        if(qtTranslator->load(":/lang/charactercode_zh_CN.qm"))
            app->installTranslator(qtTranslator);
        break;
    default:
    case QLocale::English:
        if(qtTranslator->load(":/lang/charactercode_en_US.qm"))
            app->installTranslator(qtTranslator);
        break;
    }
}

void CharacterCode::retranslateUi() {
    m_action->setText(tr("Character Code"));
}
