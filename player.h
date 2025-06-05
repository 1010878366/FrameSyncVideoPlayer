/*
 * 视频播放器主窗口类 集成媒体播放器 播放列表管理 界面控制
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include<QFileDialog>
#include<QMenu>
#include<QActionGroup>
#include<QListWidget>
#include<QMap>
#include<QStandardPaths>
#include<QDir>
#include<QMediaMetaData>
#include<QDateTime>
#include<QShortcut>
#include<QDockWidget>
#include<QShortcut>
#include<QList>
#include<QTimer>
#include<QMessageBox>

#include"ClickableSlider.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class Player;
}
QT_END_NAMESPACE

class Player : public QMainWindow
{
    Q_OBJECT

public:
    Player(QWidget *parent = nullptr);
    ~Player();

private slots:
    void toggleMute();          //切换静音状态
    void updatePosition(qint64 position);   //进度条控制
    void updateDuration(qint64 duration);   //更新总时长
    void setPosition(int position);         //设置播放位置
    void setVolume(int volume);             //设置音量（0-100）
    void updatePlayIcon(QMediaPlayer::PlaybackState state); //更新播放按钮图标

    void togglePlaylist();//切换播放列表可见性
    void playlistItemDoubleClicked(QListWidgetItem *item);  //处理列表双击事件

    void openFile();    //打开媒体文件


private:
    Ui::Player *ui;

    //1.多媒体组件
    QVideoWidget *videoWidget;  //视频显示组件
    QMediaPlayer *mediaPlayer;  //媒体播放器核心
    QAudioOutput *audioOutput;  //音频输出设备

    //
    enum PlayMode{  //播放模式枚举
        Sequential, //顺序播放
        Loop,       //列表循环
        SignleLoop, //单曲循环
        Random      //随机播放
    };
    PlayMode m_playMode;          //当前播放模式
    void updatePlayModeIcon();  //切换播放模式图标

    int m_nLastVolume = 50;      //静音之前的音量缓存
    QString formatTime(qint64 milliseconds);    //格式化显示时间(mm:ss)
    QMap<QString,qint64> lastPositions;     //文件路径——最后播放位置映射

    //播放列表组件
    QDockWidget *m_playlistDock;      //播放列表停靠窗口
    QListWidget *m_playlistWidget;    //播放列表内容控件

    struct playHistory  //播放历史记录结构
    {
        QString filePath;       //文件完整路径
        QString fileName;       //文件名称
        QDateTime playTime;     //最后播放时间
        qint64 duration;        //文件总时长
        qint64 lastPostion;     //最后播放位置
    };
    QList<playHistory> playHistory; //播放历史记录列表

    QString m_strDefaultPlaylsitFile; //默认播放列表文件
    QMenu *playbackRateMenu;    //播放速度选择菜单
    QActionGroup *rateGroup;    //播放速度动作组
    QString m_strHistoryFile;   //历史记录存储路径

    void createMenus();         //创建播放器主菜单
    void playFile(const QString& filePath); //播放文件方法
    void saveDefautlPlaylist();     //保存当前播放列表
    void clearHistory();        //清空播放历史
    void savePlayHistory();   //保存历史记录



};
#endif // PLAYER_H
