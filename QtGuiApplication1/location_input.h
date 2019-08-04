#pragma once

#ifndef LOCATION_INPUT_H
#define LOCATION_INPUT_H

#include <ui_location_gui.h>

#include "logging.h"

#include "new_location.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qdir.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qmessagebox.h>


class LocationInput : public QDialog//, public QWidget
{
	Q_OBJECT

public:

	QString selected_file_path;
	QString directory_path;
	bool path_set;

	LocationInput();
	~LocationInput();

	void RefreshListBox();
	void clear();

public slots:
	void OnItemChange(QString item);
	void OnAddNewPush();

private:
	Ui_Dialog ui;

};

#endif