#include "piechartwidget.h"
#include <QPainter>
#include <QtMath>

PieChartWidget::PieChartWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(300, 450);
}

void PieChartWidget::setData(const QMap<QString, double> &data)
{
    m_data = data;
    update();
}

void PieChartWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_data.isEmpty()) {
        p.setPen(QColor("#cccccc"));
        p.setFont(QFont("Arial", 12));
        p.drawText(rect(), Qt::AlignCenter, "Нет данных");
        return;
    }

    double sum = 0;
    for (double v : m_data.values()) sum += v;
    if (sum <= 0) return;

    QList<QColor> colors = {
        QColor("#5B9BD5"), QColor("#ED7D31"), QColor("#70AD47"),
        QColor("#FFC000"), QColor("#9E9E9E"), QColor("#6A5ACD"),
        QColor("#C060A1"), QColor("#5C8A8A")
    };


    int legendW = 120;
    int totalW = qMin(width(), height());
    int chartW = totalW - legendW - 40;
    int legendX = (width() - totalW) / 2 + chartW + 30;
    int chartX = (width() - totalW) / 2 + 10;
    int chartY = (height() - chartW) / 2;

    QRectF chartRect(chartX, chartY, chartW, chartW);

    int i = 0;
    double start = 0;

    p.setPen(QPen(Qt::white, 1));

    for (auto it = m_data.begin(); it != m_data.end(); ++it)
    {
        double percent = it.value() / sum;
        double angle = percent * 360 * 16;

        p.setBrush(colors[i % colors.size()]);
        p.drawPie(chartRect, -start, -angle);

        start += angle;
        i++;
    }

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#888"), 1));
    p.drawEllipse(chartRect);

    int legendY = chartRect.top() + 5;

    p.setFont(QFont("Arial", 9));

    i = 0;
    for (auto it = m_data.begin(); it != m_data.end(); ++it)
    {
        QColor color = colors[i % colors.size()];
        double percent = it.value() / sum;

        p.setBrush(color);
        p.setPen(QPen(QColor("#888"), 1));
        p.drawRect(legendX, legendY + i * 22, 14, 14);

        p.setPen(QColor("#ffffff"));
        p.drawText(legendX + 20, legendY + i * 22 + 12,
                   QString("%1 (%2%)").arg(it.key()).arg(qRound(percent * 100)));
        i++;
    }
}