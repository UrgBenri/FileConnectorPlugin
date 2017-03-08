!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui
	
    DEFINES += LOG_IMPORTER
    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
            $$PWD/LogImporterWidget.cpp \
            $$PWD/BatchLogImporterWidget.cpp
			
    HEADERS  += \
            $$PWD/LogImporterWidget.h \
            $$PWD/BatchLogImporterWidget.h
			
    FORMS += \
            $$PWD/LogImporterWidget.ui \
            $$PWD/BatchLogImporterWidget.ui
}
