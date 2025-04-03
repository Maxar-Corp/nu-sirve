#include "abp_version_dlg.h"

#include <set>

AbpVersionDlg::AbpVersionDlg(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
};

void AbpVersionDlg::SetVersionNumbers(std::initializer_list<double> versions, double defaultVersion) const
{
    ui.comboBoxVersion->clear();

    std::set versionsSet(versions);

    for (double version : versionsSet)
    {
        ui.comboBoxVersion->addItem(QString::number(version));
    }

    // Set the default version if it exists in the list
    if (ui.comboBoxVersion->findText(QString::number(defaultVersion)) != -1) {
        ui.comboBoxVersion->setCurrentText(QString::number(defaultVersion));
    }
}

double AbpVersionDlg::GetVersionNumber() const
{
    return ui.comboBoxVersion->currentText().toDouble();
}

bool AbpVersionDlg::LoadMTSDData() const
{
    return ui.checkBoxMTSDData->checkState() == Qt::Checked;
}
