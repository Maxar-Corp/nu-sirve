#include "location_input.h"


LocationInput::LocationInput()
{
	ui.setupUi(this);

	directory_path = "locations/";
	selected_file_path = "";

	RefreshListBox();
	
	QObject::connect(ui.lst_locations, &QListWidget::currentTextChanged, this, &LocationInput::OnItemChange);
	
}

LocationInput::~LocationInput()
{

}

void LocationInput::RefreshListBox()
{
	QDir directory(directory_path);
	QStringList location = directory.entryList(QStringList() << "*.json", QDir::Files);

	location.sort();

	foreach(QString filename, location) {

		QString path = directory.dirName() + "/" + filename;
		QFile file(path);
		if (!file.open(QFile::ReadOnly)) {
			INFO << "Error, Cannot open location file " + path.toStdString();
			continue;
		}

		file.close();

		ui.lst_locations->addItem(filename);
	}
}


void LocationInput::OnItemChange(QString item) {


	QString path = directory_path + item;
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		INFO << "Error, Cannot open location file " + path.toStdString();
	}

	QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
	QJsonObject jsonObj = jsonDoc.object();
	
	QString location_name, description;
	if (jsonObj.contains("name"))
		location_name = jsonObj.value("name").toString();
	else
		location_name = "Name not found";

	if (jsonObj.contains("description"))
		description = jsonObj.value("description").toString();
	else
		description = "Description not found";
	
	//double latitude, longitude, altitude;

	if (jsonObj.contains("latitude") & jsonObj.contains("longitude") & jsonObj.contains("altitude")) {

		double latitude = jsonObj.value("latitude").toDouble();
		double longitude = jsonObj.value("longitude").toDouble();
		double altitude = jsonObj.value("altitude").toDouble();

		file.close();

		QString text = "";
		text = "Name: " + location_name + "\n\nDescription: " + description + "\n\nLatitude: " + QString::number(latitude) + "\n\nLongitude: " + QString::number(longitude) + "\n\nAltitude: " + QString::number(altitude);

		ui.lbl_list->setText(text);

		selected_file_path = path;
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Location Data Missing"));
		QString box_text = "One or more of the following keys is missing from the file ("+ item + "):\nlatitude, longitude, and/or altitude";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		int current_row = ui.lst_locations->currentRow();
		QListWidgetItem *delete_item = ui.lst_locations->takeItem(current_row);
		delete delete_item;

	}
}

void LocationInput::clear() {

	int num = ui.lst_locations->count();
	
	for (int i = 0; i < num; i++){
		QListWidgetItem *delete_item = ui.lst_locations->takeItem(0);
		delete delete_item;
	}

}


