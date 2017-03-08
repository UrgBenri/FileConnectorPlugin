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

#include "BatchLogImporterWidget.h"
#include "ui_BatchLogImporterWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QDebug>

BatchLogImporterWidget::BatchLogImporterWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BatchLogImporterWidget)
{
    ui->setupUi(this);

    ui->totalStep->setMinimum(0);
    ui->totalStep->setMaximum(1440);
    ui->totalStep->setValue(1440);

    ui->frontStep->setMinimum(0);
    ui->frontStep->setMaximum(1440);
    ui->frontStep->setValue(540);

    ui->minDistance->setMinimum(0);
    ui->minDistance->setMaximum(60000);
    ui->minDistance->setValue(25);

    ui->maxDistance->setMinimum(0);
    ui->maxDistance->setMaximum(60000);
    ui->maxDistance->setValue(30000);

    ui->loadingIncator->hide();
    ui->exportProgress->hide();
    ui->exportProgress->setValue(0);

    distanceNumber = 3;
    intensityNumber = 3;

    connect(ui->loadInputButton, SIGNAL(clicked()),
            this, SLOT(loadInputButtonClicked()));

    connect(ui->importButton, SIGNAL(clicked()),
            this, SLOT(importButtonClicked()));

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(closeButtonClicked()));
}

BatchLogImporterWidget::~BatchLogImporterWidget()
{
            qDebug() << "BatchLogImporterWidget::~BatchLogImporterWidget";
    delete ui;
}

void BatchLogImporterWidget::changeEvent(QEvent* e)
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

void BatchLogImporterWidget::importButtonClicked()
{

    ui->importButton->setEnabled(false);
    ui->exportProgress->setValue(0);
    ui->exportProgress->show();



    for (int fileI = 0; fileI < files.size(); fileI++) {
        //----------------------------------------------
        QCoreApplication::processEvents();

        QString filenameCSV = files[fileI];

        ranges.clear();
        levels.clear();

        UrgLogHandler l;
        if (!l.getDataFromCSV(filenameCSV,
                              ranges,
                              levels,
                              distanceNumber,
                              intensityNumber)) {
            QMessageBox::warning(this, tr("Loading error"),
                                 l.what());
            return;
        }

        //--------------------------------------------------------------------------
        QString filenameUBH = QFileInfo(filenameCSV).absolutePath() + "/" + QFileInfo(filenameCSV).baseName() + ".ubh";

        if (logger.create(QFile::encodeName(filenameUBH).data()) && ranges.size() > 0) {
            RangeSensorParameter parameter;
            parameter.area_min = 0;
            parameter.area_max = ranges[0].steps.size() / (distanceNumber + 1);
            parameter.area_front = ui->frontStep->value();
            parameter.area_total = ui->totalStep->value();
            parameter.distance_min = ui->minDistance->value();
            parameter.distance_max = ui->maxDistance->value();
            parameter.model = "UTM-30LX";

            logger.setCaptureMode(NE_Capture_mode);

            logger.addRangeSensorParameter(parameter);
            logger.addCaptureMode(NE_Capture_mode);
            logger.addScanMsec(25);

            logger.addStartStep(0);
            logger.addEndStep(ranges[0].steps.size() / (distanceNumber + 1));

            logger.addSerialNumber("H00000");


            for (int i = 0; i < ranges.size(); i++) {
                logger.addData(ranges[i], levels[i], i *25);
            }

            logger.close();
        }

        int progressValue = static_cast<int>(((double)fileI / (double)files.size()) * 100);
        ui->exportProgress->setValue(progressValue);
    }

    ui->exportProgress->hide();

    QMessageBox::information(this, tr("Import information"),
                             tr("Log file import finished."));

    ui->importButton->setEnabled(true);
}

void BatchLogImporterWidget::closeButtonClicked()
{
    hide();
}

//void BatchLogImporterWidget::trimVector(QVector<QVector<long> > &ldata)
//{
//    for (int i = 0; i < ldata.size(); i++) {
//        QCoreApplication::processEvents();
//        if (ldata[i].size() > 0) {
//            if (ldata[i][ldata[i].size() - 1] == -1) {
//                ldata[i].pop_back();
//            }
//        }
//    }
//}

void BatchLogImporterWidget::loadInputButtonClicked()
{
    ui->importButton->setEnabled(false);
    ui->inputFilenameLine->setText("");

    QString directory_path = QFileDialog::getExistingDirectory(
                                 this,
                                 tr("Open CSV file directory"),
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
        ui->inputFilenameLine->setText(directory_path);
        ui->loadInputButton->setEnabled(false);

        QDirIterator directory_walker(directory_path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        files.clear();
        while (directory_walker.hasNext()) {
            directory_walker.next();
            if (directory_walker.fileInfo().completeSuffix().toUpper() == "CSV") {
                files.push_back(directory_walker.filePath());
                //                qDebug() << directory_walker.filePath();
            }
            QApplication::processEvents();
        }
        ui->loadingIncator->hide();
        ui->loadInputButton->setEnabled(true);
        ui->importButton->setEnabled(true);
    }

}

