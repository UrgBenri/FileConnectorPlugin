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

#ifndef LOGSTATWIDGET_H
#define LOGSTATWIDGET_H

#include <QWidget>
#include <QtGui>
#include <QGraphicsScene>

namespace Ui
{
class LogStatWidget;
}

using namespace std;

class LogStatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogStatWidget(QWidget* parent = 0);
    ~LogStatWidget();

    void setStartTime(int hours, int mins, int secs);
    void setEndTime(int hours, int mins, int secs);

    void setTotalScan(long scans);
    void setSkipScan(long scans);

    void setTimeStamps(const QVector<long> &timestamp);

    void setFileName(string filename);
    void reset();
    void setFileName(const QString &filename);
private:
    Ui::LogStatWidget* ui;

    QTime startTime;
    QTime endTime;

    long totalScans;
    long skippedScans;
    QVector<long> timeStamps;
    QGraphicsScene* scene;
    QPixmap m_orangeLED;
    QPixmap m_greenLED;
    QPixmap m_redLED;

    void updatePeriod();
    void checkLogFile(string fileName);
    void resetChecks();

private slots:
    void checkLogButtonClicked();
};

#endif // LOGSTATWIDGET_H

