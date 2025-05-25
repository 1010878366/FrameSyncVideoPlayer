#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include<QSlider>
#include<QMouseEvent>
#include<QStyle>

class ClickableSlider:public QSlider
{
    Q_OBJECT
public:
    explicit ClickableSlider(QWidget *parent=nullptr):QSlider(parent){
        setOrientation(Qt::Horizontal); //设置滑动条方向为水平
    }

protected:
    void mousePressEvent(QMouseEvent *event) override{
        //仅处理左键点击事件
        if(event->button()==Qt::LeftButton)
        {
            //计算点击位置对应的值
            double posRadio=event->pos().x()/static_cast<double>(width());
            int newValue=posRadio*(maximum()-minimum())+minimum();

            //设置新值并触发信号
            setValue(newValue);
            emit sliderMoved(newValue);    //发送位置变化信号
            event->accept();

        }

        //调用基类实现保持：正常拖拽 右键菜单等其他事件
        QSlider::mousePressEvent(event);

    }



};


#endif // CLICKABLESLIDER_H
