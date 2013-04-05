TEMPLATE = app
CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS += -pg -Wextra -Wconversion -Wuninitialized -Wmissing-include-dirs -Wshadow
QMAKE_LDFLAGS += -pg

LIBS += -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio -lsfml-network

SOURCES += main.cpp

