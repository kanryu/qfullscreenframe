#ifndef FULLSCREENTOPFRAME_H
#define FULLSCREENTOPFRAME_H

#include <QtWidgets>


/**
 * @brief The QFullscreenFrame class
 *
 * When displaying windows in full screen, you can display additional widgets
 * such as toolbars with overlapping coordinates without vibration of the main window.
 *
 * Frames can be displayed at the top or bottom of the screen.
 * It closes automatically when the mouse cursor leaves the frame.
 *
 * WindowSystem's own title bar can be displayed on the frame(call showMaximized()).
 * Minimize, return to normal, close buttons are replaced with appropriate actions, respectively.
 */
class QFullscreenFrame : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief QFullscreenFrame
     * @param mainWindow is a widget in the fullscreen display as a reference for displaying the frame. It can be stored in the task bar.
     * @param anchor specifies where to fix the frame in the mainWindow
     */
    explicit QFullscreenFrame(QWidget* mainWindow, Qt::AnchorPoint anchor=Qt::AnchorTop,  QWidget *parent = 0);
    ~QFullscreenFrame();

    /**
     * @brief showWithTitlebar
     * Show the frame with the title bar of WindowManager
     * Currently only Windows is implemented.
     */
    void showWithTitlebar();
    /**
     * @brief showWithoutTitleBar
     * Show the frame without title bar
     */
    void showWithoutTitleBar();
    /**
     * @brief closeFrame
     * Close Frame without exiting the application.
     * If you simply call close(), exitApp() will be emitted.
     */
    void closeFrame();
    /**
     * @brief closeAndShowNormal
     * Method to be called to end fullscreen display with button etc arranged in frame
     */
    void closeAndShowNormal();
    bool isValid() { return m_valid; }

    bool eventFilter(QObject *watched, QEvent *event);
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &, void *, long *) override;
#endif

signals:
    /**
     * @brief init
     * Called before QFullscreenFrame is displayed. To register the widgets
     * you want to display in the frame, use SLOT registered in this SIGNAL.
     */
    void init();
    /**
     * @brief deinit
     * Called before QFullscreenFrame closes.
     * In order to collect the widgets passed to this frame, register SLOT in this SIGNAL.
     */
    void deinit();
    /**
     * @brief toShowNormal
     * (only showWithTitlebar())
     * Frame's Undo (reduce) button was pressed.
     * When returning the application to normal display, register SLOT in this SIGNAL.
     */
    void toShowNormal();
    /**
     * @brief exitApp
     * (only showWithTitlebar())
     * Frame's exit app button was pressed.
     * When to exit application, register SLOT in this SIGNAL.
     */
    void exitApp();


public slots:
    /**
     * @brief closeWhenMouseIsOut
     * Determine that the current position of the mouse cursor is away from the frame,
     *  and close the frame if it is true. closed() is not emitted.
     */
    void closeWhenMouseIsOut();

private:
    QRect mainGeometry();

    QWidget* m_mainWindow;
    QWidget* m_topWidget;
    Qt::AnchorPoint m_anchor;
    bool m_wasFullscreen;
    bool m_mouseleft;
    int initializeCount;
    bool m_toShowNormal;
    bool m_valid;
    QTimer m_timer;
};

#endif // FULLSCREENTOPFRAME_H
