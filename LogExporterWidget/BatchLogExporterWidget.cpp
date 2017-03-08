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

#include "BatchLogExporterWidget.h"
#include "ui_BatchLogExporterWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QTextStream>
#include <QDebug>

BatchLogExporterWidget::BatchLogExporterWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BatchLogExporterWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    ui->loadingIncator->hide();

    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    totalTimestamps = 0;
    skipTimestamps = 0;


    stop = true;
    connect(ui->loadInputButton, SIGNAL(clicked()),
            this, SLOT(loadInputButtonClicked()));
    connect(ui->exportButton, SIGNAL(clicked()),
            this, SLOT(exportButtonClicked()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

BatchLogExporterWidget::~BatchLogExporterWidget()
{
        qDebug() << "BatchLogExporterWidget::~BatchLogExporterWidget";
    delete ui;
}

void BatchLogExporterWidget::changeEvent(QEvent* e)
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


void BatchLogExporterWidget::loadInputButtonClicked()
{
    ui->exportButton->setEnabled(false);
    ui->filenameLine->setText("");

    QString directory_path = QFileDialog::getExistingDirectory(
                                 this,
                                 tr("Open UBH files directory"),
                                 QDir::currentPath());
    if (!directory_path.isNull()) {
        QDir::setCurrent(directory_path);

        QFileInfo fileInfo(directory_path);
        if (!fileInfo.exists()) {
            QMessageBox::warning(this, tr("Connection error"),
                                 tr("File does not exist."));
            return;
        }

        ui->loadingIncator->show();
        ui->filenameLine->setText(directory_path);
        ui->loadInputButton->setEnabled(false);

        QDirIterator directory_walker(directory_path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        files.clear();
        while (directory_walker.hasNext()) {
            directory_walker.next();
            if (directory_walker.fileInfo().completeSuffix().toUpper() == "UBH") {
                files.push_back(directory_walker.filePath());
                //                qDebug() << directory_walker.filePath();
            }
            QApplication::processEvents();
        }

        ui->loadingIncator->hide();
        ui->loadInputButton->setEnabled(true);
        ui->exportButton->setEnabled(true);
    }
}
void BatchLogExporterWidget::updateDistanceIntensityNumbers()
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

void BatchLogExporterWidget::printScanNumber(const ScanData &toSave, QTextStream &out)
{
    for (int j = 0; j < toSave.size(); j++) {
        for (int k = 0; k < (distanceNumber + intensityNumber); k++) {
            out << "scan_" << j << ",";
        }
    }
    out << "\n";
}

void BatchLogExporterWidget::printTimestamps(const QVector<long> &timestamps, QTextStream &out)
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

void BatchLogExporterWidget::printLogTimes(const QVector<QString> &logTimes, QTextStream &out)
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

void BatchLogExporterWidget::printScanHeader(const ScanData &toSave, QTextStream &out)
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

void BatchLogExporterWidget::exportButtonClicked()
{
    ui->exportButton->setEnabled(false);
    ui->inputGroup->setEnabled(false);

    ui->exportProgress->show();

    for (int fileI = 0; fileI < files.size(); fileI++) {
        QString filenameUBH = files[fileI];
        if (connection_widget.load(filenameUBH)) {
//             connection_widget.timestampSequential(totalTimestamps, skipTimestamps);
//             connection_widget.getDataInit();
            connection_widget.init();
            totalTimestamps = connection_widget.getTotalTimestamps();

            updateDistanceIntensityNumbers();
        }
        else {
            continue;
        }

        long scanCount = 0;
        stop = false;

        ScanData toDataSave;
        ScanData toIntensitySave;
        QVector<long> timestamps;
        QVector<QString> logTimes;

        while ((scanCount < totalTimestamps) && !stop) {
            QCoreApplication::processEvents();
            SensorDataArray ranges;
            SensorDataArray levels;
            long timestamp;
            connection_widget.getData(ranges, levels, timestamp);
            logTimes.push_back(connection_widget.getLogTime());
            timestamps.push_back(timestamp);

            if (levels.steps.size() == 0) {
                intensityNumber = 0;
            }

            toDataSave.push_back(ranges);
            toIntensitySave.push_back(levels);
            scanCount++;
        }
        connection_widget.close();

        QString filenameCSV = QFileInfo(filenameUBH).absolutePath() + "/" + QFileInfo(filenameUBH).baseName() + ".csv";
        QFile file(filenameCSV);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);

        unsigned int stepCnt = (connection_widget.getEndStep() - connection_widget.getStartStep()) + 1;

        printScanNumber(toDataSave, out);

        printTimestamps(timestamps, out);

        printLogTimes(logTimes, out);

        printScanHeader(toDataSave, out);

        for (unsigned int step = 0; step < stepCnt; step++) {
            for (int scan = 0; scan < scanCount; scan++) {
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

        file.close();

        int progressValue = static_cast<int>(((double)fileI / (double)files.size()) * 100);
        ui->exportProgress->setValue(progressValue);
    }
    QMessageBox::information(this, tr("Export information"),
                             tr("Log file export finished."));
    ui->exportProgress->hide();
    ui->exportButton->setEnabled(true);
    ui->inputGroup->setEnabled(true);
}

void BatchLogExporterWidget::initialize()
{
    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    totalTimestamps = 0;
    skipTimestamps = 0;

    stop = true;

    connection_widget.close();
    ui->filenameLine->setText("");
    ui->exportProgress->hide();

    ui->exportButton->setEnabled(false);
    ui->inputGroup->setEnabled(true);

    connection_widget.close();
    files.clear();
    show();
}

void BatchLogExporterWidget::closeButtonClicked()
{
    stop = true;
    hide();
}

