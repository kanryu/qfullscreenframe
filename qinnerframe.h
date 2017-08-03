#ifndef QINNERFRAME_H
#define QINNERFRAME_H

#include <QtWidgets>

class QInnerFrame : public QFrame
{
    Q_OBJECT
public:
    QInnerFrame(QWidget* parent, Qt::AnchorPoint anchor=Qt::AnchorTop);
    /**
     * @brief showWithoutTitleBar
     * Show the frame without title bar
     */
    void showWithoutTitleBar();
    /**
     * @brief closeAndShowNormal
     * Method to be called to end fullscreen display with button etc arranged in frame
     */
    void closeAndShowNormal();
    bool isValid() { return m_valid; }

    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event);

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
    /**
     * @brief closed
     * It is emitted after the frame is closed.
     * Usually this instance is unnecessary and should be released here.
     */
    void closed();


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

#endif // QINNERFRAME_H
