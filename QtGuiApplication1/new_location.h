#pragma once
#ifndef LOCATION_NEW_H
#define LOCATION_NEW_H

#include "ui_new_location.h"

#include "logging.h"

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

	QString directory_path;

	AddLocation(QString path);
	~AddLocation();

	bool checkLatitude();
	bool checkLongitude();
	bool checkAltitude();

	bool isNumeric(QString text);
	bool withinRange(QString text, double min_value, double max_value);

	void makeLocationFile();

public slots:
	//void OnOkPress();
	void create_sensor();
	
private:
	
	Ui_Dialog_Add_Location ui;

};

#endif
