#include "popout_dialog.h"

PopoutDialog::PopoutDialog(QWidget* widget)
{
    mainLayout = new QGridLayout();
    mainLayout->addWidget(widget);
    setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));

    setSizeGripEnabled(true);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

PopoutDialog::~PopoutDialog()
{
    delete mainLayout;
}