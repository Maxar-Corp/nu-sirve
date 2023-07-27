#include "popout_dialog.h"

PopoutDialog::PopoutDialog(EnhancedLabel* label)
{
	mainLayout = new QGridLayout();
    mainLayout->addWidget(label);
	setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));
}

PopoutDialog::PopoutDialog(Clickable_QChartView* chart_view)
{
    mainLayout = new QGridLayout();
    mainLayout->addWidget(chart_view);
    setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));
}

PopoutDialog::PopoutDialog(NewChartView* new_chart_view)
{
    mainLayout = new QGridLayout();
    mainLayout->addWidget(new_chart_view);
    setLayout(mainLayout);

    setWindowTitle(tr("Popout Display"));
}

PopoutDialog::~PopoutDialog()
{
    delete mainLayout;
}