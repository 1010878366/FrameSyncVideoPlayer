#include "player.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Player w;

    //配置主窗口属性
    w.setWindowTitle("FrameSync视频播放器1.1.0");
    w.setMinimumSize(800,600);

    w.show();
    return a.exec();
}
