/********************************************************************************
** Form generated from reading UI file 'QtGuiApplication1.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTGUIAPPLICATION1_H
#define UI_QTGUIAPPLICATION1_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtGuiApplication1Class
{
public:
    QAction *actionLoad_OSM;
    QWidget *centralWidget;
    QTabWidget *tabMenu;
    QWidget *tab;
    QFrame *line;
    QLabel *lbl_file_name;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout;
    QLineEdit *txt_start_frame;
    QLabel *label_4;
    QLineEdit *txt_end_frame;
    QPushButton *btn_get_frames;
    QLabel *label_3;
    QLabel *lbl_max_frames;
    QFrame *line_3;
    QPushButton *btn_load_osm;
    QLabel *lbl_file_load;
    QFrame *line_4;
    QCheckBox *btn_use_epoch;
    QLineEdit *txt_epoch;
    QLabel *label_5;
    QLabel *label_6;
    QPushButton *btn_copy_directory;
    QWidget *tab_2;
    QWidget *gridLayoutWidget;
    QGridLayout *grdLayout_Color_Control;
    QLabel *lbl_gain;
    QLabel *lbl_gain_value;
    QLabel *lbl_lift_value;
    QSlider *slider_lift;
    QSlider *slider_gain;
    QLabel *lbl_lift;
    QFrame *line_2;
    QPushButton *btn_reset_color_correction;
    QCheckBox *chk_relative_histogram;
    QWidget *tab_5;
    QPushButton *btn_create_nuc;
    QCheckBox *chk_apply_nuc;
    QLineEdit *txt_nuc_start;
    QLineEdit *txt_nuc_stop;
    QLabel *lblNucStart;
    QLabel *lblNucStop;
    QLabel *lblNuc;
    QFrame *line_5;
    QLineEdit *txt_bgs_num_frames;
    QPushButton *btn_bgs;
    QCheckBox *chk_bgs;
    QLabel *lblBGS;
    QLabel *lblBGS_num_frames;
    QFrame *line_6;
    QPushButton *btn_deinterlace;
    QCheckBox *chk_deinterlace;
    QComboBox *cmb_deinterlace_options;
    QLabel *lbl_deinterlace;
    QPushButton *btn_clear_filters;
    QWidget *tab_6;
    QFrame *frmVideoPlayer;
    QSlider *sldrVideo;
    QPushButton *btn_play;
    QPushButton *btn_slow_back;
    QPushButton *btn_fast_forward;
    QFrame *frm_video;
    QLabel *lbl_fps;
    QPushButton *btn_prev_frame;
    QPushButton *btn_next_frame;
    QPushButton *btn_video_menu;
    QPushButton *btn_pause;
    QPushButton *btn_reverse;
    QPushButton *btn_frame_save;
    QPushButton *btn_frame_record;
    QLabel *lbl_video_frame;
    QLabel *lbl_video_time_midnight;
    QTabWidget *tabPlots;
    QWidget *tab_3;
    QFrame *frm_histogram;
    QFrame *frm_histogram_abs;
    QWidget *tab_4;
    QFrame *frm_plots;
    QComboBox *cmb_plot_yaxis;
    QCheckBox *chk_plot_full_data;
    QComboBox *cmb_plot_xaxis;
    QCheckBox *chk_plot_primary_data;
    QLabel *label;
    QLabel *label_2;
    QCheckBox *chk_plot_show_line;
    QPushButton *btn_save_plot;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtGuiApplication1Class)
    {
        if (QtGuiApplication1Class->objectName().isEmpty())
            QtGuiApplication1Class->setObjectName(QString::fromUtf8("QtGuiApplication1Class"));
        QtGuiApplication1Class->resize(1723, 737);
        actionLoad_OSM = new QAction(QtGuiApplication1Class);
        actionLoad_OSM->setObjectName(QString::fromUtf8("actionLoad_OSM"));
        centralWidget = new QWidget(QtGuiApplication1Class);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        tabMenu = new QTabWidget(centralWidget);
        tabMenu->setObjectName(QString::fromUtf8("tabMenu"));
        tabMenu->setGeometry(QRect(20, 30, 270, 621));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        line = new QFrame(tab);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 90, 231, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lbl_file_name = new QLabel(tab);
        lbl_file_name->setObjectName(QString::fromUtf8("lbl_file_name"));
        lbl_file_name->setGeometry(QRect(10, 20, 231, 31));
        lbl_file_name->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_file_name->setWordWrap(true);
        gridLayoutWidget_2 = new QWidget(tab);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(40, 120, 160, 91));
        gridLayout = new QGridLayout(gridLayoutWidget_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        txt_start_frame = new QLineEdit(gridLayoutWidget_2);
        txt_start_frame->setObjectName(QString::fromUtf8("txt_start_frame"));
        txt_start_frame->setEnabled(false);
        txt_start_frame->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(txt_start_frame, 1, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 0, 1, 1, 1);

        txt_end_frame = new QLineEdit(gridLayoutWidget_2);
        txt_end_frame->setObjectName(QString::fromUtf8("txt_end_frame"));
        txt_end_frame->setEnabled(false);
        txt_end_frame->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(txt_end_frame, 1, 1, 1, 1);

        btn_get_frames = new QPushButton(gridLayoutWidget_2);
        btn_get_frames->setObjectName(QString::fromUtf8("btn_get_frames"));
        btn_get_frames->setEnabled(false);

        gridLayout->addWidget(btn_get_frames, 3, 0, 1, 2);

        label_3 = new QLabel(gridLayoutWidget_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        lbl_max_frames = new QLabel(gridLayoutWidget_2);
        lbl_max_frames->setObjectName(QString::fromUtf8("lbl_max_frames"));

        gridLayout->addWidget(lbl_max_frames, 2, 0, 1, 2);

        line_3 = new QFrame(tab);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setGeometry(QRect(10, 230, 231, 20));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        btn_load_osm = new QPushButton(tab);
        btn_load_osm->setObjectName(QString::fromUtf8("btn_load_osm"));
        btn_load_osm->setGeometry(QRect(10, 70, 141, 23));
        lbl_file_load = new QLabel(tab);
        lbl_file_load->setObjectName(QString::fromUtf8("lbl_file_load"));
        lbl_file_load->setGeometry(QRect(10, 370, 231, 211));
        lbl_file_load->setFrameShape(QFrame::Box);
        lbl_file_load->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_file_load->setWordWrap(true);
        line_4 = new QFrame(tab);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setGeometry(QRect(10, 340, 231, 20));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        btn_use_epoch = new QCheckBox(tab);
        btn_use_epoch->setObjectName(QString::fromUtf8("btn_use_epoch"));
        btn_use_epoch->setEnabled(false);
        btn_use_epoch->setGeometry(QRect(20, 250, 131, 17));
        txt_epoch = new QLineEdit(tab);
        txt_epoch->setObjectName(QString::fromUtf8("txt_epoch"));
        txt_epoch->setEnabled(false);
        txt_epoch->setGeometry(QRect(20, 300, 211, 20));
        label_5 = new QLabel(tab);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setEnabled(false);
        label_5->setGeometry(QRect(20, 320, 211, 16));
        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setEnabled(false);
        label_6->setGeometry(QRect(20, 280, 47, 13));
        btn_copy_directory = new QPushButton(tab);
        btn_copy_directory->setObjectName(QString::fromUtf8("btn_copy_directory"));
        btn_copy_directory->setEnabled(false);
        btn_copy_directory->setGeometry(QRect(160, 70, 81, 23));
        tabMenu->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayoutWidget = new QWidget(tab_2);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 19, 231, 95));
        grdLayout_Color_Control = new QGridLayout(gridLayoutWidget);
        grdLayout_Color_Control->setSpacing(6);
        grdLayout_Color_Control->setContentsMargins(11, 11, 11, 11);
        grdLayout_Color_Control->setObjectName(QString::fromUtf8("grdLayout_Color_Control"));
        grdLayout_Color_Control->setContentsMargins(0, 0, 0, 0);
        lbl_gain = new QLabel(gridLayoutWidget);
        lbl_gain->setObjectName(QString::fromUtf8("lbl_gain"));
        lbl_gain->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gain, 1, 0, 1, 1);

        lbl_gain_value = new QLabel(gridLayoutWidget);
        lbl_gain_value->setObjectName(QString::fromUtf8("lbl_gain_value"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lbl_gain_value->sizePolicy().hasHeightForWidth());
        lbl_gain_value->setSizePolicy(sizePolicy);
        lbl_gain_value->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_gain_value, 1, 2, 1, 1);

        lbl_lift_value = new QLabel(gridLayoutWidget);
        lbl_lift_value->setObjectName(QString::fromUtf8("lbl_lift_value"));
        lbl_lift_value->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_lift_value, 0, 2, 1, 1);

        slider_lift = new QSlider(gridLayoutWidget);
        slider_lift->setObjectName(QString::fromUtf8("slider_lift"));
        slider_lift->setEnabled(true);
        slider_lift->setMaximum(1000);
        slider_lift->setPageStep(10);
        slider_lift->setValue(0);
        slider_lift->setOrientation(Qt::Horizontal);
        slider_lift->setTickPosition(QSlider::TicksAbove);
        slider_lift->setTickInterval(100);

        grdLayout_Color_Control->addWidget(slider_lift, 0, 1, 1, 1);

        slider_gain = new QSlider(gridLayoutWidget);
        slider_gain->setObjectName(QString::fromUtf8("slider_gain"));
        slider_gain->setEnabled(true);
        slider_gain->setMinimum(0);
        slider_gain->setMaximum(1000);
        slider_gain->setPageStep(10);
        slider_gain->setValue(1000);
        slider_gain->setOrientation(Qt::Horizontal);
        slider_gain->setTickPosition(QSlider::TicksAbove);
        slider_gain->setTickInterval(100);

        grdLayout_Color_Control->addWidget(slider_gain, 1, 1, 1, 1);

        lbl_lift = new QLabel(gridLayoutWidget);
        lbl_lift->setObjectName(QString::fromUtf8("lbl_lift"));
        lbl_lift->setSizeIncrement(QSize(0, 0));
        lbl_lift->setAlignment(Qt::AlignCenter);

        grdLayout_Color_Control->addWidget(lbl_lift, 0, 0, 1, 1);

        line_2 = new QFrame(tab_2);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(10, 160, 231, 20));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        btn_reset_color_correction = new QPushButton(tab_2);
        btn_reset_color_correction->setObjectName(QString::fromUtf8("btn_reset_color_correction"));
        btn_reset_color_correction->setGeometry(QRect(190, 130, 51, 21));
        chk_relative_histogram = new QCheckBox(tab_2);
        chk_relative_histogram->setObjectName(QString::fromUtf8("chk_relative_histogram"));
        chk_relative_histogram->setGeometry(QRect(20, 130, 151, 17));
        tabMenu->addTab(tab_2, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        btn_create_nuc = new QPushButton(tab_5);
        btn_create_nuc->setObjectName(QString::fromUtf8("btn_create_nuc"));
        btn_create_nuc->setGeometry(QRect(180, 50, 71, 31));
        chk_apply_nuc = new QCheckBox(tab_5);
        chk_apply_nuc->setObjectName(QString::fromUtf8("chk_apply_nuc"));
        chk_apply_nuc->setGeometry(QRect(20, 100, 70, 17));
        txt_nuc_start = new QLineEdit(tab_5);
        txt_nuc_start->setObjectName(QString::fromUtf8("txt_nuc_start"));
        txt_nuc_start->setGeometry(QRect(20, 60, 61, 20));
        txt_nuc_start->setAlignment(Qt::AlignCenter);
        txt_nuc_stop = new QLineEdit(tab_5);
        txt_nuc_stop->setObjectName(QString::fromUtf8("txt_nuc_stop"));
        txt_nuc_stop->setGeometry(QRect(100, 60, 61, 20));
        txt_nuc_stop->setAlignment(Qt::AlignCenter);
        lblNucStart = new QLabel(tab_5);
        lblNucStart->setObjectName(QString::fromUtf8("lblNucStart"));
        lblNucStart->setGeometry(QRect(20, 40, 61, 20));
        lblNucStart->setAlignment(Qt::AlignCenter);
        lblNucStop = new QLabel(tab_5);
        lblNucStop->setObjectName(QString::fromUtf8("lblNucStop"));
        lblNucStop->setGeometry(QRect(100, 40, 61, 20));
        lblNucStop->setAlignment(Qt::AlignCenter);
        lblNuc = new QLabel(tab_5);
        lblNuc->setObjectName(QString::fromUtf8("lblNuc"));
        lblNuc->setGeometry(QRect(10, 10, 171, 16));
        lblNuc->setTextFormat(Qt::RichText);
        line_5 = new QFrame(tab_5);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setGeometry(QRect(10, 120, 231, 20));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);
        txt_bgs_num_frames = new QLineEdit(tab_5);
        txt_bgs_num_frames->setObjectName(QString::fromUtf8("txt_bgs_num_frames"));
        txt_bgs_num_frames->setGeometry(QRect(30, 190, 61, 20));
        txt_bgs_num_frames->setAlignment(Qt::AlignCenter);
        btn_bgs = new QPushButton(tab_5);
        btn_bgs->setObjectName(QString::fromUtf8("btn_bgs"));
        btn_bgs->setGeometry(QRect(120, 180, 111, 31));
        chk_bgs = new QCheckBox(tab_5);
        chk_bgs->setObjectName(QString::fromUtf8("chk_bgs"));
        chk_bgs->setGeometry(QRect(20, 230, 181, 17));
        lblBGS = new QLabel(tab_5);
        lblBGS->setObjectName(QString::fromUtf8("lblBGS"));
        lblBGS->setGeometry(QRect(10, 140, 171, 16));
        QFont font;
        font.setPointSize(8);
        lblBGS->setFont(font);
        lblBGS->setTextFormat(Qt::RichText);
        lblBGS_num_frames = new QLabel(tab_5);
        lblBGS_num_frames->setObjectName(QString::fromUtf8("lblBGS_num_frames"));
        lblBGS_num_frames->setGeometry(QRect(20, 170, 91, 20));
        line_6 = new QFrame(tab_5);
        line_6->setObjectName(QString::fromUtf8("line_6"));
        line_6->setGeometry(QRect(0, 260, 231, 20));
        line_6->setFrameShape(QFrame::HLine);
        line_6->setFrameShadow(QFrame::Sunken);
        btn_deinterlace = new QPushButton(tab_5);
        btn_deinterlace->setObjectName(QString::fromUtf8("btn_deinterlace"));
        btn_deinterlace->setGeometry(QRect(190, 300, 61, 22));
        chk_deinterlace = new QCheckBox(tab_5);
        chk_deinterlace->setObjectName(QString::fromUtf8("chk_deinterlace"));
        chk_deinterlace->setGeometry(QRect(20, 340, 161, 17));
        cmb_deinterlace_options = new QComboBox(tab_5);
        cmb_deinterlace_options->setObjectName(QString::fromUtf8("cmb_deinterlace_options"));
        cmb_deinterlace_options->setGeometry(QRect(20, 300, 161, 22));
        lbl_deinterlace = new QLabel(tab_5);
        lbl_deinterlace->setObjectName(QString::fromUtf8("lbl_deinterlace"));
        lbl_deinterlace->setGeometry(QRect(20, 280, 121, 16));
        btn_clear_filters = new QPushButton(tab_5);
        btn_clear_filters->setObjectName(QString::fromUtf8("btn_clear_filters"));
        btn_clear_filters->setGeometry(QRect(4, 560, 251, 23));
        tabMenu->addTab(tab_5, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QString::fromUtf8("tab_6"));
        tabMenu->addTab(tab_6, QString());
        frmVideoPlayer = new QFrame(centralWidget);
        frmVideoPlayer->setObjectName(QString::fromUtf8("frmVideoPlayer"));
        frmVideoPlayer->setGeometry(QRect(309, 50, 680, 600));
        frmVideoPlayer->setFrameShape(QFrame::Box);
        frmVideoPlayer->setFrameShadow(QFrame::Raised);
        sldrVideo = new QSlider(frmVideoPlayer);
        sldrVideo->setObjectName(QString::fromUtf8("sldrVideo"));
        sldrVideo->setGeometry(QRect(10, 530, 651, 22));
        sldrVideo->setSliderPosition(0);
        sldrVideo->setOrientation(Qt::Horizontal);
        sldrVideo->setTickPosition(QSlider::NoTicks);
        btn_play = new QPushButton(frmVideoPlayer);
        btn_play->setObjectName(QString::fromUtf8("btn_play"));
        btn_play->setGeometry(QRect(350, 560, 35, 30));
        btn_slow_back = new QPushButton(frmVideoPlayer);
        btn_slow_back->setObjectName(QString::fromUtf8("btn_slow_back"));
        btn_slow_back->setGeometry(QRect(590, 560, 35, 30));
        btn_fast_forward = new QPushButton(frmVideoPlayer);
        btn_fast_forward->setObjectName(QString::fromUtf8("btn_fast_forward"));
        btn_fast_forward->setGeometry(QRect(550, 560, 35, 30));
        frm_video = new QFrame(frmVideoPlayer);
        frm_video->setObjectName(QString::fromUtf8("frm_video"));
        frm_video->setGeometry(QRect(10, 10, 660, 500));
        frm_video->setFrameShape(QFrame::StyledPanel);
        frm_video->setFrameShadow(QFrame::Raised);
        lbl_fps = new QLabel(frmVideoPlayer);
        lbl_fps->setObjectName(QString::fromUtf8("lbl_fps"));
        lbl_fps->setGeometry(QRect(490, 570, 47, 13));
        btn_prev_frame = new QPushButton(frmVideoPlayer);
        btn_prev_frame->setObjectName(QString::fromUtf8("btn_prev_frame"));
        btn_prev_frame->setGeometry(QRect(230, 560, 35, 30));
        btn_next_frame = new QPushButton(frmVideoPlayer);
        btn_next_frame->setObjectName(QString::fromUtf8("btn_next_frame"));
        btn_next_frame->setGeometry(QRect(390, 560, 35, 30));
        btn_video_menu = new QPushButton(frmVideoPlayer);
        btn_video_menu->setObjectName(QString::fromUtf8("btn_video_menu"));
        btn_video_menu->setGeometry(QRect(630, 560, 41, 30));
        btn_pause = new QPushButton(frmVideoPlayer);
        btn_pause->setObjectName(QString::fromUtf8("btn_pause"));
        btn_pause->setGeometry(QRect(310, 560, 35, 30));
        btn_reverse = new QPushButton(frmVideoPlayer);
        btn_reverse->setObjectName(QString::fromUtf8("btn_reverse"));
        btn_reverse->setGeometry(QRect(270, 560, 35, 30));
        btn_frame_save = new QPushButton(frmVideoPlayer);
        btn_frame_save->setObjectName(QString::fromUtf8("btn_frame_save"));
        btn_frame_save->setEnabled(false);
        btn_frame_save->setGeometry(QRect(10, 560, 35, 30));
        btn_frame_record = new QPushButton(frmVideoPlayer);
        btn_frame_record->setObjectName(QString::fromUtf8("btn_frame_record"));
        btn_frame_record->setEnabled(false);
        btn_frame_record->setGeometry(QRect(50, 560, 35, 30));
        lbl_video_frame = new QLabel(frmVideoPlayer);
        lbl_video_frame->setObjectName(QString::fromUtf8("lbl_video_frame"));
        lbl_video_frame->setGeometry(QRect(20, 510, 81, 20));
        lbl_video_time_midnight = new QLabel(frmVideoPlayer);
        lbl_video_time_midnight->setObjectName(QString::fromUtf8("lbl_video_time_midnight"));
        lbl_video_time_midnight->setGeometry(QRect(230, 510, 195, 20));
        lbl_video_time_midnight->setFrameShape(QFrame::NoFrame);
        lbl_video_time_midnight->setFrameShadow(QFrame::Plain);
        lbl_video_time_midnight->setAlignment(Qt::AlignCenter);
        tabPlots = new QTabWidget(centralWidget);
        tabPlots->setObjectName(QString::fromUtf8("tabPlots"));
        tabPlots->setGeometry(QRect(1000, 50, 684, 620));
        tabPlots->setTabPosition(QTabWidget::South);
        tabPlots->setTabShape(QTabWidget::Rounded);
        tabPlots->setTabsClosable(false);
        tabPlots->setMovable(true);
        tabPlots->setTabBarAutoHide(false);
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        frm_histogram = new QFrame(tab_3);
        frm_histogram->setObjectName(QString::fromUtf8("frm_histogram"));
        frm_histogram->setGeometry(QRect(10, 10, 651, 281));
        frm_histogram->setFrameShape(QFrame::StyledPanel);
        frm_histogram->setFrameShadow(QFrame::Raised);
        frm_histogram_abs = new QFrame(tab_3);
        frm_histogram_abs->setObjectName(QString::fromUtf8("frm_histogram_abs"));
        frm_histogram_abs->setGeometry(QRect(10, 300, 651, 281));
        frm_histogram_abs->setFrameShape(QFrame::StyledPanel);
        frm_histogram_abs->setFrameShadow(QFrame::Raised);
        tabPlots->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        frm_plots = new QFrame(tab_4);
        frm_plots->setObjectName(QString::fromUtf8("frm_plots"));
        frm_plots->setGeometry(QRect(10, 10, 660, 500));
        frm_plots->setFrameShape(QFrame::StyledPanel);
        frm_plots->setFrameShadow(QFrame::Raised);
        cmb_plot_yaxis = new QComboBox(tab_4);
        cmb_plot_yaxis->setObjectName(QString::fromUtf8("cmb_plot_yaxis"));
        cmb_plot_yaxis->setEnabled(false);
        cmb_plot_yaxis->setGeometry(QRect(160, 540, 101, 22));
        chk_plot_full_data = new QCheckBox(tab_4);
        chk_plot_full_data->setObjectName(QString::fromUtf8("chk_plot_full_data"));
        chk_plot_full_data->setEnabled(false);
        chk_plot_full_data->setGeometry(QRect(520, 520, 111, 21));
        chk_plot_full_data->setChecked(true);
        cmb_plot_xaxis = new QComboBox(tab_4);
        cmb_plot_xaxis->setObjectName(QString::fromUtf8("cmb_plot_xaxis"));
        cmb_plot_xaxis->setEnabled(false);
        cmb_plot_xaxis->setGeometry(QRect(10, 540, 131, 22));
        chk_plot_primary_data = new QCheckBox(tab_4);
        chk_plot_primary_data->setObjectName(QString::fromUtf8("chk_plot_primary_data"));
        chk_plot_primary_data->setEnabled(false);
        chk_plot_primary_data->setGeometry(QRect(520, 540, 131, 21));
        chk_plot_primary_data->setChecked(false);
        label = new QLabel(tab_4);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 520, 131, 20));
        label->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(tab_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(160, 520, 101, 20));
        label_2->setAlignment(Qt::AlignCenter);
        chk_plot_show_line = new QCheckBox(tab_4);
        chk_plot_show_line->setObjectName(QString::fromUtf8("chk_plot_show_line"));
        chk_plot_show_line->setEnabled(false);
        chk_plot_show_line->setGeometry(QRect(520, 560, 151, 21));
        chk_plot_show_line->setChecked(false);
        btn_save_plot = new QPushButton(tab_4);
        btn_save_plot->setObjectName(QString::fromUtf8("btn_save_plot"));
        btn_save_plot->setEnabled(false);
        btn_save_plot->setGeometry(QRect(460, 520, 41, 31));
        tabPlots->addTab(tab_4, QString());
        QtGuiApplication1Class->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(QtGuiApplication1Class);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1723, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        QtGuiApplication1Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtGuiApplication1Class);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        QtGuiApplication1Class->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(QtGuiApplication1Class);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        QtGuiApplication1Class->setStatusBar(statusBar);
        QWidget::setTabOrder(tabMenu, tabPlots);
        QWidget::setTabOrder(tabPlots, btn_slow_back);
        QWidget::setTabOrder(btn_slow_back, btn_play);
        QWidget::setTabOrder(btn_play, btn_fast_forward);
        QWidget::setTabOrder(btn_fast_forward, sldrVideo);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionLoad_OSM);

        retranslateUi(QtGuiApplication1Class);

        tabMenu->setCurrentIndex(0);
        tabPlots->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(QtGuiApplication1Class);
    } // setupUi

    void retranslateUi(QMainWindow *QtGuiApplication1Class)
    {
        QtGuiApplication1Class->setWindowTitle(QApplication::translate("QtGuiApplication1Class", "Data Processing Tool", nullptr));
        actionLoad_OSM->setText(QApplication::translate("QtGuiApplication1Class", "Load OSM", nullptr));
        lbl_file_name->setText(QApplication::translate("QtGuiApplication1Class", "File Name:", nullptr));
        txt_start_frame->setText(QApplication::translate("QtGuiApplication1Class", "0", nullptr));
        label_4->setText(QApplication::translate("QtGuiApplication1Class", "End Frame", nullptr));
        btn_get_frames->setText(QApplication::translate("QtGuiApplication1Class", "Load Frames", nullptr));
        label_3->setText(QApplication::translate("QtGuiApplication1Class", "Start Frame", nullptr));
        lbl_max_frames->setText(QApplication::translate("QtGuiApplication1Class", "Max Frames: ", nullptr));
        btn_load_osm->setText(QApplication::translate("QtGuiApplication1Class", "Load OSM File", nullptr));
        lbl_file_load->setText(QApplication::translate("QtGuiApplication1Class", "File Load Status:", nullptr));
        btn_use_epoch->setText(QApplication::translate("QtGuiApplication1Class", "Use epoch as t0", nullptr));
        label_5->setText(QApplication::translate("QtGuiApplication1Class", "Format is:    YYYY/MM/DD HH:MM:SS", nullptr));
        label_6->setText(QApplication::translate("QtGuiApplication1Class", "Epoch", nullptr));
        btn_copy_directory->setText(QApplication::translate("QtGuiApplication1Class", "Copy Directory", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab), QApplication::translate("QtGuiApplication1Class", "Import", nullptr));
        lbl_gain->setText(QApplication::translate("QtGuiApplication1Class", "Light\n"
"Set Point", nullptr));
        lbl_gain_value->setText(QApplication::translate("QtGuiApplication1Class", "1.0", nullptr));
        lbl_lift_value->setText(QApplication::translate("QtGuiApplication1Class", "0.0", nullptr));
        lbl_lift->setText(QApplication::translate("QtGuiApplication1Class", "Dark \n"
"Set Point", nullptr));
        btn_reset_color_correction->setText(QApplication::translate("QtGuiApplication1Class", "Reset", nullptr));
        chk_relative_histogram->setText(QApplication::translate("QtGuiApplication1Class", "Show Relative Histogram", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_2), QApplication::translate("QtGuiApplication1Class", "Color", nullptr));
        btn_create_nuc->setText(QApplication::translate("QtGuiApplication1Class", "Create NUC", nullptr));
        chk_apply_nuc->setText(QApplication::translate("QtGuiApplication1Class", "Apply NUC", nullptr));
        lblNucStart->setText(QApplication::translate("QtGuiApplication1Class", "Start Frame", nullptr));
        lblNucStop->setText(QApplication::translate("QtGuiApplication1Class", "Stop Frame", nullptr));
        lblNuc->setText(QApplication::translate("QtGuiApplication1Class", "Non-Uniformity Correction (NUC)", nullptr));
        btn_bgs->setText(QApplication::translate("QtGuiApplication1Class", "Create Background \n"
"Subtraction", nullptr));
        chk_bgs->setText(QApplication::translate("QtGuiApplication1Class", "Apply Background Subtraction", nullptr));
        lblBGS->setText(QApplication::translate("QtGuiApplication1Class", "Background Subtraction:", nullptr));
        lblBGS_num_frames->setText(QApplication::translate("QtGuiApplication1Class", "Number of Frames", nullptr));
        btn_deinterlace->setText(QApplication::translate("QtGuiApplication1Class", "Create", nullptr));
        chk_deinterlace->setText(QApplication::translate("QtGuiApplication1Class", "Apply De-Interlace Method", nullptr));
        lbl_deinterlace->setText(QApplication::translate("QtGuiApplication1Class", "De-Interlace Methods", nullptr));
        btn_clear_filters->setText(QApplication::translate("QtGuiApplication1Class", "Clear All Image Processing Filters", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_5), QApplication::translate("QtGuiApplication1Class", "Processing", nullptr));
        tabMenu->setTabText(tabMenu->indexOf(tab_6), QApplication::translate("QtGuiApplication1Class", "Info", nullptr));
        btn_play->setText(QApplication::translate("QtGuiApplication1Class", "Pl", nullptr));
        btn_slow_back->setText(QApplication::translate("QtGuiApplication1Class", "SS", nullptr));
        btn_fast_forward->setText(QApplication::translate("QtGuiApplication1Class", "FF", nullptr));
        lbl_fps->setText(QString());
        btn_prev_frame->setText(QApplication::translate("QtGuiApplication1Class", "S", nullptr));
        btn_next_frame->setText(QApplication::translate("QtGuiApplication1Class", "F", nullptr));
        btn_video_menu->setText(QApplication::translate("QtGuiApplication1Class", "...", nullptr));
        btn_pause->setText(QApplication::translate("QtGuiApplication1Class", "Pa", nullptr));
        btn_reverse->setText(QApplication::translate("QtGuiApplication1Class", "Re", nullptr));
        btn_frame_save->setText(QApplication::translate("QtGuiApplication1Class", "Sa", nullptr));
        btn_frame_record->setText(QApplication::translate("QtGuiApplication1Class", "Rec", nullptr));
        lbl_video_frame->setText(QString());
        lbl_video_time_midnight->setText(QString());
        tabPlots->setTabText(tabPlots->indexOf(tab_3), QApplication::translate("QtGuiApplication1Class", "Histogram", nullptr));
        chk_plot_full_data->setText(QApplication::translate("QtGuiApplication1Class", "Plot All Data", nullptr));
        chk_plot_primary_data->setText(QApplication::translate("QtGuiApplication1Class", "Plot Primary Data Only", nullptr));
        label->setText(QApplication::translate("QtGuiApplication1Class", "X - Axis", nullptr));
        label_2->setText(QApplication::translate("QtGuiApplication1Class", "Y - Axis", nullptr));
        chk_plot_show_line->setText(QApplication::translate("QtGuiApplication1Class", "Show Video Frame Location", nullptr));
        btn_save_plot->setText(QApplication::translate("QtGuiApplication1Class", "Sa", nullptr));
        tabPlots->setTabText(tabPlots->indexOf(tab_4), QApplication::translate("QtGuiApplication1Class", "Plots", nullptr));
        menuFile->setTitle(QApplication::translate("QtGuiApplication1Class", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("QtGuiApplication1Class", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtGuiApplication1Class: public Ui_QtGuiApplication1Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTGUIAPPLICATION1_H
