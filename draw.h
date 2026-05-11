#ifndef DRAW_H
#define DRAW_H

#include <QPainter>
#include <QPoint>
#include <QRect>

enum {
    CHART_DEFAULT_WIDTH = 760,
    CHART_DEFAULT_HEIGHT = 460,
    CHART_LABEL_MIN_WIDTH = 440,
    CHART_LABEL_MIN_HEIGHT = 320,

    CHART_LEFT_MARGIN = 70,
    CHART_TOP_MARGIN = 35,
    CHART_RIGHT_MARGIN = 30,
    CHART_BOTTOM_MARGIN = 55,

    CHART_POINT_RADIUS = 4,
    CHART_VALUE_PADDING_PERCENT = 8,

    CHART_AXIS_FONT_SIZE = 13,
    CHART_METRIC_FONT_SIZE = 12,
    CHART_AXIS_PEN_WIDTH = 2,
    CHART_POLYLINE_PEN_WIDTH = 3,
    CHART_METRIC_PEN_WIDTH = 1,

    CHART_YEAR_LABEL_OFFSET_Y = 25,
    CHART_MAX_YEAR_LABEL_OFFSET_X = 35,
    CHART_VALUE_LABEL_X = 12,
    CHART_MAX_VALUE_LABEL_OFFSET_Y = 5,
    CHART_X_AXIS_TITLE_OFFSET_X = 20,
    CHART_X_AXIS_TITLE_OFFSET_Y = 45,
    CHART_Y_AXIS_TITLE_OFFSET_X = 55,
    CHART_Y_AXIS_TITLE_OFFSET_Y = 12,
    CHART_METRIC_TEXT_OFFSET_X = 8,
    CHART_METRIC_TEXT_OFFSET_Y = 4
};

typedef QPoint Point;

typedef struct {
    int minYear;
    int maxYear;
    double minValue;
    double maxValue;
    double yearSpan;
    double valueSpan;
    QPainter* painter;
    const QRect* area;
    Point screenPoint;
    Point previousScreenPoint;
} ChartData;

#endif // DRAW_H
