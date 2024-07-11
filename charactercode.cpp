
#include "charactercode.h"

#include <QApplication>
#include <QTranslator>
#include <QClipboard>
#include <QMap>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QLocale>
#include <QGroupBox>
#include <QTimer>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDebug>

int CharacterCode::init(QMap<QString, QString> params, QWidget *parent)
{
    m_action = new QAction(tr("Character Code"), parent);
    connect(m_action, &QAction::triggered, [&,parent](){
        QDialog dialog(parent);
        dialog.setWindowTitle(tr("Character Code"));
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QGroupBox *bytesOrdergroupBox = new QGroupBox(tr("UTF-8 Bytes Order"), &dialog);
        QRadioButton *msbRadio = new QRadioButton(tr("Most Significant Byte First"), &dialog);
        QRadioButton *lsbRadio = new QRadioButton(tr("Least Significant Byte First"), &dialog);
        if(utf8BytesOrderMsB) {
            msbRadio->setChecked(true);
        } else {
            lsbRadio->setChecked(true);
        }
        QHBoxLayout *bytesOrderLayout = new QHBoxLayout;
        bytesOrderLayout->addWidget(msbRadio);
        bytesOrderLayout->addWidget(lsbRadio);
        bytesOrdergroupBox->setLayout(bytesOrderLayout);
        layout->addWidget(bytesOrdergroupBox);
        QGroupBox *base64groupBox = new QGroupBox(tr("Base64 Encoding"), &dialog);
        QRadioButton *base64Radio = new QRadioButton(tr("Base64"), &dialog);
        QRadioButton *base64urlRadio = new QRadioButton(tr("Base64 URL"), &dialog);
        if(base64) {
            base64Radio->setChecked(true);
        } else {
            base64urlRadio->setChecked(true);
        }
        QHBoxLayout *base64Layout = new QHBoxLayout;
        base64Layout->addWidget(base64Radio);
        base64Layout->addWidget(base64urlRadio);
        base64groupBox->setLayout(base64Layout);
        layout->addWidget(base64groupBox);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if(dialog.exec() == QDialog::Accepted) {
            utf8BytesOrderMsB = msbRadio->isChecked();
            base64 = base64Radio->isChecked();
            emit writeSettings("CharacterCode","BytesOrderMsB", QVariant::fromValue(utf8BytesOrderMsB));
            emit writeSettings("CharacterCode","Base64", QVariant::fromValue(base64));
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
        QVariant base64Variant;
        emit readSettings("CharacterCode","Base64", bytesOrderMsBVariant);
        if(base64Variant.isValid()) {
            base64 = base64Variant.toBool();
        } else {
            base64 = true;
            emit writeSettings("CharacterCode","Base64", QVariant::fromValue(base64));
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

    QByteArray::Base64Options base64Type = base64 ? QByteArray::Base64Encoding : QByteArray::Base64UrlEncoding;
    QAction *copyToBase64Action = new QAction(tr("Copy to Base64"), parentMenu);
    actions.append(copyToBase64Action);
    connect(copyToBase64Action, &QAction::triggered, [=](){
        QApplication::clipboard()->setText(selectedText.toUtf8().toBase64(base64Type|QByteArray::KeepTrailingEquals));
    });
    QFileInfo fileInfo(selectedText);
    if(fileInfo.exists()) {
        QAction *copyToBase64FileAction = new QAction(tr("Copy to Base64 (File)"), parentMenu);
        actions.append(copyToBase64FileAction);
        connect(copyToBase64FileAction, &QAction::triggered, [=](){
            QFile file(selectedText);
            QFileInfo fileInfo(file);
            if(fileInfo.size() > 8*1024) {
                int ret = QMessageBox::question(parentMenu, tr("Copy to Base64 (File)"), tr("The file is too large, continue?"), QMessageBox::Yes|QMessageBox::No);
                if(ret != QMessageBox::Yes) {
                    return;
                }
            }
            if(file.open(QIODevice::ReadOnly)) {
                QByteArray fileData = file.readAll();
                file.close();
                QByteArray fileBase64Data = fileData.toBase64(base64Type|QByteArray::KeepTrailingEquals);
                QApplication::clipboard()->setText(fileBase64Data);
            }
        });
    }
    QByteArray base64Bytes = QByteArray::fromBase64(selectedText.toUtf8(),base64Type|QByteArray::KeepTrailingEquals|QByteArray::AbortOnBase64DecodingErrors);
    if(!base64Bytes.isEmpty()) {
        QAction *copyFromBase64Action = new QAction(tr("Copy from Base64"), parentMenu);
        actions.append(copyFromBase64Action);
        connect(copyFromBase64Action, &QAction::triggered, [=](){
            QApplication::clipboard()->setText(QString::fromUtf8(base64Bytes));
        });
        QAction *saveFileFromBase64Action = new QAction(tr("Save File from Base64"), parentMenu);
        actions.append(saveFileFromBase64Action);
        connect(saveFileFromBase64Action, &QAction::triggered, [=](){
            QString saveFileName = QFileDialog::getSaveFileName(parentMenu, tr("Save File from Base64"), QString(), tr("All Files (*)"));
            if(!saveFileName.isEmpty()) {
                QFile file(saveFileName);
                if(file.open(QIODevice::WriteOnly)) {
                    file.write(base64Bytes);
                    file.close();
                }
            }
        });
    }

    auto checkStringNum = [](const QString &text, bool *isNumber) -> uint64_t {
        QLocale locale;
        if(text.startsWith("0x")) {                      // hex 0xhh
            QString testText = text.mid(2);
            return testText.toULongLong(isNumber,16);
        }else if(text.startsWith("\\u")) {              // unicode \uhhhh
            QString testText = text.mid(2);
            return testText.toULongLong(isNumber,16);
        } else if(text.startsWith("\\")) {               // dec \nnn
            QString testText = text.mid(1);
            return testText.toULongLong(isNumber,10);
        } else if(text.startsWith("u")) {                // unicode uhhhh
            QString testText = text.mid(1);
            return testText.toULongLong(isNumber,16);
        } else {                                         // dec nnn
            return text.toULongLong(isNumber,10);
            if(!isNumber) {                              // dec(locale) nnn
                return locale.toULongLong(text,isNumber);
            }
        }
        *isNumber = false;
        return 0;
    };

    // single character
    bool isUInt64Number = false;
    bool isFloatNumber = false;
    bool isDoubleNumber = false;
    uint64_t number = checkStringNum(selectedText, &isUInt64Number);
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
        float fnumber = selectedText.toFloat(&isFloatNumber);
        if(!isFloatNumber) {
            QLocale locale;
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
        double dnumber = selectedText.toDouble(&isDoubleNumber);
        if(!isDoubleNumber) {
            QLocale locale;
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

    auto tryParseNumberList = [checkStringNum](const QString &text, QList<uint64_t> &numberList) -> bool {
        bool isNumber = false;
        QList<QString> strList = text.split(" ",Qt::SkipEmptyParts);
        if(strList.isEmpty()) {
            return false;
        }
        if(strList.size() == 1) {
            return false;
        }
        foreach(QString str, strList) {
            uint64_t number = checkStringNum(str, &isNumber);
            if(isNumber) {
                numberList.append(number);
            } else {
                break;
            }
        }
        return isNumber && numberList.size() == strList.size();
    };
    // multiple characters
    if(!(isUInt64Number || isFloatNumber || isDoubleNumber)) {
        QList<uint64_t> unmberList;
        bool isNumber = tryParseNumberList(selectedText, unmberList);
        if(!isNumber) {
            unmberList.clear();
            QString selectedTextUpdate = selectedText;
            selectedTextUpdate.replace("0x"," 0x");
            isNumber = tryParseNumberList(selectedTextUpdate, unmberList);
            if(!isNumber) {
                unmberList.clear();
                selectedTextUpdate = selectedText;
                selectedTextUpdate.replace("\\"," \\");
                isNumber = tryParseNumberList(selectedTextUpdate, unmberList);
                if(!isNumber) {
                    unmberList.clear();
                    selectedTextUpdate = selectedText;
                    selectedTextUpdate.replace("u"," u");
                    isNumber = tryParseNumberList(selectedTextUpdate, unmberList);
                    if(!isNumber) {
                        unmberList.clear();
                        selectedTextUpdate = selectedText;
                        selectedTextUpdate.replace("\\u"," \\u");
                        isNumber = tryParseNumberList(selectedTextUpdate, unmberList);
                    }
                }
            }
        }

        if(isNumber){
            bool isAllASCII = true;
            bool isAllUnicode = true;
            foreach(uint64_t number, unmberList) { 
                if(number>127) {
                    isAllASCII = false;
                }
                if(number>0xffff) {
                    isAllUnicode = false;
                }
            }
            if(isAllASCII) {
                QAction *showASCIIAction = new QAction(tr("Show ASCII"), parentMenu);
                actions.append(showASCIIAction);
                connect(showASCIIAction, &QAction::triggered, [=](){
                    QString asciiText;
                    foreach(uint64_t number, unmberList) {
                        asciiText += QString("%1").arg(QChar((uint8_t)number).toLatin1());
                    }
                    QMessageBox::information(parentMenu, tr("Show ASCII"), asciiText);
                });
            }
            if(isAllUnicode) {
                QAction *showUnicodeAction = new QAction(tr("Show Unicode"), parentMenu);
                actions.append(showUnicodeAction);
                connect(showUnicodeAction, &QAction::triggered, [=](){
                    QString unicodeText;
                    foreach(uint64_t number, unmberList) {
                        unicodeText += QString("%1").arg(QChar((uint16_t)number));
                    }
                    QMessageBox::information(parentMenu, tr("Show Unicode"), unicodeText);
                });
            }
            QAction *showUTF8Action = new QAction(tr("Show UTF-8"), parentMenu);
            actions.append(showUTF8Action);
            connect(showUTF8Action, &QAction::triggered, [=](){
                QByteArray utf8Bytes;
                foreach(uint64_t number, unmberList) {
                    if(number == 0) {
                        utf8Bytes.append('0');
                        continue;
                    }
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
                }
                QMessageBox::information(parentMenu, tr("Show UTF-8"), QString::fromUtf8(utf8Bytes));
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
