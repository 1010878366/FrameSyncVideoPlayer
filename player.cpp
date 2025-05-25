#include "player.h"
#include "ui_player.h"

//自定义滑动条样式表
QString styleSheetSlider=R"(
/*水平滑槽基础样式表*/
    QSlider::groove:horizontal {
        height: 6px;
        background: #E0E0E0;
        border-radius: 1px;
    }

/*已经填充区域样式*/
    QSlider::sub-page:horizontal {
        background: #2196F3;
        border-radius: 1px;
    }

/*未填充区域样式*/
    QSlider::add-page:horizontal {
        background: #E0E0E0;
        border-radius: 1px;
    }

/*滑块默认样式*/
    QSlider::handle:horizontal {
        background: white;
        width: 5px;
        height: 5px;
        margin: -8px 0;
        border-radius: 1px;
        border: 2px solid #2196F3;
    }

/*滑块渐变效果   白到浅蓝渐变*/
    QSlider::handle:horizontal {
    background: qradialgradient(cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 white, stop:1 #E3F2FD);
}

/*悬停状态*/
    QSlider::handle:horizontal:hover {
        background: #E3F2FD;
        border: 2px solid #1976D2;
    }

/*按下状态*/
    QSlider::handle:horizontal:pressed {
        background: #BBDEFB;
        border: 2px solid #0D47A1; /*最深蓝边框 */
    }
)";



Player::Player(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Player)
{
    ui->setupUi(this);

    //设置窗口标题
    setWindowIcon(QIcon(":/icons/player.svg"));

    //初始化视频显示组件
    videoWidget = new QVideoWidget(this);   //创建视频显示组件
    ui->videoLayout->addWidget(videoWidget);    //添加到主界面布局

    //初始化媒体播放系统
    mediaPlayer = new QMediaPlayer(this);   //媒体播放器核心
    audioOutput = new QAudioOutput(this);   //音频输出设备
    mediaPlayer->setAudioOutput(audioOutput);   //绑定视频输出
    mediaPlayer->setVideoOutput(videoWidget);   //绑定音频输出

    //初始化音频设置（0到100）
    audioOutput->setVolume(ui->volumeSlider->value()/100.0);

    //应用自定义滑动条样式
     ui->progressSlider->setStyleSheet(styleSheetSlider);    //播放进度条
    ui->volumeSlider->setStyleSheet(styleSheetSlider);      //音量进度条

    ui->currentTimeLabel->setStyleSheet("Qlabel{clolr:white;font-weight:bold;}");
    ui->totalTimeLabel->setStyleSheet("Qlabel{color:white;font-weight:bold;}");

    //设置按钮图标
    ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->forwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->backwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    //ui->playModeButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

Player::~Player()
{
    delete ui;
}

