#pragma once
#ifndef LOCATION_NEW_H
#define LOCATION_NEW_H

#include "ui_new_location.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qdir.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

class AddLocation : public QDialog
{
	Q_OBJECT

public:

	QString directory_path, save_path;

	AddLocation(QString path);
	~AddLocation();

    bool isNumeric(QString text);
    void MakeLocationFile();
    bool VerifyLatitude();
    bool VerifyLongitude();
    bool VerifyAltitudeAsNumeric();
    bool VerifyWithinRange(QString text, double min_value, double max_value);

public slots:

    void CreateSensor();

private:

    Ui_Dialog_Add_Location ui;

};

#endif
