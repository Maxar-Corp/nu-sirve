#pragma once

#ifndef LOCATION_INPUT_H
#define LOCATION_INPUT_H

#include <ui_location_gui.h>

#include "new_location.h"
#include "support/earth.h"

#include <armadillo>
#include <qdialog.h>
#include <qdir.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpushbutton.h>


class LocationInput : public QDialog//, public QWidget
{
	Q_OBJECT

public:

    LocationInput();
    ~LocationInput();

	QString selected_file_path;
	QString directory_path;
	bool path_set;

    void ClearWidgetListItems();
	std::vector<double> GetECEFVector();
	void RefreshListBox();

public slots:
    void OnAddNewPush();
	void OnItemChange(QString item);

private:
	Ui_Dialog ui;

};

#endif
