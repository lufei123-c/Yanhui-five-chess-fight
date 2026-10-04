SOURCES += $$PWD/protocol.cpp
HEADERS += $$PWD/protocol.h
INCLUDEPATH += $$PWD

# Protocol 依赖 GameEngine 的头文件
include(../GameEngine/gameengine.pri)
