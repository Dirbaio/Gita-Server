TEMPLATE = app
CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS += -pg -Wextra -Wconversion -Wuninitialized -Wmissing-include-dirs -Wshadow -g
QMAKE_LDFLAGS += -pg -g

LIBS += -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio -lsfml-network

SOURCES += main.cpp

