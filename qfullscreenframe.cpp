#include "QFullscreenFrame.h"

QFullscreenFrame::QFullscreenFrame(QWidget* mainWindow, Qt::AnchorPoint anchor, QWidget *parent)
    : QMainWindow(parent)
    , m_anchor(anchor)
    , m_mainWindow(mainWindow)
    , m_wasFullscreen(false)
    , m_mouseleft(false)
    , m_toShowNormal(false)
    , m_valid(true)
    , initializeCount(2)
{
    connect(&m_timer, &QTimer::timeout, this, [&]{
        if(!isMinimized())
            closeWhenMouseIsOut();
    });

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setWindowTitle(mainWindow->windowTitle());

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QRect rectMain = mainWindow->geometry();
    setGeometry(QRect(QPoint(rectMain.left(), rectMain.top()+20), QSize(rectMain.width()-100, minimumSize().height())));
}

QFullscreenFrame::~QFullscreenFrame()
{
}

void QFullscreenFrame::changeEvent(QEvent *event)
{
//    qDebug() << "QFullscreenFrame::changeEvent";
    QWindowStateChangeEvent * winEvent=nullptr;
    if(event->type() == QEvent::WindowStateChange && (winEvent = dynamic_cast<QWindowStateChangeEvent*>(event)) != nullptr) {
        qDebug() << winEvent << winEvent->oldState();
        qDebug() << windowHandle()->windowState();
        QWindow* qwindow = windowHandle();
        switch(qwindow->windowState()) {
            case Qt::WindowMaximized:
                if(winEvent->oldState() == Qt::WindowMinimized) {
                    m_mainWindow->setVisible(true);
                    closeFrame();
                    break;
                }
                // Since Qt::WindowMaximized is called twice and needs to be dealt with
                --initializeCount;
                if(initializeCount > 0) {
                    setMaximumHeight(size().height());
                } else {
//                    qDebug() << qwindow->geometry() << qwindow->frameGeometry();

                    // As QMainWindow::geometry() does not include size of the window frame,
                    // you need to set the position and size considering QWindow::frameMargines()
                    QRect rectMain = m_mainWindow->geometry();
                    QMargins margin = qwindow->frameMargins();
                    qwindow->setGeometry(QRect(QPoint(rectMain.left()+margin.left(), rectMain.top()+margin.top()),
                                               QSize(rectMain.width()-margin.left()-margin.right(), qwindow->height()-margin.top()-margin.bottom())));

                    m_wasFullscreen = true;
                    closeWhenMouseIsOut();
                    m_timer.start(2000);
                }
                break;
            case Qt::WindowMinimized:
                // Close the window while keeping the full screen mode.
                // Leave this window and hide the MainWindow
                m_mainWindow->setVisible(false);
                break;
            case Qt::WindowNoState:
                // First call should be ignored
                if(m_wasFullscreen) {
                    closeAndShowNormal();
                }
                break;
        }
    }
    return QMainWindow::changeEvent(event);
}

void QFullscreenFrame::closeAndShowNormal()
{
    m_toShowNormal = true;
    closeFrame();
}

void QFullscreenFrame::showWithTitlebar()
{
    emit init();
    showMaximized();
//    qDebug() << "showWithTitlebar";
}

void QFullscreenFrame::showWithoutTitleBar()
{
    emit init();
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    showNormal();

    initializeCount-=2;
    setMaximumHeight(size().height());
    QRect rectMain = m_mainWindow->geometry();
    switch(m_anchor) {
    case Qt::AnchorTop:
        setGeometry(QRect(rectMain.topLeft(),
                                   QSize(rectMain.width(), height())));
        break;
    case Qt::AnchorBottom:
        setGeometry(QRect(QPoint(rectMain.left(), rectMain.bottom()-height()),
                                   QSize(rectMain.width(), height())));
        break;
    case Qt::AnchorLeft:
        setGeometry(QRect(rectMain.topLeft(),
                                   QSize(width(), rectMain.height())));
        break;
    case Qt::AnchorRight:
        setGeometry(QRect(QPoint(rectMain.left()-width(), rectMain.top()),
                                   QSize(width(), rectMain.height())));
        break;
    }

    m_wasFullscreen = true;
    closeWhenMouseIsOut();
    m_timer.start(2000);
}

void QFullscreenFrame::closeFrame()
{
    m_mouseleft = true;
    close();
}

void QFullscreenFrame::closeWhenMouseIsOut()
{
//    qDebug() << "closeWhenMouseIsOut" << m_mainWindow->mapFromGlobal(cursor().pos()).y() << m_mainWindow->height() << height();
    switch(m_anchor) {
    case Qt::AnchorTop:
        if(m_mainWindow->mapFromGlobal(cursor().pos()).y() > height()+30) {
            closeFrame();
        }
        break;
    case Qt::AnchorBottom:
        if(m_mainWindow->mapFromGlobal(cursor().pos()).y() < m_mainWindow->height()-height()-30) {
            closeFrame();
        }
        break;
    case Qt::AnchorLeft:
        if(m_mainWindow->mapFromGlobal(cursor().pos()).x() > width()+30) {
            closeFrame();
        }
        break;
    case Qt::AnchorRight:
        if(m_mainWindow->mapFromGlobal(cursor().pos()).x() < m_mainWindow->width()-width()-30) {
            closeFrame();
        }
        break;
    }
}

void QFullscreenFrame::closeEvent(QCloseEvent *event)
{
//    qDebug() << "QFullscreenFrame::closeEvent";
    m_timer.stop();
    m_valid = false;
    emit deinit();

    if(m_toShowNormal) {
        emit toShowNormal();
    }

    if(!m_mouseleft) {
        emit exitApp();
    }
    QMainWindow::closeEvent(event);
}

#ifdef Q_OS_WIN

#include <Windows.h>
#include <windowsx.h>

// This method is used because QShowEvent may not be called
bool QFullscreenFrame::nativeEvent(const QByteArray &, void *message, long *result)
{
    MSG *msg = reinterpret_cast<MSG*>(message);
    auto hwnd = msg->hwnd;
    // Only react when the mouse cursor moves below the window
    if(msg->message == WM_MOUSELEAVE && isFullScreen()) {
        closeWhenMouseIsOut();
    }
    if(msg->message == WM_NCMOUSELEAVE && isMaximized()) {
        closeWhenMouseIsOut();
    }
    // Prevent movement of the window by dragging the title bar of QFullscreenFrame
    if(isMaximized() && (msg->message == WM_NCLBUTTONDOWN || msg->message == WM_NCLBUTTONUP)) {
        LRESULT hitid = ::DefWindowProcW(hwnd, WM_NCHITTEST, 0, msg->lParam);
//        qDebug() << hitid << QPoint(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));
        if(hitid != HTCAPTION)
            return false;
        *result = 0;
        return true;
    }
    return false;
}

#endif
