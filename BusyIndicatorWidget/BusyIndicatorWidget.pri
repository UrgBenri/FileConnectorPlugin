!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT       += core gui

    DEPENDPATH += "$$PWD/src"
    INCLUDEPATH += "$$PWD/src"

    SOURCES += $$PWD/src/BusyIndicatorWidget.cpp

    HEADERS  += $$PWD/src/BusyIndicatorWidget.h

    RESOURCES += $$PWD/BusyIndicatorWidget.qrc

}
