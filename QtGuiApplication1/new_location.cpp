#include "new_location.h"

AddLocation::AddLocation(QString path)
{
	ui.setupUi(this);
	directory_path = path;
	QObject::connect(ui.btn_create, &QPushButton::clicked, this, &AddLocation::create_sensor);
}

AddLocation::~AddLocation()
{
}

bool AddLocation::checkLatitude()
{
	QString value = ui.txt_latitude->text();

	bool check_numeric = isNumeric(value);
	bool check_within_range = withinRange(value, -90, 90);

	return check_numeric & check_within_range;
}

bool AddLocation::checkLongitude()
{
	QString value = ui.txt_longitude->text();

	bool check_numeric = isNumeric(value);
	bool check_within_range1 = withinRange(value, -180, 180);
	bool check_within_range2 = withinRange(value, 0, 360);

	return check_numeric & (check_within_range1 || check_within_range2);
	
	return false;
}

bool AddLocation::checkAltitude()
{
	QString value = ui.txt_altitude->text();

	bool check_numeric = isNumeric(value);
	
	return check_numeric;
}

bool AddLocation::isNumeric(QString text)
{
	bool convert_value_numeric;
	int value = text.toDouble(&convert_value_numeric);

	return convert_value_numeric;
}

bool AddLocation::withinRange(QString text, double min_value, double max_value)
{
	int input_value = text.toDouble();
	
	if (input_value < min_value) 		
		return false;
	
	if (input_value > max_value)
		return false;
	
	return true;
}

void AddLocation::makeLocationFile()
{
	QString new_file_name = "updated_location.json";

	QJsonObject json_obj;
	json_obj["name"] = ui.txt_name->text();
	json_obj["description"] = ui.txt_description->text();
	json_obj["latitude"] = ui.txt_latitude->text().toDouble();
	json_obj["longitude"] = ui.txt_latitude->text().toDouble();
	json_obj["altitude"] = ui.txt_latitude->text().toDouble();

	QJsonDocument json_doc(json_obj);
	QByteArray json_string = json_doc.toJson();

	QString save_path = directory_path + new_file_name;
	QFile save_file(save_path);
	save_file.open(QIODevice::WriteOnly | QIODevice::Text);

	save_file.write(json_string.data());
	save_file.close();
}

void AddLocation::create_sensor() {
	makeLocationFile();
}
