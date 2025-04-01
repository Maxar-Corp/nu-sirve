#include "popout_dialog.h"

PopoutDialog::PopoutDialog()
{
    mainLayout = new QGridLayout();
    setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));

    setSizeGripEnabled(true);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}


void PopoutDialog::acquire(QWidget* widget)
{
    mainLayout->addWidget(widget);
}