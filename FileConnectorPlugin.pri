!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui widgets

    !include($$PWD/PluginInterface/UrgBenriPluginInterface.pri) {
            error("Unable to include Viewer Plugin Interface.")
    }

    !include($$PWD/ModeSwitcherWidget/ModeSwitcherWidget.pri) {
            error("Unable to include Mode Switcher Widget.")
    }

    !include($$PWD/QUrgLib/QUrgLib.pri) {
            error("Unable to include QUrg Library.")
    }

    !include($$PWD/RangeViewWidget/RangeViewWidget.pri) {
            error("Unable to include Range View Widget.")
    }

    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
            $$PWD/FileConnectorPlugin.cpp \
            $$PWD/FileConnectionCheck.cpp \
            $$PWD/FileDataConverter.cpp

    HEADERS  += \
            $$PWD/FileConnectorPlugin.h \
            $$PWD/FileConnectionCheck.h \
            $$PWD/FileDataConverter.h

    FORMS    += \
            $$PWD/FileConnectorPlugin.ui

    RESOURCES += \
            $$PWD/FileConnectorPlugin.qrc

    TRANSLATIONS = $$PWD/i18n/plugin_fr.ts \
            $$PWD/i18n/plugin_en.ts \
            $$PWD/i18n/plugin_ja.ts
}
