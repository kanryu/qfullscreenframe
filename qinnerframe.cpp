#include "qinnerframe.h"

QInnerFrame::QInnerFrame(QWidget *parent, Qt::AnchorPoint anchor)
    : QFrame(parent)
    , m_anchor(anchor)
    , m_mainWindow(parent)
    , m_valid(true)
    , m_toShowNormal(false)
{
    connect(&m_timer, &QTimer::timeout, this, [&]{
        if(!isMinimized())
            closeWhenMouseIsOut();
    });
    m_mainWindow->installEventFilter(this);

    // Since no background is set, borrow the setting of the top level Widget
    auto layout = new QBoxLayout(
                Qt::AnchorTop || Qt::AnchorBottom ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight,
                this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    QPalette Pal(parent->topLevelWidget()->palette());
    qDebug() << Pal;
    Pal.setColor(QPalette::Base, Pal.color(QPalette::Window));
    setAutoFillBackground(true);
    setPalette(Pal);
}

bool QInnerFrame::eventFilter(QObject *watched, QEvent *event)
{
//    qDebug() << watched << event;
    if(m_mainWindow == dynamic_cast<QWidget*>(watched)) {
        switch(event->type()) {
        case QEvent::Resize:
            close();
        }
    }
    return false;
}

void QInnerFrame::showWithoutTitleBar()
{
    emit init();
    showNormal();

    QRect rectMain = mainGeometry();
    setMaximumHeight(size().height());
    switch(m_anchor) {
    case Qt::AnchorTop:
        setGeometry(QRect(rectMain.topLeft(),
                                   QSize(rectMain.width(), height())));
        break;
    case Qt::AnchorBottom:
        setGeometry(QRect(QPoint(rectMain.left(), rectMain.bottom()-height()+1),
                                   QSize(rectMain.width(), height())));
        break;
    case Qt::AnchorLeft:
        setGeometry(QRect(rectMain.topLeft(),
                                   QSize(width(), rectMain.height())));
        break;
    case Qt::AnchorRight:
        setGeometry(QRect(QPoint(rectMain.right()-width()+1, rectMain.top()),
                                   QSize(width(), rectMain.height())));
        break;
    }
    closeWhenMouseIsOut();
    if(isValid())
        m_timer.start(2000);
}

void QInnerFrame::closeAndShowNormal()
{
    m_toShowNormal = true;
    close();
}

void QInnerFrame::closeEvent(QCloseEvent *event)
{
    m_timer.stop();
    m_mainWindow->removeEventFilter(this);
    m_valid = false;
    emit deinit();

    if(m_toShowNormal) {
        emit toShowNormal();
    }
    QFrame::closeEvent(event);
    emit closed();
}

void QInnerFrame::closeWhenMouseIsOut()
{
    QPoint ptInMain = m_mainWindow->mapFromGlobal(cursor().pos());
    if(!mainGeometry().contains(ptInMain)) {
        close();
        return;
    }
    switch(m_anchor) {
    case Qt::AnchorTop:
        if(ptInMain.y() > height()+30) {
            close();
        }
        break;
    case Qt::AnchorBottom:
        if(ptInMain.y() < m_mainWindow->height()-height()-30) {
            close();
        }
        break;
    case Qt::AnchorLeft:
        if(ptInMain.x() > width()+30) {
            close();
        }
        break;
    case Qt::AnchorRight:
        if(ptInMain.x() < m_mainWindow->width()-width()-30) {
            close();
        }
        break;
    }
}

QRect QInnerFrame::mainGeometry()
{
    return QRect(QPoint(), m_mainWindow->size());
}
