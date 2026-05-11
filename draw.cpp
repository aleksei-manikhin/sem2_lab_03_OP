#include "mainwindow.h"
#include "draw.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
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

void updateChartSpans(ChartData* chartData) {
    chartData->yearSpan = chartData->maxYear - chartData->minYear;
    chartData->valueSpan = chartData->maxValue - chartData->minValue;
}

void addValuePadding(ChartData* chartData) {
    double padding = chartData->valueSpan == 0.0
                         ? 1.0
                         : chartData->valueSpan * CHART_VALUE_PADDING_PERCENT / 100.0;

    chartData->minValue -= padding;
    chartData->maxValue += padding;
    updateChartSpans(chartData);
}

double valueRate(double value, const ChartData* chartData) {
    return chartData->valueSpan == 0.0 ? 0.5 : (value - chartData->minValue) / chartData->valueSpan;
}

int valueToY(double value, const ChartData* chartData) {
    return chartData->area->bottom() - static_cast<int>(valueRate(value, chartData) * chartData->area->height());
}

void updateScreenPoint(ChartData* chartData, const ChartPoint* point) {
    double xRate = chartData->yearSpan == 0.0 ? 0.5 : (point->year - chartData->minYear + 0.5) / (chartData->yearSpan + 1.0);
    int x = chartData->area->left() + static_cast<int>(xRate * chartData->area->width());
    int y = valueToY(point->value, chartData);

    chartData->screenPoint = Point(x, y);
}

void drawChartAxes(ChartData* chartData) {
    QPainter* painter = chartData->painter;
    const QRect* area = chartData->area;

    painter->setFont(boldFont(painter->font(), CHART_AXIS_FONT_SIZE));
    painter->setPen(QPen(QColor("#253247"), CHART_AXIS_PEN_WIDTH));

    painter->drawLine(area->bottomLeft(), area->bottomRight());
    painter->drawLine(area->bottomLeft(), area->topLeft());

    painter->drawText(area->left(), area->bottom() + CHART_YEAR_LABEL_OFFSET_Y, QString::number(chartData->minYear));
    painter->drawText(area->right() - CHART_MAX_YEAR_LABEL_OFFSET_X, area->bottom() + CHART_YEAR_LABEL_OFFSET_Y, QString::number(chartData->maxYear));
    painter->drawText(CHART_VALUE_LABEL_X, area->top() + CHART_MAX_VALUE_LABEL_OFFSET_Y, QString::number(chartData->maxValue, 'f', 2));
    painter->drawText(CHART_VALUE_LABEL_X, area->bottom(), QString::number(chartData->minValue, 'f', 2));
    painter->drawText(area->center().x() - CHART_X_AXIS_TITLE_OFFSET_X, area->bottom() + CHART_X_AXIS_TITLE_OFFSET_Y, "Year");
    painter->drawText(area->left() - CHART_Y_AXIS_TITLE_OFFSET_X, area->top() - CHART_Y_AXIS_TITLE_OFFSET_Y, "Value");
}

void drawMetricLine(ChartData* chartData, double value, const QString& label) {
    QPainter* painter = chartData->painter;
    const QRect* area = chartData->area;
    int y = valueToY(value, chartData);

    painter->setFont(boldFont(painter->font(), CHART_METRIC_FONT_SIZE));
    painter->setPen(QPen(QColor("#c73535"), CHART_METRIC_PEN_WIDTH, Qt::DashLine));
    painter->drawLine(area->left(), y, area->right(), y);
    painter->drawText(area->left() + CHART_METRIC_TEXT_OFFSET_X,
                      y - CHART_METRIC_TEXT_OFFSET_Y,
                      label + ": " + QString::number(value, 'f', 2));
}

void drawChartMetrics(ChartData* chartData, const Metrics* metrics) {
    drawMetricLine(chartData, metrics->min, "Min");
    drawMetricLine(chartData, metrics->median, "Median");
    drawMetricLine(chartData, metrics->max, "Max");
}

void drawChartPolyline(ChartData* chartData, const List* points) {
    Iterator it = begin(points);
    int hasPreviousPoint = 0;

    chartData->painter->setPen(QPen(QColor("#1d4fc0"), CHART_POLYLINE_PEN_WIDTH));
    chartData->painter->setBrush(QColor("#17c2c2"));

    while (isSet(&it)) {
        const ChartPoint* point = static_cast<const ChartPoint*>(get(&it));

        if (point != nullptr) {
            updateScreenPoint(chartData, point);
            if (hasPreviousPoint)
                chartData->painter->drawLine(chartData->previousScreenPoint, chartData->screenPoint);
            chartData->painter->drawEllipse(chartData->screenPoint, CHART_POINT_RADIUS, CHART_POINT_RADIUS);
            chartData->previousScreenPoint = chartData->screenPoint;
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

int MainWindow::chartDataBounds(ChartData* chartData) const {
    Iterator it = begin(context.chartPoints);
    int hasPoints = 0;

    while (isSet(&it)) {
        const ChartPoint* point = static_cast<const ChartPoint*>(get(&it));

        if (point != nullptr) {
            if (!hasPoints) {
                chartData->minYear = point->year;
                chartData->maxYear = point->year;
                chartData->minValue = point->value;
                chartData->maxValue = point->value;
                hasPoints = 1;
            } else {
                if (point->year < chartData->minYear)
                    chartData->minYear = point->year;
                if (point->year > chartData->maxYear)
                    chartData->maxYear = point->year;
                if (point->value < chartData->minValue)
                    chartData->minValue = point->value;
                if (point->value > chartData->maxValue)
                    chartData->maxValue = point->value;
            }
        }

        next(&it);
    }

    if (hasPoints)
        updateChartSpans(chartData);

    return hasPoints;
}

void MainWindow::drawChart() {
    ChartData chartData = {0, 0, 0.0, 0.0, 0.0, 0.0, nullptr, nullptr, Point(), Point()};

    if (chartDataBounds(&chartData)) {
        addValuePadding(&chartData);

        QSize pixmapSize = chartPixmapSize(ui->chartLabel);
        double pixelRatio = ui->chartLabel->devicePixelRatioF();
        QPixmap pixmap(pixmapSize * pixelRatio);
        QRect area = chartArea(pixmapSize);

        pixmap.setDevicePixelRatio(pixelRatio);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        chartData.painter = &painter;
        chartData.area = &area;

        drawChartAxes(&chartData);
        drawChartMetrics(&chartData, &context.metrics);
        drawChartPolyline(&chartData, context.chartPoints);

        ui->chartLabel->setPixmap(pixmap);
        ui->ChartStackedWidget->setCurrentWidget(ui->chartPage);
    } else
        clearChart();
}
