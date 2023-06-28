#include "popout_dialog.h"

PopoutDialog::PopoutDialog(EnhancedLabel* label)
{
	mainLayout = new QGridLayout();
    mainLayout->addWidget(label);
	setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));
}

PopoutDialog::~PopoutDialog()
{
    delete mainLayout;
}