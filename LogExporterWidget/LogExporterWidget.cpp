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

#include "LogExporterWidget.h"
#include "ui_LogExporterWidget.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDateTime>
#include <QDebug>

#include "log_printf.h"

LogExporterWidget::LogExporterWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LogExporterWidget),
    urg(new UrgDevice)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    ui->loadingIncator->hide();
    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    totalTimestamps = 0;
    range_view_widget = ui->rangeViewWidget;


    distanceNumber = 3;
    intensityNumber = 3;

    stop = true;
    connect(ui->loadInputButton, SIGNAL(clicked()),
            this, SLOT(loadInputButtonClicked()));
    connect(ui->loadOutputButton, SIGNAL(clicked()),
            this, SLOT(loadOutputButtonClicked()));
    connect(ui->exportButton, SIGNAL(clicked()),
            this, SLOT(exportButtonClicked()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    connect(ui->startStep, SIGNAL(valueChanged(int)),
            this, SLOT(startStepChanged(int)));
    connect(ui->endStep, SIGNAL(valueChanged(int)),
            this, SLOT(endStepChanged(int)));
    connect(ui->startScan, SIGNAL(valueChanged(int)),
            this, SLOT(startScanChanged(int)));
    connect(ui->endScan, SIGNAL(valueChanged(int)),
            this, SLOT(endScanChanged(int)));
}

LogExporterWidget::~LogExporterWidget()
{
    qDebug() << "LogExporterWidget::~LogExporterWidget";
    delete ui;
}

void LogExporterWidget::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LogExporterWidget::loadInputButtonClicked()
{
    ui->exportButton->setEnabled(false);
    if (connection_widget.isOpen()) {
        connection_widget.close();
    }
    ui->filenameLine->setText("");
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open log file"),
                QDir::currentPath(),
                tr("Log file (*.ubh)"));
    if (!filename.isNull()) {
        QFileInfo fileInfo(filename);
        QDir::setCurrent(fileInfo.absolutePath());

        if (!fileInfo.exists()) {
            QMessageBox::warning(this, tr("Connection error"),
                                 tr("File does not exist."));
            return;
        }

        connection_widget.close();

        if (connection_widget.load(filename)) {
            ui->loadingIncator->show();
            //            connection_widget.timestampSequential(totalTimestamps, skipTimestamps);
            //            connection_widget.getDataInit();
            connection_widget.init();
            totalTimestamps = connection_widget.getTotalTimestamps();
            RangeSensorParameter parameter = connection_widget.getRangeSensorParameter();
            urg->setParameter(parameter);

            updateDistanceIntensityNumbers();
            updateUiWithInfo();

            ui->filenameLine->setText(filename);
            if (!ui->filenameLine->text().isEmpty() &&
                    !ui->recordFilenameLine->text().isEmpty()) {
                ui->exportButton->setEnabled(true);
            }
            ui->loadingIncator->hide();
        }
    }

}

void LogExporterWidget::updateDistanceIntensityNumbers()
{
    RangeCaptureMode mode = connection_widget.getCaptureMode();
    switch (mode) {
    case GD_Capture_mode:
    case MD_Capture_mode:{
        distanceNumber = 1;
        intensityNumber = 0;
    }break;
    case GE_Capture_mode:
    case ME_Capture_mode:{
        distanceNumber = 1;
        intensityNumber = 1;
    }break;
    case HD_Capture_mode:
    case ND_Capture_mode:{
        distanceNumber = 3;
        intensityNumber = 0;
    }break;
    case HE_Capture_mode:
    case NE_Capture_mode:{
        distanceNumber = 3;
        intensityNumber = 3;
    }break;
    default:{
        distanceNumber = 0;
        intensityNumber = 0;
    }break;
    }
}

void LogExporterWidget::loadOutputButtonClicked()
{
    ui->exportButton->setEnabled(false);
    ui->recordFilenameLine->setText("");
    QString defaulName = QString("/") +
                QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss_zzz") +
                ".csv";
    QString filename = QFileDialog::getSaveFileName(
                this,
                tr("Save CSV"),
                QDir::currentPath() + defaulName,
                tr("CSV file (*.csv)"));
    if (!filename.isNull()) {
        QFileInfo fi(filename);
        QDir::setCurrent(fi.absolutePath());

        ui->recordFilenameLine->setText(filename);
        if (!ui->filenameLine->text().isEmpty() &&
                !ui->recordFilenameLine->text().isEmpty()) {
            ui->exportButton->setEnabled(true);
        }
    }
}

void LogExporterWidget::printScanNumber(const ScanData &toSave, QTextStream &out)
{
    for (int j = 0; j < toSave.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << "scan_" << j << ",";
        }
    }
    out << "\n";
}

void LogExporterWidget::printTimestamps(const QVector<long> &timestamps, QTextStream &out)
{
    for (int j = 0; j < timestamps.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << "timestamp_" << j << ",";
        }
    }
    out << "\n";

    for (int j = 0; j < timestamps.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << timestamps[j] << ",";
        }
    }
    out << "\n";
}

void LogExporterWidget::printLogTimes(const QVector<QString> &logTimes, QTextStream &out)
{
    for (int j = 0; j < logTimes.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << "logTime_" << j << ",";
        }
    }
    out << "\n";

    for (int j = 0; j < logTimes.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << logTimes[j] << ",";
        }
    }
    out << "\n";
}

void LogExporterWidget::printScanHeader(const ScanData &toSave, QTextStream &out)
{
    for (int j = 0; j < toSave.size(); j++) {
        for (int k = 0; k < distanceNumber; k++) {
            out << "distance_" << k << ",";
        }
        for (int k = 0; k < intensityNumber; k++) {
            out << "intensity_" << k << ",";
        }
    }
    out << "\n";
}

void LogExporterWidget::exportButtonClicked()
{
    ui->exportButton->setEnabled(false);
    ui->inputGroup->setEnabled(false);
    ui->outputGroup->setEnabled(false);
    ui->scanGroup->setEnabled(false);
    ui->stepGroup->setEnabled(false);

    ui->exportProgress->show();

    connection_widget.setReadStartStep(ui->startStep->value());
    connection_widget.setReadEndStep(ui->endStep->value());

    connection_widget.setDataPos(ui->startScan->value());
    long currentScan = ui->startScan->value();
    stop = false;

    ScanData toDataSave;
    ScanData toIntensitySave;
    QVector<long> timestamps;
    QVector<QString> logTimes;

    while ((currentScan <= ui->endScan->value()) && !stop) {
        QCoreApplication::processEvents();
        SensorDataArray ranges;
        SensorDataArray levels;
        long timestamp;
        connection_widget.getData(ranges, levels, timestamp);
        timestamps.push_back(timestamp);
        logTimes.push_back(connection_widget.getLogTime());

        toDataSave.push_back(ranges);
        toIntensitySave.push_back(levels);
        currentScan++;
    }

    //    connection_widget.close();

    ui->exportProgress->setValue(50);


    int stepCnt = (ui->endStep->value() - ui->startStep->value()) + 1;
    //int multiScanCnt = ui->multiPartSpinBox->value();
    //    if(!ui->multiPartCheckBox->isChecked()){
    //        multiScanCnt = toSave.size();
    //    }

    QString recordFilename = ui->recordFilenameLine->text();
    QFile file(recordFilename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    printScanNumber(toDataSave, out);

    printTimestamps(timestamps, out);

    printLogTimes(logTimes, out);

    printScanHeader(toDataSave, out);

    ui->exportProgress->setValue(75);

    for (int step = 0; step < stepCnt; step++) {
        for (int scan = 0; scan < toDataSave.size(); scan++) {
            QCoreApplication::processEvents();
            for (int echo = 0; echo < distanceNumber; echo++) {
                if(echo < toDataSave[scan].steps[step].size()){
                    out << toDataSave[scan].steps[step][echo] << ",";
                }else{
                    out << "0,";
                }
            }
            for (int echo = 0; echo < intensityNumber; echo++) {
                if(echo < toIntensitySave[scan].steps[step].size()){
                    out << toIntensitySave[scan].steps[step][echo] << ",";
                }else{
                    out << "0,";
                }
            }
        }
        out << "\n";
    }

    ui->exportProgress->setValue(100);

    file.close();
    QMessageBox::information(this, tr("Export information"),
                             tr("Log file export finished."));
    ui->exportProgress->hide();
    ui->exportButton->setEnabled(true);
    ui->inputGroup->setEnabled(true);
    ui->outputGroup->setEnabled(true);
    ui->scanGroup->setEnabled(true);
    ui->stepGroup->setEnabled(true);
}

void LogExporterWidget::updateUiWithInfo()
{
    RangeSensorParameter logParam = connection_widget.getRangeSensorParameter();


    ui->startStep->setMinimum(connection_widget.getStartStep());
    ui->startStep->setMaximum(connection_widget.getEndStep());
    ui->startStep->setValue(connection_widget.getStartStep());

    ui->endStep->setMinimum(connection_widget.getStartStep());
    ui->endStep->setMaximum(connection_widget.getEndStep());
    ui->endStep->setValue(connection_widget.getEndStep());

    range_view_widget->setParameters(logParam.area_front, logParam.area_total);
    range_view_widget->setRange(connection_widget.getStartStep(),
                                connection_widget.getEndStep());

    ui->startScan->setMinimum(0);
    ui->startScan->setMaximum(totalTimestamps);
    ui->startScan->setValue(0);

    ui->endScan->setMinimum(0);
    ui->endScan->setMaximum(totalTimestamps - 1);
    ui->endScan->setValue(totalTimestamps - 1);
}

void LogExporterWidget::startStepChanged(int value)
{
    ui->endStep->setMinimum(value);
    range_view_widget->setRange(ui->startStep->value(), ui->endStep->value());
}

void LogExporterWidget::endStepChanged(int value)
{
    ui->startStep->setMaximum(value);
    range_view_widget->setRange(ui->startStep->value(), ui->endStep->value());
}

void LogExporterWidget::startScanChanged(int value)
{
    ui->endScan->setMinimum(value);
}

void LogExporterWidget::endScanChanged(int value)
{
    ui->startScan->setMaximum(value);
}

void LogExporterWidget::closeButtonClicked()
{
    stop = true;
    hide();
}

void LogExporterWidget::initialize()
{
    ui->loadingIncator->hide();
    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    totalTimestamps = 0;
    range_view_widget = ui->rangeViewWidget;

    stop = true;

    connection_widget.close();
    ui->filenameLine->setText("");
    ui->recordFilenameLine->setText("");
    ui->loadingIncator->hide();
    ui->exportProgress->hide();

    ui->exportButton->setEnabled(false);
    ui->inputGroup->setEnabled(true);
    ui->outputGroup->setEnabled(true);
    ui->scanGroup->setEnabled(true);
    ui->stepGroup->setEnabled(true);
    show();
}

