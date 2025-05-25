/*
 *
 *
 * 视频播放器主窗口类 集成媒体播放器 播放列表管理 界面控制
 *
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

private:
    Ui::Player *ui;

    //1.多媒体组件
    QVideoWidget *videoWidget;  //视频显示组件
    QMediaPlayer *mediaPlayer;  //媒体播放器核心
    QAudioOutput *audioOutput;  //音频输出设备

};
#endif // PLAYER_H
