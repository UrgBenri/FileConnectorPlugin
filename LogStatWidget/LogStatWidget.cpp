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

#include "LogStatWidget.h"
#include "ui_LogStatWidget.h"

#include "UrgLogHandler.h"
#include <QFileDialog>

#include <sstream>
#include <iostream>

LogStatWidget::LogStatWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LogStatWidget),
    m_orangeLED(":/icons/ledorange_icon"),
    m_greenLED(":/icons/ledgreen_icon"),
    m_redLED(":/icons/ledred_icon")
{
    ui->setupUi(this);

    //    scene = new QGraphicsScene(this);
    //    ui->graphicsView->setScene(scene);
    //    ui->graphicsView->setRenderHint(QPainter::Antialiasing);


    connect(ui->checkLogButton, SIGNAL(clicked()),
            this, SLOT(checkLogButtonClicked()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(hide()));

    ui->loadIndicator->hide();

    resetChecks();

}

LogStatWidget::~LogStatWidget()
{
    qDebug() << "LogStatWidget::~LogStatWidget";
    delete ui;
}

void LogStatWidget::reset()
{
    ui->fileNameLine->setText("");

    ui->startTimeLabel->setText("0:0:0");
    ui->endTimeLabel->setText("0:0:0");
    ui->periodLabel->setText("0:0:0");

    ui->totalScansLabel->setText("0");
    ui->skipScansLabel->setText("0");
    ui->skipRatioLabel->setText("0%");

    totalScans = 0;
    skippedScans = 0;

    resetChecks();
}

void LogStatWidget::setStartTime(int hours, int mins, int secs)
{
    startTime.setHMS(hours, mins, secs);

    ui->startTimeLabel->setText(QString("%1:%2:%3")
                                .arg(hours, 2, 10, QLatin1Char('0'))
                                .arg(mins, 2, 10, QLatin1Char('0'))
                                .arg(secs, 2, 10, QLatin1Char('0')));

    updatePeriod();
}

void LogStatWidget::updatePeriod()
{

    int secs = startTime.secsTo(endTime);
    int mins = (secs / 60) % 60;
    int hours = (secs / 3600);
    secs = secs % 60;

    ui->periodLabel->setText(QString("%1:%2:%3")
                             .arg(hours, 2, 10, QLatin1Char('0'))
                             .arg(mins, 2, 10, QLatin1Char('0'))
                             .arg(secs, 2, 10, QLatin1Char('0')));
}

void LogStatWidget::setEndTime(int hours, int mins, int secs)
{
    endTime.setHMS(hours, mins, secs);

    ui->endTimeLabel->setText(QString("%1:%2:%3")
                              .arg(hours, 2, 10, QLatin1Char('0'))
                              .arg(mins, 2, 10, QLatin1Char('0'))
                              .arg(secs, 2, 10, QLatin1Char('0')));

    updatePeriod();
}

void LogStatWidget::setTotalScan(long scans)
{
    totalScans = scans;
    ui->totalScansLabel->setText(QString("%1").arg(scans));
}

void LogStatWidget::setSkipScan(long scans)
{
    skippedScans = scans;
    ui->skipScansLabel->setText(QString("%1").arg(scans));

    int ratio = 100 - (((float)scans / totalScans) * 100);
    ui->skipRatioLabel->setText(QString("%1\%").arg(ratio));
}

void LogStatWidget::setTimeStamps(const QVector<long> &timestamp)
{
    Q_UNUSED(timestamp);

    //    scene->clear();
    //    timeStamps = timestamp;

    //    scene->setSceneRect(timeStamps.size()*2, 0, timeStamps.size()*2, 20);
    //    QPen penRed = QPen(Qt::red);
    //    QPen penBlue = QPen(Qt::blue);

    //    for(unsigned i = 0; i < timeStamps.size(); ++i){
    //        QPointF topLeft = scene->sceneRect().topLeft();
    //        QPointF bottomLeft = scene->sceneRect().bottomLeft();

    //        topLeft.setX(topLeft.x()+ (i*2));
    //        bottomLeft.setX(bottomLeft.x()+ (i*2));
    //        QLineF line(topLeft, bottomLeft);

    //        if(i != 0){
    //            bool rest = (timeStamps[i] - timeStamps[i-1]) > 30;
    //            if(rest){
    //                scene->addLine(line, penRed);
    //            }
    //        }

    //        scene->addLine(line, penBlue);
    //    }

}


void LogStatWidget::checkLogButtonClicked()
{
    UrgLogHandler urgLogHandler;

    QString filename = QFileDialog::getOpenFileName(
                           this,
                           tr("Open log"),
                           QDir::currentPath(),
                           tr("UBH file (*.ubh)"));

    if (!filename.isNull()) {
        QFileInfo fi(filename);
        QDir::setCurrent(fi.absolutePath());

        resetChecks();
        ui->loadIndicator->show();
        setFileName(filename);

        ui->check1Loading->show();
        if (urgLogHandler.load(filename)) {
            ui->check1Loading->hide();
            ui->check1Result->setPixmap(m_greenLED);
            ui->check1Result->setToolTip(tr("No error"));
            ui->check1Text->setToolTip(tr("No error"));

            ui->check2Loading->show();
            // Header check
            if (urgLogHandler.headerCheck() >= 0) {
                ui->check2Loading->hide();
                ui->check2Result->setPixmap(m_greenLED);
                ui->check2Result->setToolTip(tr("No error"));
                ui->check2Text->setToolTip(tr("No error"));
            }
            else {
                ui->check2Loading->hide();
                ui->check2Result->setPixmap(m_redLED);
                ui->check2Result->setToolTip(urgLogHandler.what());
                ui->check2Text->setToolTip(urgLogHandler.what());
                ui->console->addItem(new QListWidgetItem(urgLogHandler.what()));
            }

            ui->check3Loading->show();
            // Sequencial timestamp check
            if (urgLogHandler.init(true)) {
                setTotalScan(urgLogHandler.getTotalTimestamps());
                setSkipScan(urgLogHandler.getSkippedTimeStamps());
                ui->check3Loading->hide();
                ui->check3Result->setPixmap(m_greenLED);
                ui->check3Result->setToolTip(tr("No error"));
                ui->check3Text->setToolTip(tr("No error"));

                if (urgLogHandler.setDataPos(0) < 0) {
                    qDebug() << "Error setDataPos: 0";
                }
                SensorDataArray ranges, levels;
                long t;
                if (urgLogHandler.getData(ranges, levels, t) <= 0) {
                    qDebug() << "Error: " << urgLogHandler.what();
                }
                QString firsttime = urgLogHandler.getLogTime();
                QStringList ftParts = firsttime.split(':');
                if (ftParts.size() > 6) {
                    setStartTime(ftParts.at(3).toInt(), ftParts.at(4).toInt(), ftParts.at(5).toInt());
                }

                if (urgLogHandler.setDataPos(urgLogHandler.getTotalTimestamps() - 2) < 0) {
                    qDebug() << "Error setDataPos: " << urgLogHandler.getTotalTimestamps() - 1;
                    qDebug() << urgLogHandler.what();
                }
                if (urgLogHandler.getData(ranges, levels, t) <= 0) {
                    qDebug() << "Error: " << urgLogHandler.what();
                }
                QString lasttime = urgLogHandler.getLogTime();
                QStringList ltParts = lasttime.split(':');
                if (ltParts.size() > 6) {
                    setEndTime(ltParts.at(3).toInt(), ltParts.at(4).toInt(), ltParts.at(5).toInt());
                }

            }
            else {
                ui->check3Loading->hide();
                ui->check3Result->setPixmap(m_redLED);
                ui->check3Result->setToolTip(urgLogHandler.what());
                ui->check3Text->setToolTip(urgLogHandler.what());
                ui->console->addItem(new QListWidgetItem(urgLogHandler.what()));
            }

            ui->check4Loading->show();

            // Scan data coherance check
            if (urgLogHandler.scanCoherent(true)) {
                ui->check4Loading->hide();
                ui->check4Result->setPixmap(m_greenLED);
                ui->check4Result->setToolTip(tr("No error"));
                ui->check4Text->setToolTip(tr("No error"));
            }
            else {
                ui->check4Loading->hide();
                ui->check4Result->setPixmap(m_redLED);
                ui->check4Result->setToolTip(urgLogHandler.what());
                ui->check4Text->setToolTip(urgLogHandler.what());
                ui->console->addItem(new QListWidgetItem(urgLogHandler.what()));
            }

            urgLogHandler.close();
            ui->loadIndicator->hide();
        }
        else {
            ui->check1Loading->hide();
            ui->check1Result->setPixmap(m_redLED);
            ui->check1Result->setToolTip(urgLogHandler.what());
            ui->check1Text->setToolTip(urgLogHandler.what());
            ui->console->addItem(new QListWidgetItem(urgLogHandler.what()));
        }

        urgLogHandler.close();
    }
}

void LogStatWidget::setFileName(string filename)
{
    ui->fileNameLine->setText(QString(filename.c_str()));
}

void LogStatWidget::setFileName(const QString &filename)
{
    ui->fileNameLine->setText(filename);
}

void LogStatWidget::resetChecks()
{
    ui->check1Result->setPixmap(m_orangeLED);
    ui->check2Result->setPixmap(m_orangeLED);
    ui->check3Result->setPixmap(m_orangeLED);
    ui->check4Result->setPixmap(m_orangeLED);

    ui->check1Loading->hide();
    ui->check2Loading->hide();
    ui->check3Loading->hide();
    ui->check4Loading->hide();

    ui->check1Loading->setIndicatorStyle(BusyIndicatorWidget::StyleWindows7);
    ui->check2Loading->setIndicatorStyle(BusyIndicatorWidget::StyleWindows7);
    ui->check3Loading->setIndicatorStyle(BusyIndicatorWidget::StyleWindows7);
    ui->check4Loading->setIndicatorStyle(BusyIndicatorWidget::StyleWindows7);

    ui->console->clear();
}

