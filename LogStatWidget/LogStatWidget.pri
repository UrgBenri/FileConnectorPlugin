!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui

    DEFINES += LOG_STAT
    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
            $$PWD/LogStatWidget.cpp

    HEADERS  += \
            $$PWD/LogStatWidget.h

    FORMS += \
            $$PWD/LogStatWidget.ui
}
