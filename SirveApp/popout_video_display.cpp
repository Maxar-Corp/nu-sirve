#include "popout_video_display.h"

PopoutVideoDisplay::PopoutVideoDisplay(EnhancedLabel* label)
{
	mainLayout = new QGridLayout();
    mainLayout->addWidget(label);
	setLayout(mainLayout);

    setWindowTitle(tr("Video Display"));
}

PopoutVideoDisplay::~PopoutVideoDisplay()
{
    delete mainLayout;
}