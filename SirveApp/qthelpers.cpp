#include "qthelpers.h"

void QtHelpers::LaunchMessageBox(QString title, QString message) {
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}