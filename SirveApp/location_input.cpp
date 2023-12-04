#include "location_input.h"


LocationInput::LocationInput()
{
	ui.setupUi(this);

	directory_path = "locations/";
	selected_file_path = "";
	path_set = false;

	RefreshListBox();
	
	connect(ui.lst_locations, &QListWidget::currentTextChanged, this, &LocationInput::OnItemChange);
	connect(ui.btn_add_location, &QPushButton::clicked, this, &LocationInput::OnAddNewPush);
	connect(ui.lst_locations, &QListWidget::itemDoubleClicked, this, &LocationInput::accept);
	
}

LocationInput::~LocationInput()
{

}

std::vector<double> LocationInput::GetECEFVector()
{

	QString path = selected_file_path;
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		INFO << "Location Input: Error, Cannot open location file " + path.toStdString();
		return std::vector<double>();
	}

	QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
	QJsonObject jsonObj = jsonDoc.object();

	double latitude = jsonObj.value("latitude").toDouble();
	double longitude = jsonObj.value("longitude").toDouble();
	double altitude = jsonObj.value("altitude").toDouble();

	arma::mat lla(3, 1);
	lla(0, 0) = latitude;
	lla(1, 0) = longitude;
	lla(2, 0) = altitude / 1000.;

	arma::mat ecef_mat = earth::LLAtoECF(lla);
	arma::vec ecef_vec = arma::vectorise(ecef_mat);
	ecef_vec = ecef_vec * 1000.;

	std::vector<double>out = arma::conv_to<std::vector<double>>::from(ecef_vec);
	out.push_back(0);
	out.push_back(0);
	out.push_back(0);

	return out;
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
			INFO << "Location Input: Error, Cannot open location file " + path.toStdString();
			continue;
		}

		file.close();

		ui.lst_locations->addItem(filename);
	}
}

void LocationInput::OnItemChange(QString item) {

	path_set = false;

	QString path = directory_path + item;
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		INFO << "Location Input: Error, Cannot open location file " + path.toStdString();
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
	
	if (jsonObj.contains("latitude") & jsonObj.contains("longitude") & jsonObj.contains("altitude")) {

		double latitude = jsonObj.value("latitude").toDouble();
		double longitude = jsonObj.value("longitude").toDouble();
		double altitude = jsonObj.value("altitude").toDouble();

		file.close();

		QString text = "";
		text = "Name: " + location_name + "\n\nDescription: " + description + "\n\nLatitude(deg): " + QString::number(latitude) + "\n\nLongitude(deg): " + QString::number(longitude) + "\n\nAltitude(m): " + QString::number(altitude);

		ui.lbl_list->setText(text);

		selected_file_path = path;
		path_set = true;
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

void LocationInput::OnAddNewPush()
{
	AddLocation add_new_location(directory_path);
	auto response = add_new_location.exec();
	
	clear();
	RefreshListBox();	

	if (response) {
		selected_file_path = add_new_location.save_path;
		path_set = true;

		accept();
	}
}


