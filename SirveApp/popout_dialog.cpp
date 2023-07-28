#include "popout_dialog.h"

PopoutDialog::PopoutDialog(QWidget* widget)
{
    mainLayout = new QGridLayout();
    mainLayout->addWidget(widget);
    setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));
}

PopoutDialog::~PopoutDialog()
{
    delete mainLayout;
}