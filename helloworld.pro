TEMPLATE = lib
CONFIG += plugin
QT += core widgets

INCLUDEPATH += $$PWD/plugininterface \
        $$PWD
HEADERS = plugininterface/plugininterface.h \
        helloworld.h
SOURCES = plugininterface/plugininterface.cpp \
        helloworld.cpp

TARGET = $$qtLibraryTarget(helloworld)

win32:{
    build_info.commands = $$quote("c:/Windows/system32/WindowsPowerShell/v1.0/powershell.exe -ExecutionPolicy Bypass -NoLogo -NoProfile -File \"$$PWD/tools/replacer.ps1\" -cc_info $$QMAKE_CXX -tpath $$PWD/plugininterface/plugininterface.json.temp -opath $$PWD/plugininterface/plugininterface.json")
}

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    build_info.commands = $$quote("cd $$PWD && ./tools/replacer.sh $$QMAKE_CXX $$PWD/plugininterface/plugininterface.json.temp $$PWD/plugininterface/plugininterface.json")
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    build_info.commands = $$quote("cd $$PWD && ./tools/replacer.sh $$QMAKE_CXX $$PWD/plugininterface/plugininterface.json.temp $$PWD/plugininterface/plugininterface.json")
}

build_info.target = $$PWD/plugininterface/plugininterface.json
build_info.depends = FORCE
PRE_TARGETDEPS += $$PWD/plugininterface/plugininterface.json
QMAKE_EXTRA_TARGETS += build_info
