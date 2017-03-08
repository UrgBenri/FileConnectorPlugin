/*
	This file is part of the UrgBenri application.

	Copyright (c) 2016 Mehrez Kristou.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Please contact kristou@hokuyo-aut.jp for more details.

*/

#ifndef FILECOMMUNICATORWIDGET_H
#define FILECOMMUNICATORWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QTranslator>

#include "ConnectorPluginInterface.h"
#include "HelperPluginInterface.h"

#include "RangeViewWidget.h"
#include "UrgDevice.h"

#include "FileConnectionCheck.h"

#include "UrgLogHandler.h"
#include "Thread.h"

using namespace std;
using namespace qrk;

namespace Ui
{
class FileConnectorPlugin;
}

class FileConnectorPlugin : public ConnectorPluginInterface
{
    Q_OBJECT    
    Q_INTERFACES(ConnectorPluginInterface)
    Q_PLUGIN_METADATA(IID "org.kristou.UrgBenri.FileConnectorPlugin")
    PLUGIN_FACTORY(FileConnectorPlugin)
public:
    explicit FileConnectorPlugin(QWidget* parent = 0);
    virtual ~FileConnectorPlugin();

    bool isConnected() { return m_connected;}
    bool isStarted() { return m_started;}
    bool isPaused() { return m_paused;}
    bool loadFile(const QString &filename);

    static int receivingThreadProcess(void* args);
    void setLengthData(const SensorDataArray &l_ranges
                       , const SensorDataArray &l_levels
                       , long l_timeStamp
                       , long l_position
                       , const QString &l_logTime);

    QString pluginName() const{return tr("File");}
    QIcon pluginIcon() const {return QIcon(":/FileConnectorPlugin/tabIcon");}
    QString pluginDescription() const {return tr("Replay UBH files");}
    PluginVersion pluginVersion() const {return PluginVersion(1, 0, 0);}
    QString pluginAuthorName() const {return "Mehrez Kristou";}
    QString pluginAuthorContact() const {return "mehrez@kristou.com";}
    int pluginLoadOrder() const {return 2;}
    bool pluginIsExperimental() const { return false; }

    QString pluginLicense() const { return "GPL"; }
    QString pluginLicenseDescription() const { return "GPL"; }

    void saveState(QSettings &settings);
    void restoreState(QSettings &settings);

    void loadTranslator(const QString &locale);

signals:
    void languageChanged();

public slots:
    void start();
    void stop();
    void pause();
    void restart();

    void onLoad(PluginManagerInterface *manager);
    void onUnload();

protected:
    void changeEvent(QEvent* e);

private:
    void setupConnections();
    void updateUiWithInfo();
    void setCaptureMode();

    void updatePlayControls(long totalScans);

    void setupShortcuts();
    QString timeConversion(long msecs);
    void resetPlayControls();

    Ui::FileConnectorPlugin* ui;

    UrgLogHandler m_connection_widget;
    RangeViewWidget* m_range_view_widget;
    UrgDevice *m_sensor;
    QTranslator m_translator;

    FileConnectionCheck m_connectionCheck;

    HelperPluginInterface *m_sensorInfo;

    Thread *m_receiving_thread;

    HelperPluginInterface *m_recorder;

    bool m_paused;
    bool m_connected;
    bool m_started;

    long m_totalTimestamps;
    long m_skipTimestamps;

    long m_readPosition;
    QString m_logTime;

    double m_speedFactor;

    bool m_sliderPaused;

    void addDebugInformation(const QString &info);
    void addRecorder(PluginManagerInterface *manager);
    void addSensorInformation(PluginManagerInterface *manager);

private slots:
    void connectPressed();
    void disconnectPressed();
    void deviceConnected();
    void deviceConnectFailed();

    void startStepChanged(int value);
    void endStepChanged(int value);

    void receivingTimerProcess();
    void updateCaptureMode(RangeCaptureMode mode);
    void resetUi();

    void sliderChanged(int pos);
    void spingChanged();
    void speedChanged(int pos);
    void resetSpeedButtonClicked();

    void forwardButtonPressed();
    void backButtonPressed();

    void logFileInitProgress(int progress);
    void updatePlayer();

    void sliderPressed();
    void sliderReleased();

    void receivingThreadFinished();
};

#endif // FILECOMMUNICATORWIDGET_H

