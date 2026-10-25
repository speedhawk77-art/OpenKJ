#include "tickernew.h"

#include <QPainter>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QMutex>
#include <QApplication>
#include <QTextStream>
#include <utility>
#include <QTimer>
#include <chrono>
#include <thread>

#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>
#include <timeapi.h>
#endif

void TickerNew::run() {
    using namespace std::chrono;
    auto lastUpdate = steady_clock::now();
    int offset = 0;
#ifdef _MSC_VER
    timeBeginPeriod(1);
#endif
    while (!m_stop.load()) {
        int speed;
        QImage image;
        QRect area;
        bool changed = false;
        {
            QMutexLocker lock(&m_mutex);
            speed = m_speed;
            if (!m_textOverflows || offset >= m_txtWidth)
                offset = 0;
            if (!m_reducedCpuMode || steady_clock::now() - lastUpdate > milliseconds(7)) {
                changed = m_textChanged;
                m_textChanged = false;
                if (changed)
                    image = scrollImage;
                area = QRect(offset, 0, m_width, m_height);
                lastUpdate = steady_clock::now();
            }
        }
        // Only images and geometry cross the thread boundary. QPixmap stays in the GUI.
        if (!area.isNull()) {
            if (changed)
                emit newFrameRect(image, area);
            else
                emit newRect(area);
        }
        ++offset;
        TickerNew::usleep(static_cast<unsigned long>(speed) * 125);
    }
#ifdef _MSC_VER
    timeEndPeriod(1);
#endif
}

void TickerNew::stop()
{
    m_stop.store(true);
}

void TickerNew::startScrolling()
{
    if (isRunning())
        return;
    m_reducedCpuMode = m_settings.tickerReducedCpuMode();
    m_stop.store(false);
    {
        QMutexLocker lock(&m_mutex);
        m_textChanged = true;
    }
    start();
}

TickerNew::~TickerNew()
{
    stop();
    wait();
    for (auto *creator : findChildren<TickerImageCreator *>())
        creator->wait();
}

TickerNew::TickerNew()
{
    m_logger = spdlog::get("logger");
    setText("No ticker data", false);
    setObjectName("Ticker");
}

QSize TickerNew::getSize()
{
    if (!m_mutex.tryLock(100))
    {
        m_logger->warn("{} getSize() unable to lock m_mutex!", m_loggingPrefix);
        return {};
    }
    QSize size = scrollImage.size();
    m_mutex.unlock();
    return size;
}

void TickerNew::setWidth(int width)
{
    if (!m_mutex.tryLock(100))
    {
        m_logger->warn("{} setWidth() unable to lock m_mutex", m_loggingPrefix);
        return;
    }
#ifdef Q_OS_WIN
    m_height = QFontMetrics(m_settings.tickerFont()).height();
#else
    m_height = static_cast<int>(QFontMetrics(m_settings.tickerFont()).tightBoundingRect("PLACEHOLDERtextgj|i01").height() * 1.2);
#endif
    m_width = width;
    scrollImage = QImage(qMax(1, width * 2), qMax(1, m_height), QImage::Format_ARGB32_Premultiplied);
    scrollImage.fill(m_settings.tickerBgColor());
    m_textChanged = true;
    //qInfo() << "Unlocking m_mutex in setWidth()";
    m_mutex.unlock();
    setText(m_text, false);
}

void TickerNew::setText(const QString &text, bool force)
{
    if (m_text == text && !force)
        return;
    m_text = text;
    const auto generation = ++m_renderGeneration;
    auto imageCreator = new TickerImageCreator(text, m_width);
    imageCreator->setParent(this);
    connect(imageCreator, &TickerImageCreator::imageCreated, this,
            [this, generation](const QImage &image, int textWidth) {
                if (generation == m_renderGeneration)
                    replaceImage(image, textWidth);
            });
    connect(imageCreator, &TickerImageCreator::finished, imageCreator, &TickerImageCreator::deleteLater);
    imageCreator->start();
}

void TickerNew::refresh()
{
    setText(m_text, true);
}

void TickerNew::setSpeed(int speed)
{
    if (!m_mutex.tryLock(100))
    {
        m_logger->warn("{} setSpeed() unable to lock m_mutex!", m_loggingPrefix);
        return;
    }
    m_speed = qBound(1, 51 - speed, 50);
    m_mutex.unlock();
}

void TickerNew::replaceImage(const QImage &image, int textWidth) {
    if (!m_mutex.tryLock(1000))
    {
        m_logger->error("{} setText() unable to lock m_mutex!", m_loggingPrefix);
        return;
    }
    m_textChanged = true;
    m_textOverflows = false;
    m_height = image.height();
    if (image.width() > m_width)
        m_textOverflows = true;
    scrollImage = image;
    m_txtWidth = qMax(1, textWidth);
    m_mutex.unlock();
}

TickerDisplayWidget::TickerDisplayWidget(QWidget *parent)
        : QWidget(parent)
{
    m_logger = spdlog::get("logger");
    ticker = new TickerNew();
    ticker->setWidth(this->width());
    connect(ticker, &TickerNew::newFrame, this, &TickerDisplayWidget::newFrame);
    connect(ticker, &TickerNew::newFrameRect, this, &TickerDisplayWidget::newFrameRect);
    connect(ticker, &TickerNew::newRect, this, &TickerDisplayWidget::newRect);
}

TickerDisplayWidget::~TickerDisplayWidget()
{
    ticker->stop();
    ticker->wait();
    delete ticker;
}

void TickerDisplayWidget::setText(const QString& newText, bool force)
{
    m_currentText = newText;
    ticker->setText(newText, force);
    setFixedHeight(ticker->getSize().height());
}

QSize TickerDisplayWidget::sizeHint() const
{
    return ticker->getSize();
}

void TickerDisplayWidget::setSpeed(int speed)
{
    ticker->setSpeed(speed);
}

void TickerDisplayWidget::stop()
{
    ticker->stop();
    ticker->wait();
}

void TickerDisplayWidget::setTickerEnabled(bool enabled)
{
    m_logger->info("{} Enabled set to: {}", m_loggingPrefix, enabled);
    if (enabled && !ticker->isRunning()) {
        ticker->startScrolling();
        ticker->setPriority(QThread::TimeCriticalPriority);
    }
    else if (!enabled && ticker->isRunning()) {
        ticker->stop();
        ticker->wait();
    }
}


void TickerDisplayWidget::resizeEvent(QResizeEvent *event)
{
    ticker->setWidth(event->size().width());
}

void TickerDisplayWidget::newFrameRect(const QImage& frame, const QRect displayArea)
{
    rectBasedDrawing = true;
    m_image = QPixmap::fromImage(frame);
    drawRect = displayArea;
    update();
}

void TickerDisplayWidget::newRect(const QRect displayArea)
{
    if (!isVisible())
        return;
    rectBasedDrawing = true;
    drawRect = displayArea;
    update();
}

void TickerDisplayWidget::newFrame(const QImage& frame)
{
    if (!isVisible())
        return;
    rectBasedDrawing = false;
    m_image = QPixmap::fromImage(frame);
    update();
}

void TickerDisplayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (!isVisible())
        return;
    QPainter p(this);
    if (!rectBasedDrawing)
        p.drawPixmap(this->rect(), m_image);
    else
        p.drawPixmap(this->rect(), m_image, drawRect);
}

void TickerImageCreator::run() {
    Settings settings;
    std::string m_loggingPrefix{"[TickerImageCreator]"};
    std::shared_ptr<spdlog::logger> m_logger = spdlog::get("logger");
    m_logger->trace("{} Thread starting up", m_loggingPrefix);
    auto st = std::chrono::high_resolution_clock::now();

    m_logger->info("{} Rendering ticker text: {}", m_loggingPrefix, m_tickerText);
    QFont tickerFont = settings.tickerFont();
    QImage img;
    int imgHeight;
    int imgWidth;
    int txtWidth;
    if (m_tickerText == "")
        m_tickerText = "Placeholder - ticker was set to empty string";
#ifdef Q_OS_WIN
    imgHeight = QFontMetrics(tickerFont).height();
#else
    imgHeight = QFontMetrics(tickerFont).size(Qt::TextSingleLine, m_tickerText).height();
#endif
    imgWidth = QFontMetrics(tickerFont).size(Qt::TextSingleLine, m_tickerText).width();
    txtWidth = imgWidth;
    QString drawText;
    if (imgWidth > m_targetWidth)
    {
        drawText.append(m_tickerText + " • " + m_tickerText + " • ");
        imgWidth = QFontMetrics(tickerFont).size(Qt::TextSingleLine, drawText).width();
        txtWidth = txtWidth + QFontMetrics(tickerFont).size(Qt::TextSingleLine," • ").width();

    }
    else {
        drawText = m_tickerText;

    }
    img = QImage(qMax(1, imgWidth), qMax(1, imgHeight), QImage::Format_ARGB32_Premultiplied);
    if (img.isNull())
        return;
    img.fill(settings.tickerBgColor());
    QPainter p;
    p.begin(&img);
    p.setPen(QPen(settings.tickerTextColor()));
    p.setFont(settings.tickerFont());
    p.drawText(img.rect(), Qt::AlignLeft | Qt::AlignVCenter, drawText);
    p.end();
    if (settings.auxTickerFile() != QString())
    {
        m_logger->debug("{} Saving ticker data to aux file: {}", m_loggingPrefix, settings.auxTickerFile());
        QFile auxFile(settings.auxTickerFile());
        auxFile.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&auxFile);
        out << drawText;
        auxFile.close();
    }
    emit imageCreated(img, txtWidth);
    m_logger->trace("{} Ticker image rendered in {}ms",
                    m_loggingPrefix,
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - st).count()
    );
}

TickerImageCreator::TickerImageCreator(QString TickerText, int targetWidth) : m_tickerText(std::move(TickerText)), m_targetWidth(targetWidth)
{

}
