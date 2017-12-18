TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


HEADERS += ../src/fplib.h \
           ../src/fpreference.h \
           reftest.h

SOURCES += main.cpp \
           reftest.cpp \
           ../src/fplib.cpp \
           ../src/fpreference.cpp
