#pragma once

#include <ui_location_gui.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qdir.h>


class LocationInput : public QDialog
{
	Q_OBJECT

public:

	QString selected_file_path;

	LocationInput();
	~LocationInput();

private:
	Ui_Dialog ui;
	int num;
};