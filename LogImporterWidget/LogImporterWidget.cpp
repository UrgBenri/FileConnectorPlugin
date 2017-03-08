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

#include "LogImporterWidget.h"
#include "ui_LogImporterWidget.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

LogImporterWidget::LogImporterWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LogImporterWidget)
{
    ui->setupUi(this);

    ui->loadingIncator->hide();
    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    scanNumber = 0;
    distanceNumber = 3;
    intensityNumber = 3;

    connect(ui->loadInputButton, SIGNAL(clicked()),
            this, SLOT(loadInputButtonClicked()));
    connect(ui->loadOutputButton, SIGNAL(clicked()),
            this, SLOT(loadOutputButtonClicked()));

    connect(ui->importButton, SIGNAL(clicked()),
            this, SLOT(importButtonClicked()));

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(closeButtonClicked()));
}

LogImporterWidget::~LogImporterWidget()
{
        qDebug() << "LogImporterWidget::~LogImporterWidget";
    delete ui;
}

void LogImporterWidget::changeEvent(QEvent* e)
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


void LogImporterWidget::loadInputButtonClicked()
{
    ui->importButton->setEnabled(false);
    ui->inputFilenameLine->setText("");

    scanNumber = 0;
    m_ranges.clear();
    m_levels.clear();

    QString filename = QFileDialog::getOpenFileName(
                           this,
                           tr("Open CSV file"),
                           QDir::currentPath(),
                           tr("Log file (*.csv)"));
    if (!filename.isNull()) {
        QFileInfo fi(filename);
        QDir::setCurrent(fi.absolutePath());

        ui->loadingIncator->show();

        UrgLogHandler l;
        if (!l.getDataFromCSV(filename,
                              m_ranges,
                              m_levels,
                              distanceNumber,
                              intensityNumber)) {
            QMessageBox::warning(this, tr("Loading error"),
                                 l.what());
            return;
        }

        ui->inputFilenameLine->setText(filename);
        if (!ui->outpurFilenameLine->text().isEmpty() &&
                !ui->outpurFilenameLine->text().isEmpty()) {
            ui->importButton->setEnabled(true);
        }
        else {
            ui->importButton->setEnabled(false);
        }

        ui->loadingIncator->hide();

        RangeSensorParameter logParam;

        ui->startStep->setMinimum(0);
        //        ui->startStep->setMaximum(data[0].size() / (distanceNumber + 1));
        ui->startStep->setMaximum(1440);
        ui->startStep->setValue(0);
        logParam.area_min =  ui->startStep->value();

        ui->endStep->setMinimum(0);
        //        ui->endStep->setMaximum(data[0].size() / (distanceNumber + 1));
        ui->endStep->setMaximum(1440);
        ui->endStep->setValue(m_ranges[0].steps.size() / (distanceNumber + 1));
        logParam.area_max = ui->endStep->value();

        ui->totalStep->setMinimum(0);
        ui->totalStep->setMaximum(1440);
        ui->totalStep->setValue(1440);
        logParam.area_total = ui->totalStep->value();

        ui->frontStep->setMinimum(0);
        //        ui->frontStep->setMaximum(data[0].size() / (distanceNumber + 1));
        ui->frontStep->setMaximum(1440);
        ui->frontStep->setValue((m_ranges[0].steps.size() / (distanceNumber + 1)) / 2);
        logParam.area_front = ui->frontStep->value();

        ui->minDistance->setMinimum(0);
        ui->minDistance->setMaximum(60000);
        ui->minDistance->setValue(25);
        logParam.distance_min = ui->minDistance->value();

        ui->maxDistance->setMinimum(0);
        ui->maxDistance->setMaximum(60000);
        ui->maxDistance->setValue(30000);
        logParam.distance_max = ui->maxDistance->value();

        ui->rangeViewWidget->setParameters(logParam.area_front, logParam.area_total);
        ui->rangeViewWidget->setRange(logParam.area_min,
                                      logParam.area_max);
    }

}

void LogImporterWidget::printVector(const SensorDataArray &ldata)
{
    for (int i = 0; i < ldata.steps.size(); i++) {
        for (int j = 0; j < ldata.steps[i].size(); j++) {
            QCoreApplication::processEvents();
            printf("Scan[%d] data[%ld]\n", i, ldata.steps[i][j]);
        }
    }
}

void LogImporterWidget::trimVector(SensorDataArray &ldata)
{
    for (int i = 0; i < ldata.steps.size(); i++) {
        QCoreApplication::processEvents();
        if (ldata.steps[i].size() > 0) {
            if (ldata.steps[i][ldata.steps[i].size() - 1] == -1) {
                ldata.steps[i].pop_back();
            }
        }
    }
}

void LogImporterWidget::loadOutputButtonClicked()
{
    ui->importButton->setEnabled(false);
    ui->outpurFilenameLine->setText("");

    if (logger.isOpen()) {
        logger.close();
    }

    QDateTime dateTime = QDateTime::currentDateTime();
    QString defaulName = QString("/") +
                QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss_zzz") +
                ".ubh";
    QString filename = QFileDialog::getSaveFileName(
                           this,
                           tr("Save Sensor Data"),
                           QDir::currentPath() + defaulName,
                           tr("Log file (*.ubh)"));
    if (!filename.isNull()) {
        QFileInfo fi(filename);
        QDir::setCurrent(fi.absolutePath());

        ui->outpurFilenameLine->setText(filename);
        if (!ui->inputFilenameLine->text().isEmpty() &&
                !ui->outpurFilenameLine->text().isEmpty()) {
            ui->importButton->setEnabled(true);
        }
        else {
            ui->importButton->setEnabled(false);
        }
    }
}

void LogImporterWidget::importButtonClicked()
{
    if (logger.create(QFile::encodeName(ui->outpurFilenameLine->text()).data())) {

        ui->exportProgress->show();

        RangeSensorParameter parameter;
        parameter.area_min = ui->startStep->value();
        parameter.area_max = ui->endStep->value();
        parameter.area_front = ui->frontStep->value();
        parameter.area_total = ui->totalStep->value();
        parameter.distance_min = ui->minDistance->value();
        parameter.distance_max = ui->maxDistance->value();
        parameter.model = "UTM-30LX";

        logger.setCaptureMode(NE_Capture_mode);

        logger.addRangeSensorParameter(parameter);
        logger.addCaptureMode(NE_Capture_mode);
        logger.addScanMsec(25);

        logger.addStartStep(ui->startStep->value());
        logger.addEndStep(ui->endStep->value());

        logger.addSerialNumber("H00000");


        for (int i = 0; i < m_ranges.size(); i++) {
            logger.addData(m_ranges[i], m_levels[i], i);
            QCoreApplication::processEvents();
            int progressValue = static_cast<int>(((double)i / (double)m_ranges.size()) * 100.0);
            ui->exportProgress->setValue(progressValue);
        }
        logger.close();

        QMessageBox::information(this, tr("Import information"),
                                 tr("Log file import finished."));
        ui->exportProgress->hide();
        logger.close();
    }
}

void LogImporterWidget::closeButtonClicked()
{
    hide();
}

