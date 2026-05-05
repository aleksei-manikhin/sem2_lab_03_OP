#include "mainwindow.h"
#include "draw.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QSizePolicy>
#include <QString>

QFont boldFont(const QFont& baseFont, int pointSize) {
    QFont font = baseFont;
    font.setPointSize(pointSize);
    font.setBold(true);
    return font;
}
QSize chartPixmapSize(const QLabel* label) {
    QSize size = label->size();

    if (size.width() < CHART_DEFAULT_WIDTH)
        size.setWidth(CHART_DEFAULT_WIDTH);
    if (size.height() < CHART_DEFAULT_HEIGHT)
        size.setHeight(CHART_DEFAULT_HEIGHT);

    return size;
}

QRect chartArea(const QSize& size) {
    return QRect(CHART_LEFT_MARGIN,
                 CHART_TOP_MARGIN,

                 size.width() - CHART_LEFT_MARGIN - CHART_RIGHT_MARGIN,
                 size.height() - CHART_TOP_MARGIN - CHART_BOTTOM_MARGIN);
}

void addValuePadding(ChartValues* values) {
    double span = values->maxValue - values->minValue;
    double padding = span == 0.0 ? 1.0 : span * CHART_VALUE_PADDING_PERCENT / 100.0;

    values->minValue -= padding;
    values->maxValue += padding;
}

double valueRate(double value, const ChartValues* values) {
    double span = values->maxValue - values->minValue;
    return span == 0.0 ? 0.5 : (value - values->minValue) / span;
}

int valueToY(double value, const QRect& area, const ChartValues* values) {
    return area.bottom() - static_cast<int>(valueRate(value, values) * area.height());
}

QPoint chartPointToScreen(const ChartPoint* point, const QRect& area, const ChartValues* values) {
    double yearSpan = values->maxYear - values->minYear;
    double xRate = yearSpan == 0.0 ? 0.5 : (point->year - values->minYear + 0.5) / (yearSpan + 1.0);
    int x = area.left() + static_cast<int>(xRate * area.width());//
    int y = valueToY(point->value, area, values);

    return QPoint(x, y);
}

void drawChartAxes(QPainter* painter, const QRect& area, const ChartValues* values) {
    painter->setFont(boldFont(painter->font(), CHART_AXIS_FONT_SIZE));
    painter->setPen(QPen(QColor("#253247"), CHART_AXIS_PEN_WIDTH));

    painter->drawLine(area.bottomLeft(), area.bottomRight());
    painter->drawLine(area.bottomLeft(), area.topLeft());

    painter->drawText(area.left(), area.bottom() + CHART_YEAR_LABEL_OFFSET_Y, QString::number(values->minYear));
    painter->drawText(area.right() - CHART_MAX_YEAR_LABEL_OFFSET_X,
                      area.bottom() + CHART_YEAR_LABEL_OFFSET_Y,
                      QString::number(values->maxYear));
    painter->drawText(CHART_VALUE_LABEL_X,
                      area.top() + CHART_MAX_VALUE_LABEL_OFFSET_Y,
                      QString::number(values->maxValue, 'f', 2));
    painter->drawText(CHART_VALUE_LABEL_X, area.bottom(), QString::number(values->minValue, 'f', 2));
    painter->drawText(area.center().x() - CHART_X_AXIS_TITLE_OFFSET_X,
                      area.bottom() + CHART_X_AXIS_TITLE_OFFSET_Y,
                      "Year");
    painter->drawText(area.left() - CHART_Y_AXIS_TITLE_OFFSET_X,
                      area.top() - CHART_Y_AXIS_TITLE_OFFSET_Y,
                      "Value");
}

void drawMetricLine(QPainter* painter, const QRect& area, double value,
                    const ChartValues* values, const QString& label) {
    int y = valueToY(value, area, values);

    painter->setFont(boldFont(painter->font(), CHART_METRIC_FONT_SIZE));
    painter->setPen(QPen(QColor("#c73535"), CHART_METRIC_PEN_WIDTH, Qt::DashLine));
    painter->drawLine(area.left(), y, area.right(), y);
    painter->drawText(area.left() + CHART_METRIC_TEXT_OFFSET_X,
                      y - CHART_METRIC_TEXT_OFFSET_Y,
                      label + ": " + QString::number(value, 'f', 2));
}

void drawChartMetrics(QPainter* painter, const QRect& area,
                      const Metrics* metrics, const ChartValues* values) {
    drawMetricLine(painter, area, metrics->min, values, "Min");
    drawMetricLine(painter, area, metrics->median, values, "Median");
    drawMetricLine(painter, area, metrics->max, values, "Max");
}

void drawChartPolyline(QPainter* painter, const List* points, const QRect& area,
                       const ChartValues* values) {
    Iterator it = begin(points);
    QPoint previousPoint;
    int hasPreviousPoint = 0;

    painter->setPen(QPen(QColor("#1d4fc0"), CHART_POLYLINE_PEN_WIDTH));
    painter->setBrush(QColor("#17c2c2"));

    while (isSet(&it)) {
        const ChartPoint* point = static_cast<const ChartPoint*>(get(&it));

        if (point != nullptr) {
            QPoint screenPoint = chartPointToScreen(point, area, values);
            if (hasPreviousPoint)
                painter->drawLine(previousPoint, screenPoint);
            painter->drawEllipse(screenPoint, CHART_POINT_RADIUS, CHART_POINT_RADIUS);
            previousPoint = screenPoint;
            hasPreviousPoint = 1;
        }

        next(&it);
    }
}

void MainWindow::setupChart() {
    ui->chartLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->chartLabel->setScaledContents(true);
    ui->chartLabel->setMinimumSize(CHART_LABEL_MIN_WIDTH, CHART_LABEL_MIN_HEIGHT);
    clearChart();
}

void MainWindow::clearChart() {
    ui->chartLabel->clear();
    ui->ChartStackedWidget->setCurrentWidget(ui->chartEmptyPage);
}

int MainWindow::chartDataBounds(ChartValues* values) const {
    Iterator it = begin(context.chartPoints);
    int hasPoints = 0;

    while (isSet(&it)) {
        const ChartPoint* point = static_cast<const ChartPoint*>(get(&it));

        if (point != nullptr) {
            if (!hasPoints) {
                values->minYear = point->year;
                values->maxYear = point->year;
                values->minValue = point->value;
                values->maxValue = point->value;
                hasPoints = 1;
            } else {
                if (point->year < values->minYear)
                    values->minYear = point->year;
                if (point->year > values->maxYear)
                    values->maxYear = point->year;
                if (point->value < values->minValue)
                    values->minValue = point->value;
                if (point->value > values->maxValue)
                    values->maxValue = point->value;
            }
        }

        next(&it);
    }

    return hasPoints;
}

void MainWindow::drawChart() {
    ChartValues values = {0, 0, 0.0, 0.0};

    if (chartDataBounds(&values)) {
        addValuePadding(&values);

        QSize pixmapSize = chartPixmapSize(ui->chartLabel);
        double pixelRatio = ui->chartLabel->devicePixelRatioF();
        QPixmap pixmap(pixmapSize * pixelRatio);
        QRect area = chartArea(pixmapSize);

        pixmap.setDevicePixelRatio(pixelRatio);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        drawChartAxes(&painter, area, &values);
        drawChartMetrics(&painter, area, &context.metrics, &values);
        drawChartPolyline(&painter, context.chartPoints, area, &values);

        ui->chartLabel->setPixmap(pixmap);
        ui->ChartStackedWidget->setCurrentWidget(ui->chartPage);
    } else
        clearChart();
}
