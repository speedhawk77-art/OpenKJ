#ifndef TICKERNEW_H
#define TICKERNEW_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <atomic>
#include <QThread>
#include <settings.h>
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>
#include <spdlog/fmt/ostr.h>
#include <QMutex>

std::ostream& operator<<(std::ostream& os, const QString& s);

class TickerImageCreator : public QThread {
Q_OBJECT
    QString m_tickerText;
    int m_targetWidth;
    void run() override;

public:
    TickerImageCreator(QString TickerText, int targetWidth);

signals:
    void imageCreated(QImage image, int textWidth);

};

class TickerNew : public QThread
{
Q_OBJECT
private:
    QMutex m_mutex;
    Settings m_settings;
    std::atomic<bool> m_stop{true};
    QImage scrollImage;
    QString m_text;
    int m_height{0};
    int m_width{0};
    int m_txtWidth{1024};
    quint64 m_renderGeneration{0};
    bool m_reducedCpuMode{false};
    bool m_textOverflows{false};
    int m_speed{5};
    bool m_textChanged{false};
    std::string m_loggingPrefix{"[TickerThread]"};
    std::shared_ptr<spdlog::logger> m_logger;

    void run() override;

public:
    TickerNew();
    ~TickerNew() override;
    void startScrolling();
    QSize getSize();
    void stop();

public slots:
    void setWidth(int width);
    void setText(const QString &text, bool force = false);
    void replaceImage(const QImage &image, int textWidth);
    void refresh();
    void setSpeed(int speed);

signals:
    void newFrame(QImage frame);
    void newFrameRect(QImage frame, QRect displayArea);
    void newRect(QRect displayArea);
};

class TickerDisplayWidget : public QWidget
{
Q_OBJECT
private:
    std::string m_loggingPrefix{"[TickerDisplayWidget]"};
    std::shared_ptr<spdlog::logger> m_logger;
    TickerNew *ticker;
    QPixmap m_image;
    QRect drawRect;
    QString m_currentText;

public:
    explicit TickerDisplayWidget(QWidget *parent = nullptr);
    ~TickerDisplayWidget() override;
    void setText(const QString& newText, bool force = false);
    [[nodiscard]] QSize sizeHint() const override;
    void setSpeed(int speed);
    QString getCurrentText() { return m_currentText; }
    bool rectBasedDrawing{false};
    void stop();
    void setTickerEnabled(bool enabled);
    void refresh() {ticker->refresh();}

private slots:
    void newFrameRect(const QImage& frame, QRect displayArea);
    void newRect(QRect displayArea);
    void newFrame(const QImage& frame);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // TICKERNEW_H