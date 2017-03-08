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

#include "FileConnectorPlugin.h"
#include "ui_FileConnectorPlugin.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QTime>
#include <QShortcut>
#include <QDebug>
#include <QStandardPaths>
#include <QToolTip>

#include "PluginUtils.h"

#include <iostream>
using namespace std;

#include "delay.h"

//#include "SettingsWindow.h"
#include "FileDataConverter.h"
const QString LogHeader = "FileConnectorPlugin";

FileConnectorPlugin::FileConnectorPlugin(QWidget* parent)
    : ConnectorPluginInterface(parent)
    , ui(new Ui::FileConnectorPlugin)
    , m_connected(false)
    , m_recorder(Q_NULLPTR)
    , m_sensorInfo(Q_NULLPTR)
    , m_receiving_thread(new Thread(&receivingThreadProcess, this))
    , m_sensor(new UrgDevice)
{
    ui->setupUi(this);

    ui->moreButton->setVisible(false);

    setupConnections();
    setupShortcuts();

    ui->loadActivityIndicator->hide();
    ui->loadActivityIndicator->setValue(0);
    m_range_view_widget = ui->rangeViewWidget;

    ui->moreButton->setEnabled(false);

    ui->recorder->hide();

    m_paused = false;
    m_connected = false;
    m_started = false;

    m_sliderPaused = false;

    m_readPosition = 0;
    m_speedFactor = 1.0;
    m_logTime = "";

    resetUi();
}

void FileConnectorPlugin::addDebugInformation(const QString &info)
{
    //    new QListWidgetItem(info, ui->debugList);
}

void FileConnectorPlugin::addRecorder(PluginManagerInterface *manager)
{
    if(manager){
        GeneralPluginInterface *recorderInterface = manager->makePlugin("org.kristou.UrgBenri.LogRecorderHelperPlugin", this);
        if(recorderInterface){
            m_recorder = qobject_cast<HelperPluginInterface *>(recorderInterface);
            if(m_recorder){
                if(m_recorder->hasFunction("setDeviceMethod")
                        && m_recorder->hasFunction("addSensorDataMethod")
                        && m_recorder->hasFunction("stopMethod")){
                    m_recorder->callFunction("setDeviceMethod", QVariantList() << qVariantFromValue((void *)m_sensor));
                    ui->recorder->layout()->addWidget(m_recorder);
                    m_recorder->setEnabled(false);
                    ui->recorder->setVisible(true);
                }
                else{
                    delete m_recorder;
                    m_recorder = Q_NULLPTR;
                }
            }
        }
    }
}

void FileConnectorPlugin::addSensorInformation(PluginManagerInterface *manager)
{
    if(manager){
        GeneralPluginInterface *pluginInterface = manager->makePlugin("org.kristou.UrgBenri.SensorInformationHelperPlugin", Q_NULLPTR);
        if(pluginInterface){
            m_sensorInfo = qobject_cast<HelperPluginInterface *>(pluginInterface);
            if(m_sensorInfo){
                if(m_sensorInfo->hasFunction("setDeviceMethod")
                        && m_sensorInfo->hasFunction("noReloadMethod")){
                    m_sensorInfo->callFunction("setDeviceMethod", qVariantFromValue((void *)m_sensor));
                    connect(ui->moreButton, &QPushButton::clicked,
                            this, [&](){
                        if (!m_connection_widget.isOpen()) {
                            emit information(LogHeader,
                                             tr("The sensor is not connected."));
                            return;
                        }
                        if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)){
                            QPoint pos = ui->moreButton->mapToGlobal(ui->moreButton->rect().bottomLeft());
                            pos.ry() -= 16;
                            QToolTip::showText(pos, m_connection_widget.headerRecords().join("\n")
                                               , ui->moreButton, ui->moreButton->rect());
                        }
                        else{
                            m_sensorInfo->callFunction("noReloadMethod", !m_started);
                            showPluginModal(m_sensorInfo, this);
                        }
                    });
                    ui->moreButton->setIcon(pluginInterface->pluginIcon());
                    ui->moreButton->setVisible(true);
                }
                else{
                    delete m_sensorInfo;
                    m_sensorInfo = Q_NULLPTR;
                }
            }
        }
    }
}

void FileConnectorPlugin::setupShortcuts()
{
    (void) new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this, SLOT(forwardButtonPressed()));
    (void) new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this, SLOT(backButtonPressed()));
}

void FileConnectorPlugin::setupConnections()
{
    connect(ui->connectButton, &QAbstractButton::clicked,
            this, &FileConnectorPlugin::connectPressed);
    connect(ui->disconnectButton, &QAbstractButton::clicked,
            this, &FileConnectorPlugin::disconnectPressed);

    connect(&m_connectionCheck, &FileConnectionCheck::connected,
            this, &FileConnectorPlugin::deviceConnected);
    connect(&m_connectionCheck, &FileConnectionCheck::connectionFailed,
            this, &FileConnectorPlugin::deviceConnectFailed);

    connect(&m_connection_widget, &UrgLogHandler::initProgress,
            this, &FileConnectorPlugin::logFileInitProgress);

    connect(ui->captureMode, &ModeSwitcherWidget::captureModeChanged,
            this, &FileConnectorPlugin::restart);

    connect(ui->startStep, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &FileConnectorPlugin::startStepChanged);
    connect(ui->endStep, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &FileConnectorPlugin::endStepChanged);


    connect(ui->forwardButton, &QAbstractButton::clicked,
            this, &FileConnectorPlugin::forwardButtonPressed);
    connect(ui->backButton, &QAbstractButton::clicked,
            this, &FileConnectorPlugin::backButtonPressed);

    connect(ui->currentPosition, &QAbstractSlider::sliderMoved,
            this, &FileConnectorPlugin::sliderChanged);
    connect(ui->currentPosition, &QAbstractSlider::sliderPressed,
            this, &FileConnectorPlugin::sliderPressed);
    connect(ui->currentPosition, &QAbstractSlider::sliderReleased,
            this, &FileConnectorPlugin::sliderReleased);

    connect(ui->currentPositionBox, &QAbstractSpinBox::editingFinished,
            this, &FileConnectorPlugin::spingChanged);
    connect(ui->currentSpeed, &QAbstractSlider::valueChanged,
            this, &FileConnectorPlugin::speedChanged);
    connect(ui->resetSpeedButton, &QAbstractButton::clicked,
            this, &FileConnectorPlugin::resetSpeedButtonClicked);

    connect(m_receiving_thread, &Thread::finished,
            this, &FileConnectorPlugin::receivingThreadFinished);
}

FileConnectorPlugin::~FileConnectorPlugin()
{
    //    qDebug() << "FileCommunicatorWidget::~FileCommunicatorWidget";

    if(m_receiving_thread){
        if(m_receiving_thread->isRunning()){
            m_receiving_thread->stop();
        }
        delete m_receiving_thread;
    }
    m_connection_widget.close();

    if(m_sensor) delete m_sensor;

    m_connected = false;

    delete ui;
}

void FileConnectorPlugin::start()
{
    qDebug() << "FileCommunicatorWidget::start";

    if (!m_connected) {
        return;
    }

    if (m_receiving_thread->isRunning()) {
        return;
    }

    if (! m_paused) {
        //        QString filename = ui->filenameLine->text();
        if (m_connection_widget.isOpen()) {
            if (!m_connection_widget.getDataInit()) {
                emit warning(QApplication::applicationName(),
                             m_connection_widget.what());
            }

            m_connection_widget.setReadStartStep(ui->startStep->value());
            m_connection_widget.setReadEndStep(ui->endStep->value());
            m_sensor->setCaptureRange(ui->startStep->value(), ui->endStep->value());

            setCaptureMode();
            updatePlayControls(m_connection_widget.getTotalTimestamps() - 1);

            ui->loadActivityIndicator->show();
        }
    }
    else {
        updatePlayControls(m_connection_widget.getTotalTimestamps() - 1);
        ui->loadActivityIndicator->show();
    }
    m_paused = false;
    m_started = true;
    m_receiving_thread->run();
    emit started();
}

void FileConnectorPlugin::stop()
{
    addDebugInformation(tr("Stopping acquisition if active"));
    if (!m_connected) {
        return;
    }

    m_receiving_thread->stop();

    if(m_recorder) m_recorder->callFunction("stopMethod");

    ui->loadActivityIndicator->hide();
    ui->loadActivityIndicator->setValue(0);
    ui->currentTimeLabel->setText("");
    ui->currentLogTimeLabel->setText("");

    resetPlayControls();

    m_paused = false;
    m_started = false;
    emit stopped();
}

void FileConnectorPlugin::resetPlayControls()
{

    ui->currentPosition->setMinimum(0);
    ui->currentPositionBox->setMinimum(0);
    ui->currentPosition->setMaximum(0);
    ui->currentPositionBox->setMaximum(0);
    ui->currentPosition->setValue(0);
    ui->currentPositionBox->setValue(0);

    ui->currentSpeed->setValue(0);
    ui->currentTimeLabel->setText("");
    ui->currentLogTimeLabel->setText("");
    ui->rateLabel->setText("1:1");
}

void FileConnectorPlugin::pause()
{
    if (!m_connected) {
        return;
    }

    if (! m_receiving_thread->isRunning()) {
        return;
    }
    m_receiving_thread->stop();

    m_paused = true;
    m_started = false;

    emit paused();
}

bool FileConnectorPlugin::loadFile(const QString &filename)
{
    disconnectPressed();
    //    ui->loadActivityIndicator->setStyleSheet("QProgressBar::chunk {background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 red, stop: 1 white); }");
    ui->loadActivityIndicator->setInvertedAppearance(true);
    ui->loadActivityIndicator->setValue(0);
    ui->loadActivityIndicator->show();

    ui->connectButton->setEnabled(false);
    ui->disconnectButton->setEnabled(true);

    m_connectionCheck.setHandler(&m_connection_widget);
    m_connectionCheck.setFilename(filename);
    m_connectionCheck.check();

    return true;
}

int FileConnectorPlugin::receivingThreadProcess(void* args)
{
    qDebug() << "FileCommunicatorWidget::receivingThreadProcess";

    FileConnectorPlugin* obj = static_cast<FileConnectorPlugin*>(args);

    //    qDebug() << "FileCommunicatorWidget thread started";

    QTime timer;
    long lastTimestamp = 0;
    SensorDataArray l_ranges;
    SensorDataArray l_levels;

    while (!obj->m_receiving_thread->exitThread) {
        if (obj->m_connection_widget.isOpen()) {
            timer.start();

            long l_timeStamp;
            long playPosition;
            l_ranges.steps.clear();
            l_levels.steps.clear();
            if ((playPosition = obj->m_connection_widget.getData(l_ranges, l_levels, l_timeStamp)) < 0) {
                qDebug() << "Error: " << obj->m_connection_widget.what();
                emit obj->error(LogHeader, obj->m_connection_widget.what());
                break;
            }

            if (!l_ranges.steps.isEmpty()) {
                QString l_logTime = obj->m_connection_widget.getLogTime();
                obj->setLengthData(l_ranges, l_levels, l_timeStamp, playPosition, l_logTime);

                int elapsedTime = timer.elapsed();

                if (lastTimestamp == 0) {
                    lastTimestamp = l_timeStamp;
                }

                // TODO: fix this (l_timeStamp - lastTimestamp), obj->connection_widget.getScanMsec()
                long timeLapse = l_timeStamp - lastTimestamp;
                if (timeLapse < 0 || timeLapse > 1000) {
                    timeLapse = obj->m_connection_widget.getScanMsec();
                }
                int diffTime  = static_cast<int>((double)(timeLapse) * obj->m_speedFactor);
                //                qDebug() << "Delay: " << diffTime << "l_timeStamp: " << l_timeStamp << "lastTimestamp" << lastTimestamp << "speedFactor" << obj->speedFactor;
                diffTime = diffTime - elapsedTime;
                if (diffTime < (elapsedTime + 10)) {
                    diffTime = elapsedTime + 10;
                }
                //                qDebug() << "Delay: " << diffTime << "Elapsed: " << elapsedTime;
                delay(diffTime);
                lastTimestamp = l_timeStamp;
            }
        }
    }

    //    qDebug() << "FileCommunicatorWidget thread finished";
    return 0;
}

void FileConnectorPlugin::setLengthData(const SensorDataArray &l_ranges, const SensorDataArray &l_levels,
                                        long l_timeStamp, long l_position, const QString &l_logTime)
{
    if(m_recorder) m_recorder->callFunction("addSensorDataMethod", QVariantList() << qVariantFromValue((void *)&l_ranges)
                                            << qVariantFromValue((void *)&l_levels)
                                            << qVariantFromValue((void *)&l_timeStamp));

    PluginDataStructure data;
    data.ranges = l_ranges.steps;
    data.levels = l_levels.steps;
    data.timestamp = l_timeStamp;
    data.converter = QSharedPointer<DataConverterInterface>(new FileDataConverter(l_ranges.converter));
    emit measurementDataReady(data);

    m_readPosition = l_position;
    m_logTime = l_logTime;

    QMetaObject::invokeMethod(this, "updatePlayer");

}

void FileConnectorPlugin::saveState(QSettings &settings)
{
    settings.setValue("capture_mode", ui->captureMode->state());

    if(m_recorder){
        settings.beginGroup(m_recorder->metaObject()->className());
        m_recorder->saveState(settings);
        settings.endGroup();
    }
    if(m_sensorInfo){
        settings.beginGroup(m_sensorInfo->metaObject()->className());
        m_sensorInfo->saveState(settings);
        settings.endGroup();
    }
}

void FileConnectorPlugin::restoreState(QSettings &settings)
{
    ui->captureMode->setState(settings.value("capture_mode", "").toString());

    if(m_recorder){
        settings.beginGroup(m_recorder->metaObject()->className());
        m_recorder->restoreState(settings);
        settings.endGroup();
    }
    if(m_sensorInfo){
        settings.beginGroup(m_sensorInfo->metaObject()->className());
        m_sensorInfo->restoreState(settings);
        settings.endGroup();
    }
}

void FileConnectorPlugin::loadTranslator(const QString &locale)
{
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(QString("plugin.%1").arg(locale), ":/FileConnectorPlugin")) {
        qApp->installTranslator(&m_translator);
    }

    if(m_recorder) m_recorder->loadTranslator(locale);
    if(m_sensorInfo) m_sensorInfo->loadTranslator(locale);
}

void FileConnectorPlugin::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            if(ui) ui->retranslateUi(this);
            emit languageChanged();
            break;
        default:
            break;
    }
}


void FileConnectorPlugin::updateCaptureMode(RangeCaptureMode mode)
{
    ui->captureMode->setCaptureMode(mode);
}

void FileConnectorPlugin::connectPressed()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open log file"),
                QDir::currentPath(),
                tr("Log file (*.ubh)")
                );
    if (!filename.isNull()) {
        QFileInfo fi(filename);
        QDir::setCurrent(fi.absolutePath());

        loadFile(filename);

        emit fileOpened(filename);
    }
}

void FileConnectorPlugin::disconnectPressed()
{
    m_connection_widget.stopInit();
    m_connectionCheck.stop();
    stop();

    m_connected = false;
    m_started = false;
    m_paused = false;

    resetUi();
    ui->filenameLine->setText("");

    m_connection_widget.close();

    if(m_recorder) m_recorder->setEnabled(false);

    ui->moreButton->setEnabled(m_connected);

    ui->connectButton->setEnabled(!m_connected);
    ui->disconnectButton->setEnabled(m_connected);

    emit information(LogHeader,
                     tr("Log file unloaded."));

    emit connexionLost();
}

void FileConnectorPlugin::deviceConnected()
{
    m_connected = true;
    ui->loadActivityIndicator->setInvertedAppearance(false);

    RangeSensorParameter parameter = m_connection_widget.getRangeSensorParameter();
    m_sensor->setParameter(parameter);
    m_sensor->setCaptureGroupSteps(m_connection_widget.getGrouping());

    updateCaptureMode(m_connection_widget.getCaptureMode());
    if (m_connection_widget.getTotalTimestamps() > 0) {
        updatePlayControls(m_connection_widget.getTotalTimestamps() - 1);
    }
    else {
        updatePlayControls(0);
        emit warning(QApplication::applicationName(),
                     tr("The file is empty!"));
    }

    updateUiWithInfo();

    setCaptureMode();

    ui->filenameLine->setText(m_connection_widget.getFileName());

    if(m_recorder) m_recorder->setEnabled(true);

    ui->moreButton->setEnabled(m_connected);

    ui->connectButton->setEnabled(!m_connected);
    ui->disconnectButton->setEnabled(m_connected);


    ui->loadActivityIndicator->hide();
    ui->loadActivityIndicator->setValue(0);
    ui->loadActivityIndicator->setInvertedAppearance(false);

    emit information(LogHeader,
                     tr("Log file loaded."));

    //    if (SettingsWindow::getSetting("scanAutostart", true)) {
    start();
    //    }

    emit connexionReady();
}

void FileConnectorPlugin::deviceConnectFailed()
{
    ui->loadActivityIndicator->hide();
    ui->loadActivityIndicator->setValue(0);
    ui->loadActivityIndicator->setInvertedAppearance(false);
    disconnectPressed();

    emit warning(QApplication::applicationName(),
                 m_sensor->what());

    emit connexionLost();
}

void FileConnectorPlugin::updateUiWithInfo()
{

    RangeSensorParameter logParam = m_connection_widget.getRangeSensorParameter();

    ui->modelLine->setText(m_connection_widget.getModel());
    ui->serialLine->setText(m_connection_widget.getSerialNumber());
    //    ui->ScansLine->setText(QString::number(m_connection_widget.getScanMsec()) + "ms");

    ui->startStep->setMinimum(m_connection_widget.getStartStep());
    ui->startStep->setMaximum(m_connection_widget.getEndStep());
    ui->startStep->setValue(m_connection_widget.getStartStep());

    ui->endStep->setMinimum(m_connection_widget.getStartStep());
    ui->endStep->setMaximum(m_connection_widget.getEndStep());
    ui->endStep->setValue(m_connection_widget.getEndStep());

    m_range_view_widget->setParameters(logParam.area_front, logParam.area_total);
    m_range_view_widget->setRange(m_connection_widget.getStartStep(),
                                  m_connection_widget.getEndStep());

    ui->captureMode->setSupportedModes(m_connection_widget.supportedModes());
}


void FileConnectorPlugin::resetUi()
{
    ui->modelLine->setText("");
    ui->serialLine->setText("");
    //    ui->ScansLine->setText("");
    ui->currentTimeLabel->setText("");
    ui->currentLogTimeLabel->setText("");
    ui->rateLabel->setText("1:1");

    ui->startStep->setValue(0);
    ui->startStep->setMinimum(0);
    ui->startStep->setMaximum(0);
    ui->endStep->setValue(0);
    ui->endStep->setMinimum(0);
    ui->endStep->setMaximum(0);

    resetPlayControls();

    m_range_view_widget->unsetParameters();

//    ui->captureMode->reset();

    ui->extraGroup->setVisible(ui->extraGroup->layout()->count() > 0);
}

void FileConnectorPlugin::updatePlayControls(long totalScans)
{
    ui->currentPosition->setMinimum(0);
    ui->currentPositionBox->setMinimum(0);
    ui->currentPosition->setMaximum(totalScans);
    ui->currentPositionBox->setMaximum(totalScans);
}

void FileConnectorPlugin::setCaptureMode()
{
    RangeCaptureMode captureMode = ui->captureMode->captureMode();
    m_sensor->setCaptureMode(captureMode);
    m_connection_widget.setCaptureMode(captureMode);
}

void FileConnectorPlugin::startStepChanged(int value)
{
    ui->endStep->setMinimum(value);

    m_connection_widget.setReadStartStep(ui->startStep->value());
    m_connection_widget.setReadEndStep(ui->endStep->value());

    m_range_view_widget->setRange(ui->startStep->value(), ui->endStep->value());

    restart();
}


void FileConnectorPlugin::endStepChanged(int value)
{
    ui->startStep->setMaximum(value);

    m_connection_widget.setReadStartStep(ui->startStep->value());
    m_connection_widget.setReadEndStep(ui->endStep->value());

    m_range_view_widget->setRange(ui->startStep->value(), ui->endStep->value());

    restart();
}


void FileConnectorPlugin::receivingTimerProcess()
{
    if (m_connection_widget.isOpen()) {
        SensorDataArray l_ranges;
        SensorDataArray l_levels;
        long l_timeStamp;
        long playPosition;
        if ((playPosition = m_connection_widget.getData(l_ranges, l_levels, l_timeStamp)) < 0) {
            //            fprintf(stderr, "ERROR: %s\n", connection_widget.what());
            //            cerr << "Error: " << connection_widget.what().toStdString() << endl;
            //            stop();
            return;
        }

        if (l_ranges.steps.isEmpty()) {
            //            fprintf(stderr, "ERROR: %s\n", connection_widget.what());
            //            cerr << "Error: " << connection_widget.what().toStdString() << endl;
            //            stop();
            return;
        }

        QString l_logTime = m_connection_widget.getLogTime();

        setLengthData(l_ranges, l_levels, l_timeStamp, playPosition, l_logTime);
    }
}

void FileConnectorPlugin::sliderChanged(int pos)
{
    if (m_paused) {
        m_connection_widget.setDataPos(pos);
        receivingTimerProcess();
    }
}

void FileConnectorPlugin::spingChanged()
{
    if (m_connection_widget.isOpen()) {
        int pos = ui->currentPositionBox->value();
        m_connection_widget.setDataPos(pos);
        receivingTimerProcess();
    }
}


void FileConnectorPlugin::speedChanged(int pos)
{
    if (pos >= 0) {
        m_speedFactor = 1.0 - ((double)pos / (ui->currentSpeed->maximum() + 1));
        ui->rateLabel->setText(QString("1:%1").arg(qAbs(pos + 1)));
    }
    else {
        pos = pos * -1;
        m_speedFactor = pos;
        ui->rateLabel->setText(QString("%1:1").arg(qAbs(pos)));
    }

}

void FileConnectorPlugin::resetSpeedButtonClicked()
{
    ui->currentSpeed->setValue(0);
}

void FileConnectorPlugin::forwardButtonPressed()
{
    int pos = ui->currentPosition->value() + 1;
    ui->currentPosition->setValue(pos);
    ui->currentPositionBox->setValue(pos);
    if (m_connection_widget.isOpen()) {
        m_connection_widget.setDataPos(pos);
        receivingTimerProcess();
    }
}

void FileConnectorPlugin::backButtonPressed()
{
    int pos = ui->currentPosition->value() - 1;
    ui->currentPosition->setValue(pos);
    ui->currentPositionBox->setValue(pos);
    if (m_connection_widget.isOpen()) {
        m_connection_widget.setDataPos(pos);
        receivingTimerProcess();
    }
}

void FileConnectorPlugin::logFileInitProgress(int progress)
{
    //    ui->loadActivityIndicator->setMinimum(0);
    //    ui->loadActivityIndicator->setMaximum(100);
    ui->loadActivityIndicator->setValue(progress);
}

void FileConnectorPlugin::updatePlayer()
{
    int playProgress = static_cast<int>((double)m_readPosition / ((double)m_connection_widget.getTotalTimestamps() - 1) * 100.0);
    ui->loadActivityIndicator->setValue(playProgress);
    ui->currentPosition->setValue(m_readPosition);
    ui->currentPositionBox->setValue(m_readPosition);

    QStringList l_logTime = m_logTime.split(':');
    QString logT = "";
    if (l_logTime.size() == 7) {
        logT = l_logTime[0] + "/" + l_logTime[1] + "/" + l_logTime[2] + ", ";
        logT += l_logTime[3] + ":" + l_logTime[4] + ":" + l_logTime[5] + "," + l_logTime[6];
    }
    else {
        logT = m_logTime;
    }
    ui->currentLogTimeLabel->setText(logT);

    long totalTime = m_readPosition * m_connection_widget.getScanMsec();
    QString timeSpan = timeConversion(totalTime);
    ui->currentTimeLabel->setText(timeSpan);
}

QString FileConnectorPlugin::timeConversion(long msecs)
{
    QString formattedTime;

    long hours = msecs / (1000 * 60 * 60);
    long minutes = (msecs - (hours * 1000 * 60 * 60)) / (1000 * 60);
    long seconds = (msecs - (minutes * 1000 * 60) - (hours * 1000 * 60 * 60)) / 1000;
    long milliseconds = msecs - (seconds * 1000) - (minutes * 1000 * 60) - (hours * 1000 * 60 * 60);

    formattedTime.append(QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(seconds, 2, 10, QLatin1Char('0')) + ":" +
                         QString("%1").arg(milliseconds, 3, 10, QLatin1Char('0')));

    return formattedTime;
}

void FileConnectorPlugin::sliderPressed()
{
    if (m_started) {
        pause();
        m_sliderPaused = true;
    }
}

void FileConnectorPlugin::sliderReleased()
{
    if (m_sliderPaused) {
        start();
    }

}

void FileConnectorPlugin::receivingThreadFinished()
{
    emit information(LogHeader,
                     tr("Data acquisition finished."));
}

void  FileConnectorPlugin::restart()
{
    if (m_started) {
        stop();
        start();
    }

    if (m_paused) {
        stop();
        start();
        pause();
    }
}

void FileConnectorPlugin::onLoad(PluginManagerInterface *manager)
{
    addRecorder(manager);
    addSensorInformation(manager);

    resetUi();
}

void FileConnectorPlugin::onUnload()
{

}

