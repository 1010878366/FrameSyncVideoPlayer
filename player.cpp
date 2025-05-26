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

    //更新播放模式图标
    //updatePlayModeIcon();
    connect(ui->playModeButton,&QPushButton::clicked,this,&Player::updatePlayModeIcon);

    //静音切换操作
    connect(ui->volumeButton,&QPushButton::clicked,this,&Player::toggleMute);
    //通过键盘m按键实现静音切换功能
    QShortcut *muteShrtcut=new QShortcut(Qt::Key_M,this);
    connect(muteShrtcut,&QShortcut::activated,this,&Player::toggleMute);

    //进度条相关
    connect(mediaPlayer,&QMediaPlayer::positionChanged,this,&Player::updatePosition);
    connect(mediaPlayer,&QMediaPlayer::durationChanged,this,&Player::updateDuration);
    connect(ui->progressSlider,&QSlider::sliderMoved,this,&Player::setPosition);

    //音量控制相关
}

Player::~Player()
{
    delete ui;
}

void Player::updatePlayModeIcon()
{
    QIcon icon;
    QString tooltip;
    switch (playMode) {
    case Sequential:
        icon = QIcon(QStringLiteral("/icons/sequential.svg"));
        tooltip="顺序播放";
        break;
    case Loop:
        icon = QIcon(QStringLiteral("/icons/loop.svg"));
        tooltip="列表循环";
        break;
    case SignleLoop:
        icon = QIcon(QStringLiteral("/icons/signal-loop.svg"));
        tooltip="单曲循环";
        break;
    case Random:
        icon = QIcon(QStringLiteral("/icons/random.svg"));
        tooltip="随机播放";
        break;
    default:
        break;
    }

    //确保图标大小合适
    QSize iconSize(24,24);
    ui->playModeButton->setIconSize(iconSize);
    ui->playModeButton->setIcon(icon);
    ui->playModeButton->setToolTip(tooltip);
}


void Player::toggleMute()
{
    if(audioOutput->isMuted())
    {
        //取消静音
        audioOutput->setMuted(false);
        ui->volumeSlider->setValue(m_nLastVolumn);
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }
    else
    {
        //静音操作
        m_nLastVolumn=ui->volumeSlider->value();
        audioOutput->setMuted(true);
        ui->volumeSlider->setValue(0);
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
}

QString Player::formatTime(qint64 milliseconds)
{
    qint64 seconds=milliseconds/1000;
    qint64 minutes=seconds/60;
    seconds=seconds%60;

    return QString("%1:%2")
        .arg(minutes,2,10,QChar('0'))
        .arg(seconds,2,10,QChar('0'));
}

void Player::updatePosition(qint64 position)   //进度条控制
{
    ui->progressSlider->setValue(position);
    ui->currentTimeLabel->setText(formatTime(position));

    //保存当前播放位置
    if(!mediaPlayer->source().isEmpty())
    {
        lastPositions[mediaPlayer->source().toString()]=position;
    }
}

void Player::updateDuration(qint64 duration)    //更新总时长
{
    ui->progressSlider->setRange(0,duration);
    ui->totalTimeLabel->setText(formatTime(duration));
}

void Player::setPosition(int position)         //设置播放位置
{
    if(mediaPlayer->isSeekable())
    {
        mediaPlayer->setPosition(position);
    }
}


