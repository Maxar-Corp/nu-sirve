#include "location_input.h"


LocationInput::LocationInput()
{
	ui.setupUi(this);

	selected_file_path = "";

	QDir directory("locations/");
	QStringList location = directory.entryList(QStringList() << "*.json", QDir::Files);
	foreach(QString filename, location) {
		QString a = filename;
	}

	selected_file_path = "bob1";
}

LocationInput::~LocationInput()
{

}
