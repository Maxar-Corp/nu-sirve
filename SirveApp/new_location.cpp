#include "new_location.h"

AddLocation::AddLocation(QString path)
{
	ui.setupUi(this);
	directory_path = path;
    connect(ui.btn_create, &QPushButton::clicked, this, &AddLocation::CreateSensor);
}

AddLocation::~AddLocation()
{
}

bool AddLocation::VerifyLatitude()
{
	QString value = ui.txt_latitude->text();

	bool check_numeric = isNumeric(value);
    bool check_within_range = VerifyWithinRange(value, -90, 90);

	return check_numeric & check_within_range;
}

bool AddLocation::VerifyLongitude()
{
	QString value = ui.txt_longitude->text();

	bool check_numeric = isNumeric(value);
    bool check_within_range1 = VerifyWithinRange(value, -180, 180);
    bool check_within_range2 = VerifyWithinRange(value, 0, 360);

	return check_numeric & (check_within_range1 || check_within_range2);
	
	return false;
}

bool AddLocation::VerifyAltitudeAsNumeric()
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

bool AddLocation::VerifyWithinRange(QString text, double min_value, double max_value)
{
	int input_value = text.toDouble();
	
	if (input_value < min_value) 		
		return false;
	
	if (input_value > max_value)
		return false;
	
	return true;
}

void AddLocation::MakeLocationFile()
{
	bool ok;
	QString new_file_name = QInputDialog::getText(this, "Save File", "Enter filename", QLineEdit::Normal, "", &ok);

	if (!new_file_name.contains(".json", Qt::CaseInsensitive))
		new_file_name = new_file_name + ".json";

	QFileInfo check_file(directory_path + new_file_name);
	if (check_file.exists()) {
		QString label = "This file (" + new_file_name + ") already exists. Would you like to write oer the existing file?";
		auto reply = QMessageBox::question(this, "File Already Exists", label, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		
		if (reply == QMessageBox::StandardButton::No)
			ok = false;
	}
		
	if (ok && !new_file_name.isEmpty())
	{
		QJsonObject json_obj;
		json_obj["name"] = ui.txt_name->text();
		json_obj["description"] = ui.txt_description->text();
		json_obj["latitude"] = ui.txt_latitude->text().toDouble();
		json_obj["longitude"] = ui.txt_longitude->text().toDouble();
		json_obj["altitude"] = ui.txt_altitude->text().toDouble();

		QJsonDocument json_doc(json_obj);
		QByteArray json_string = json_doc.toJson();

		save_path = directory_path + new_file_name;
		QFile save_file(save_path);
		save_file.open(QIODevice::WriteOnly | QIODevice::Text);

		save_file.write(json_string.data());
		save_file.close();

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Location File Created"));
		QString box_text = "Location file successfully created!";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		
		accept();
	}
	
}

void AddLocation::CreateSensor() {

    if(VerifyLatitude() & VerifyLongitude() & VerifyAltitudeAsNumeric())
        MakeLocationFile();
	else {

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Invalid Entry"));
		QString box_text = "Location file not created. Check inputs boxes contain numeric data only.\nLatitude should be between -90 to 90.\nLongitude should vary between -180 to 180 or 0 to 360.";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
	}

}
