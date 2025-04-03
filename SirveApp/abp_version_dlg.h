
#ifndef ABP_VERSION_DLG_H
#define ABP_VERSION_DLG_H

#include "ui_abp_version.h"

class AbpVersionDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AbpVersionDlg(QWidget* parent = nullptr);
    ~AbpVersionDlg() override = default;

    void SetVersionNumbers(std::initializer_list<double> versions, double defaultVersion) const;
    double GetVersionNumber() const;
    bool LoadMTSDData() const;

private:
    Ui_APBVersionDlg ui;
};


#endif // ABP_VERSION_DLG_H

