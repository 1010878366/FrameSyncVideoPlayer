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

    //初始化播放模式
    m_playMode = Sequential;    //顺序播放（默认）

    //初始化视频显示组件
    m_videoWidget = new QVideoWidget(this);   //创建视频显示组件
    ui->videoLayout->addWidget(m_videoWidget);    //添加到主界面布局

    //初始化媒体播放系统
    m_mediaPlayer = new QMediaPlayer(this);   //媒体播放器核心
    m_audioOutput = new QAudioOutput(this);   //音频输出设备
    m_mediaPlayer->setAudioOutput(m_audioOutput);   //绑定视频输出
    m_mediaPlayer->setVideoOutput(m_videoWidget);   //绑定音频输出

    //初始化音频设置（0到100）
    m_audioOutput->setVolume(ui->volumeSlider->value()/100.0);

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
    connect(m_mediaPlayer,&QMediaPlayer::positionChanged,this,&Player::updatePosition);
    connect(m_mediaPlayer,&QMediaPlayer::durationChanged,this,&Player::updateDuration);
    connect(ui->progressSlider,&QSlider::sliderMoved,this,&Player::setPosition);

    //音量控制相关
    connect(ui->volumeSlider,&QSlider::valueChanged,this,&Player::setVolume);

    //播放状态改变时
    connect(m_mediaPlayer,&QMediaPlayer::playbackStateChanged,this,&Player::updatePlayIcon);

    //创建视频播放列表
    m_playlistDock = new QDockWidget(this);
    m_playlistDock->setTitleBarWidget(new QWidget());
    m_playlistWidget=new QListWidget(m_playlistDock);
    m_playlistDock->setWidget(m_playlistWidget);
    addDockWidget(Qt::RightDockWidgetArea,m_playlistDock);

    //根据屏幕分辨率设置播放列表最小宽度
    QScreen *screen=QGuiApplication::primaryScreen();
    if(screen)
    {
        int screenWidth = screen->availableGeometry().width();
        //设置为屏幕宽度的15%，但不小于200像素，不大于400像素
        int minWidth = qBound(200,screenWidth/6,400);

        m_playlistDock->setMinimumWidth(minWidth);
    }

    //设置播放列表样式
    m_playlistWidget->setStyleSheet(R"(
        QListWidget{
            background-color: #2b2b2b;
            border:none;
        }

        /*设置列表默认样式*/
        QListWidget::item{
            color:#ffffff;  /*文字颜色为白色*/
            padding:4px;    /*文字四周留4像素内边距*/
            border-bottom:1px solid #3a3a3a;    /*底层添加浅灰色1像素分割线*/
        }

        /*设置被选中项的样式*/
        QListWidget::itme:hover{
            background-color:#323232;   /*浅灰色*/
        }

        /*设置鼠标悬停项的样式*/
        QListWidget::itme:selected{
            background-color:#3a3a3a;   /*灰色*/
        }
    )");

    //创建播放列表切换按钮
    ui->PlaylistButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    ui->PlaylistButton->setFixedSize(32,32);
    ui->PlaylistButton->setFocusPolicy(Qt::NoFocus);
    ui->PlaylistButton->setToolTip("显示|隐藏播放列表");
    ui->PlaylistButton->setStyleSheet(R"(
        QPushButton{
            border:none;
            padding:1px;
        }
        QPushButton{
            background-color:rgba(255,255,255,30);
        }
    )");

    //将播放列表命令按钮添加到控制栏
    connect(ui->PlaylistButton,&QPushButton::clicked,this,&Player::togglePlaylist);

    //连接播放列表信号 - 双击实现播放功能
    connect(m_playlistWidget,&QListWidget::itemDoubleClicked,this,&Player::playlistItemDoubleClicked);

    //初始化菜单指针
    m_playbackRateMenu = nullptr;
    m_rateGroup = nullptr;
    //调用创建主菜单函数
    createMenus();

    //设置默认播放列表路径
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!appDataDir.exists())
        appDataDir.mkpath(".");

    m_strDefaultPlaylistFile = appDataDir.filePath("default.m3u");

    //添加默认播放列表
    loadDefaultPlaylist();

    //添加媒体状态变化处理操作
    connect(m_mediaPlayer,&QMediaPlayer::mediaStatusChanged,this,[this](QMediaPlayer::MediaStatus status){
        switch (status) {
        case QMediaPlayer::LoadedMedia:
            //媒体加载完成后，可以开始操作
            ui->playButton->setEnabled(true);
            break;
        case QMediaPlayer::InvalidMedia:
            //无效媒体
            QMessageBox::warning(this,"错误","无效的媒体文件！");
            break;

        default:
            break;
        }
    });

    //设置流媒体历史记录文件路径
    m_strStreamHistoryFile=appDataDir.filePath("streams.dat");

    //加载流媒体历史记录
    loadStreamHistory();

    //播放速度控制（创建播放速度菜单）
    createPlaybackRateMenu();

    //添加错误处理连接
    connect(m_mediaPlayer,&QMediaPlayer::errorOccurred,this,[this](QMediaPlayer::Error error,const QString &errorString){
        if(error!=QMediaPlayer::NoError)
            QMessageBox::warning(this,"播放错误","播放出错，请重试："+errorString);
    });

    //媒体播放结束时的处理操作
    connect(m_mediaPlayer,&QMediaPlayer::mediaStatusChanged,this,[this](QMediaPlayer::MediaStatus status){
        if(status==QMediaPlayer::EndOfMedia)
            {
            switch (m_playMode) {
            case SingleLoop:

                break;
            case Loop:

                break;
            case Sequential:

                break;
            case Random:

                break;
            default:
                break;
            }
        }
    });
}

Player::~Player()
{
    delete ui;
}

void Player::updatePlayModeIcon()
{
    QIcon icon;
    QString tooltip;
    switch (m_playMode) {
    case Sequential:
        icon = QIcon(QStringLiteral("/icons/sequential.svg"));
        tooltip="顺序播放";
        break;
    case Loop:
        icon = QIcon(QStringLiteral("/icons/loop.svg"));
        tooltip="列表循环";
        break;
    case SingleLoop:
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
    if(m_audioOutput->isMuted())
    {
        //取消静音
        m_audioOutput->setMuted(false);
        ui->volumeSlider->setValue(m_nLastVolume);
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }
    else
    {
        //静音操作
        m_nLastVolume=ui->volumeSlider->value();
        m_audioOutput->setMuted(true);
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
    if(!m_mediaPlayer->source().isEmpty())
    {
        m_lastPositions[m_mediaPlayer->source().toString()]=position;
    }
}

void Player::updateDuration(qint64 duration)    //更新总时长
{
    ui->progressSlider->setRange(0,duration);
    ui->totalTimeLabel->setText(formatTime(duration));
}

void Player::setPosition(int position)         //设置播放位置
{
    if(m_mediaPlayer->isSeekable())
    {
        m_mediaPlayer->setPosition(position);
    }
}

void Player::setVolume(int volume)
{
    m_audioOutput->setVolume(volume/100.0);
    //更新音量图标
    //Qt5存在但Qt6不存在QStyle::SP_MediaVolumeLow 和 QStyle::SP_MediaVolumeHigh，暂时没找到低中高音量的图标，到时候有了素材可以替换。
    if(volume == 0)
    {
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
    else if(volume < 33)
    {
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        ui->volumeButton->setToolTip(QString("音量：%1").arg(volume));
    }
    else if(volume < 66)
    {
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        ui->volumeButton->setToolTip(QString("音量：%1").arg(volume));
    }
    else
    {
        ui->volumeButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        ui->volumeButton->setToolTip(QString("音量：%1").arg(volume));
    }

    //如果音量不为0 更新m_nLastVolume
    if(volume > 0)
    {
        m_nLastVolume = volume;
        m_audioOutput->setMuted(false);
    }
}

void Player::updatePlayIcon(QMediaPlayer::PlaybackState state)
{
    QPushButton *button = ui->playButton;
    switch(state)
    {
    case QMediaPlayer::PlayingState:
    {
        button->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        button->setToolTip("暂停");
        break;
    }
    case QMediaPlayer::PausedState:
    {
        button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        button->setToolTip("播放");
        break;
    }
    case QMediaPlayer::StoppedState:
    {
        button->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        button->setToolTip("暂停");
        break;
    }
    default:
        break;

    }
}

void Player::togglePlaylist()
{
    m_playlistDock->setVisible(!m_playlistDock->isVisible());
    ui->PlaylistButton->setIcon(style()->standardIcon(m_playlistDock->isVisible()?QStyle::SP_FileDialogDetailedView : QStyle::SP_FileDialogListView));
}

void Player::playlistItemDoubleClicked(QListWidgetItem *item)
{
    //播放视频文件方法 双击播放
    playFile(item->data(Qt::UserRole).toString());

}

void Player::createMenus()
{
    //一、文件菜单
    QMenu *fileMenu=ui->menubar->addMenu("文件(&F)");
    //打开文件
    QAction *openAct = new QAction("打开(&O)");
    openAct->setShortcut(QKeySequence::Open);   //绑定快捷键Ctrl+O
    openAct->setStatusTip("打开视频媒体文件");
    //槽函数连接 实现打开文件
    connect(openAct,&QAction::triggered,this,&Player::openFile);

    //添加到播放列表
    QAction *addToPlaylistAct = new QAction("添加到播放列表(&A)",this);
    //槽函数连接
    connect(addToPlaylistAct,&QAction::triggered,this,&Player::addToPlaylist);

    //从播放列表移除
    QAction *removeFromPlaylistAct = new QAction("从播放列表移除(&R)",this);
    //槽函数连接
    connect(removeFromPlaylistAct,&QAction::triggered,this,&Player::removeFromPlaylist);

    //将动作添加到文件菜单
    fileMenu->addAction(openAct);
    fileMenu->addAction(addToPlaylistAct);
    fileMenu->addAction(removeFromPlaylistAct);
    fileMenu->addSeparator();
    //播放历史
    QMenu *historyMenu = fileMenu->addMenu("播放历史(&H)");
    connect(historyMenu,&QMenu::aboutToShow,this,[this,historyMenu](){
        historyMenu->clear();
        if(m_playHistory.isEmpty())
        {
            QAction *emptyAct = historyMenu->addAction("暂无播放历史记录");
            emptyAct->setEnabled(false);
        }
        else
        {
            //添加最近播放文件
            for (const auto &history:m_playHistory)
            {
                QString timeStr=history.playTime.toString("yyyy-MM-dd hh::mm");
                QString text=QString("%1(%2)").arg(history.fileName,timeStr);
                QAction *action=historyMenu->addAction(text);
                connect(action,&QAction::triggered,this,[this,history](){
                    playFile(history.filePath);
                    //恢复上次播放位置
                    QTimer::singleShot(100,this,[this,history](){
                        m_mediaPlayer->setPosition(history.lastPostion);
                    });
                });
            }
            historyMenu->addSeparator();
            QAction *clearAct = historyMenu->addAction("清除历史记录");
            connect(clearAct,&QAction::triggered,this,&Player::clearHistory);
        }
    });

    QMenu *streamMenu = fileMenu->addMenu("网络流(&N)");
    //添加网络流并打开
    streamMenu->addAction("打开网络流(&O)",this,&Player::openStreamUrl);

    //添加 最近播放 流媒体子菜单
    QMenu *recentStreamsMenu = streamMenu->addMenu("最近播放(&R)");
    connect(recentStreamsMenu,&QMenu::aboutToShow,this,[=](){
        recentStreamsMenu->clear();
        for(const QString &url:m_recentStreams){
            recentStreamsMenu->addAction(url,[=](){
                m_mediaPlayer->setSource(QUrl(url));
                m_mediaPlayer->play();
            });
        }

        if(!m_recentStreams.isEmpty())
        {
            recentStreamsMenu->addSeparator();
            QAction *clearAct = recentStreamsMenu->addAction("清除历史记录");
            connect(clearAct,&QAction::triggered,this,[this](){
                m_recentStreams.clear();
                saveStreamHistory();
            });
        }
    });

    streamMenu->addSeparator();
    streamMenu->addMenu(recentStreamsMenu);

    //二、播放菜单
    QMenu *playMenu = ui->menubar->addMenu("播放(&P)");
    m_playbackRateMenu = playMenu->addMenu("播放速度(&R)");
    QMenu *playModeMenu = playMenu->addMenu("播放模式(&M)");

    QActionGroup *modeGroup = new QActionGroup(this);
    QStringList modeNames={"顺序播放","列表循环","单曲循环","随机播放"};

    for(int i = 0;i<modeNames.size();i++)
    {
        //QAction *action = new QAction(modeNames[i],this);
        QAction *action = new QAction(this);
        action->setData(i);

        action->setCheckable(true);
        action->setText(modeNames[i]);
        modeGroup->addAction(action);
        playModeMenu->addAction(action);

        if(m_playMode == i)
        {
            action->setChecked(true);
        }
    }

    connect(modeGroup,&QActionGroup::triggered,this,[this](QAction *action){
        setPlayMode(static_cast<PlayMode>(action->data().toInt()));
    });

    ///////////




    //三、帮助菜单

}

void Player::openFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,"打开文件","","媒体文件(*.mp4 *.avi *.mkv *.txt);;所有文件(*.*)");

    if(!fileNames.isEmpty())
    {
        //添加到播放列表
        for(const QString &fileName:fileNames)
        {
            QListWidgetItem *item=new QListWidgetItem(QFileInfo(fileName).fileName());
            item->setData(Qt::UserRole,fileName);
            m_playlistWidget->addItem(item);
        }
        //自动保存到默认播放列表
        saveDefaultPlaylist();

        //播放第一个媒体文件
        playFile(fileNames.first());

        m_playlistWidget->setCurrentRow(m_playlistWidget->count()-fileNames.size());

    }
}

void Player::playFile(const QString& filePath)
{
    if(!filePath.isEmpty())
    {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

        //恢复上次播放位置
        if(m_lastPositions.contains(filePath))
        {
            m_mediaPlayer->setPosition(m_lastPositions[filePath]);
        }
        m_mediaPlayer->play();

        setWindowTitle("FrameSync视频播放器 - "+QFileInfo(filePath).fileName());

        //当前播放项高亮显示
        for(int i=0;i<m_playlistWidget->count();i++)
        {
            QListWidgetItem *item = m_playlistWidget->item(i);
            if(item->data(Qt::UserRole).toString() == filePath)
            {
                m_playlistWidget->setCurrentItem(item);
                break;
            }
        }

        //添加到历史记录
        addToHistory(filePath);
    }
}

void Player::saveDefaultPlaylist()
{
    QFile file(m_strDefaultPlaylistFile);
    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream out(&file);
        for(int i = 0;i < m_playlistWidget->count();i++)
        {
            out<<m_playlistWidget->item(i)->data(Qt::UserRole).toString()<<"\n";
        }
    }
}

void Player::clearHistory()
{
    if(QMessageBox::question(this,"确认","是否要清除所有播放记录？")==QMessageBox::Yes)
    {
        m_playHistory.clear();
        //保存播放历史记录
        savePlayHistory();
    }
}

void Player::savePlayHistory()
{
    QFile file(m_strHistoryFile);
    if(file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out<<qint32(m_playHistory.size());
    }
}

void Player::addToHistory(const QString &filePath)
{
    //检查是否已经存在
    auto it= std::find_if(m_playHistory.begin(),m_playHistory.end(),
                           [&filePath](const PlayHistory &h)
                            {return h.filePath == filePath;});

    PlayHistory history;
    history.filePath=filePath;
    history.fileName=QFileInfo(filePath).fileName();
    history.playTime=QDateTime::currentDateTime();
    history.duration=m_mediaPlayer->duration();
    history.lastPostion=m_mediaPlayer->position();

    if(it != m_playHistory.end())
    {
        //更新现有记录
        *it=history;
    }
    else
    {
        //添加新记录
        m_playHistory.prepend(history);

        //限制历史记录数量
        while (m_playHistory.size()>30) {
            m_playHistory.removeLast();
        }
    }

    //调用保存历史记录
    savePlayHistory();
}

void Player::addToPlaylist()
{
    QString fileName = QFileDialog::getSaveFileName(this,"保存播放列表","","播放列表(*.m3u)");
    if(!fileName.isEmpty())
    {
        QFile file(fileName);

        if(file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QTextStream out(&file);
            for(int i=0;i<m_playlistWidget->count();i++)
            {
                out << m_playlistWidget->item(i)->data(Qt::UserRole).toString()<<"\n";
            }
            m_strCurrentPlaylistFile = fileName;
        }
    }
}

void Player::removeFromPlaylist()
{
    QList<QListWidgetItem*> items = m_playlistWidget->selectedItems();
    for(QListWidgetItem* item:items)
    {
        delete m_playlistWidget->takeItem(m_playlistWidget->row(item));
    }

    //自动保存到默认播放列表
    saveDefaultPlaylist();

}

void Player::loadDefaultPlaylist()
{
    QFile file(m_strDefaultPlaylistFile);

    if(file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        m_playlistWidget->clear();
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString filePath = in.readLine().trimmed();
            if(!filePath.isEmpty() && QFile::exists(filePath))
            {
                QListWidgetItem *item = new QListWidgetItem(QFileInfo(filePath).fileName());
                item->setData(Qt::UserRole,filePath);
                m_playlistWidget->addItem(item);
            }
        }
    }
}

void Player::openStreamUrl()
{
    bool bOK;
    QString url = QInputDialog::getText(this,"打开网络流","请输入流媒体：",QLineEdit::Normal,"http://",&bOK);

    if(bOK && !url.isEmpty())
    {
        //验证URL格式
        QUrl streamUrl(url);
        if(!streamUrl.isValid())
        {
            QMessageBox::warning(this,"错误","无效的URL！");
            return;
        }

        //添加到播放列表
        if(!m_recentStreams.contains(url))
        {
            m_recentStreams.prepend(url);

            //限制最近播放列表大小
            while (m_recentStreams.size()>10)
            {
                m_recentStreams.removeLast();
            }
            saveStreamHistory();    //保存流媒体历史记录
        }

        //播放流媒体
        m_mediaPlayer->setSource(QUrl(url));
        m_mediaPlayer->play();

        //更新播放器界面
        setWindowTitle("FrameSync视频播放器 - " + url);
    }
}

void Player::saveStreamHistory()
{
    QFile file(m_strStreamHistoryFile);

    if(file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out << m_recentStreams;
    }
}

void Player::loadStreamHistory()
{
    QFile file(m_strStreamHistoryFile);
    if(file.open(QIODevice::ReadOnly))
    {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);
        in >> m_recentStreams;
    }
}

void Player::createPlaybackRateMenu()
{
    if(!m_playbackRateMenu) return;

    m_rateGroup = new QActionGroup(this);

    // 定义局部结构体（仅在函数内可见）
    struct RateOption {
        QString displayText;
        double rateValue;
    };

    // 使用结构体初始化向量
    QVector<RateOption> rateOptions = {
        {"0.5x", 0.5},
        {"0.75x", 0.75},
        {"1.0x(正常)", 1.0},  // 恢复"正常"标注
        {"1.25x", 1.25},
        {"1.5x", 1.5},
        {"2.0x", 2.0}
    };

    // 使用基于范围的for循环（更清晰）
    for (const auto& option : rateOptions) {
        QAction* rateAct = new QAction(option.displayText, this);
        rateAct->setData(option.rateValue);
        rateAct->setCheckable(true);

        // 默认选中1.0x
        if (qFuzzyCompare(option.rateValue, 1.0)) {
            rateAct->setChecked(true);
        }

        m_rateGroup->addAction(rateAct);
        m_playbackRateMenu->addAction(rateAct);
    }

    connect(m_rateGroup, &QActionGroup::triggered, this, [this](QAction* action) {
        setPlayBackRate(action->data().toDouble());
    });
}

void Player::setPlayMode(PlayMode mode)
{
    m_playMode=mode;
    updatePlayModeIcon();
}

void Player::setPlayBackRate(double rate)
{
    if(m_mediaPlayer)
    {
        m_mediaPlayer->setPlaybackRate(rate);
    }
}

void Player::playNext()
{
    if(m_playlistWidget->count()>0)
    {
        int nNextRow = m_playlistWidget->currentRow()+1;
        if(nNextRow>=m_playlistWidget->count())
            nNextRow=m_playMode==Loop?0:-1;

        if(nNextRow>=0)
        {
            m_playlistWidget->setCurrentRow(nNextRow);
            playFile(m_playlistWidget->currentItem()->data(Qt::UserRole).toString());
        }
    }
}
