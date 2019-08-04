#pragma once

#include <ui_location_gui.h>

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


class LocationInput : public QDialog//, public QWidget
{
	Q_OBJECT

public:

	QString selected_file_path;
	QString directory_path;

	LocationInput();
	~LocationInput();

	void RefreshListBox();
	void clear();

public slots:
	void OnItemChange(QString item);

private:
	Ui_Dialog ui;

};