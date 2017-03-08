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

#ifndef LOGEXPORTERWIDGET_H
#define LOGEXPORTERWIDGET_H

#include <QWidget>
#include <QTextStream>

#include "RangeViewWidget.h"
#include "UrgDevice.h"

#include "UrgLogHandler.h"

using namespace qrk;
using namespace std;

namespace Ui
{
class LogExporterWidget;
}

class LogExporterWidget : public QWidget
{
    Q_OBJECT

public:
    typedef QVector<SensorDataArray > ScanData;

    explicit LogExporterWidget(QWidget* parent = 0);
    ~LogExporterWidget();


protected:
    void changeEvent(QEvent* e);

private:
    Ui::LogExporterWidget* ui;
    UrgLogHandler connection_widget;
    RangeViewWidget* range_view_widget;
    UrgDevice* urg;

    long totalTimestamps;

    int distanceNumber;
    int intensityNumber;

    bool stop;

    void printScanNumber(const ScanData &toSave, QTextStream &out);
    void printTimestamps(const QVector<long> &timestamps, QTextStream &out);
    void printLogTimes(const QVector<QString> &logTimes, QTextStream &out);
    void printScanHeader(const ScanData &toSave, QTextStream &out);

private slots:
    void loadInputButtonClicked();
    void loadOutputButtonClicked();
    void exportButtonClicked();
    void updateUiWithInfo();
    void startStepChanged(int value);
    void endStepChanged(int value);
    void startScanChanged(int value);
    void endScanChanged(int value);
    void closeButtonClicked();
    void initialize();
    void updateDistanceIntensityNumbers();
};

#endif // LOGEXPORTERWIDGET_H

