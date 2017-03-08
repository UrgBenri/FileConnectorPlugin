!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui

    !include($$PWD/../RangeViewWidget/RangeViewWidget.pri) {
            error("Unable to include Range View Widget.")
    }

    DEFINES += LOG_EXPORTER
    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
            $$PWD/LogExporterWidget.cpp \
            $$PWD/BatchLogExporterWidget.cpp
			
    HEADERS  += \
            $$PWD/LogExporterWidget.h \
            $$PWD/BatchLogExporterWidget.h
			
    FORMS += \
            $$PWD/LogExporterWidget.ui \
            $$PWD/BatchLogExporterWidget.ui
}
