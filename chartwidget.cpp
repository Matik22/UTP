#include "chartwidget.h"
#include <QPainter>
#include <QtMath>
#include <QPainterPath>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent), m_chartType(Pie), m_dataMode(ByGenre) {
 setMinimumSize(400, 450);
 m_colors = {
  QColor("#5B9BD5"), QColor("#ED7D31"), QColor("#70AD47"),
  QColor("#FFC000"), QColor("#9E9E9E"), QColor("#6A5ACD"),
  QColor("#C060A1"), QColor("#5C8A8A"), QColor("#FF6F61"),
  QColor("#6B8E23"), QColor("#00CED1"), QColor("#DC143C")
 };
}

void ChartWidget::setData(const QMap<QString, double> &data) {
 m_data = data;
 update();
}

void ChartWidget::setChartType(ChartType type) {
 m_chartType = type;
 update();
}

void ChartWidget::setDataMode(DataMode mode) {
 m_dataMode = mode;
 update();
}

void ChartWidget::paintEvent(QPaintEvent *) {
 QPainter p(this);
 p.setRenderHint(QPainter::Antialiasing);

 p.fillRect(rect(), QColor("#3c3c3c"));

 if (m_data.isEmpty()) {
  p.setPen(QColor("#cccccc"));
  p.setFont(QFont("Arial", 12));
  p.drawText(rect(), Qt::AlignCenter, "Нет данных");
  return;
 }

 switch (m_chartType) {
 case Pie: drawPie(p); break;
 case Bar: drawBar(p); break;
 case Line: drawLine(p); break;
 }
}

void ChartWidget::drawPie(QPainter &p) {
 double sum = 0;
 for (double v : m_data.values()) sum += v;
 if (sum <= 0) return;

 int legendW = 140;
 int totalW = qMin(width(), height());
 int chartW = totalW - legendW - 60;
 int legendX = (width() - totalW) / 2 + chartW + 40;
 int chartX = (width() - totalW) / 2 + 20;
 int chartY = (height() - chartW) / 2;

 QRectF chartRect(chartX, chartY, chartW, chartW);

 int i = 0;
 double start = 0;

 p.setPen(QPen(QColor("#3c3c3c"), 2));

 for (auto it = m_data.begin(); it != m_data.end(); ++it) {
  double percent = it.value() / sum;
  double angle = percent * 360 * 16;

  p.setBrush(m_colors[i % m_colors.size()]);
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
 for (auto it = m_data.begin(); it != m_data.end(); ++it) {
  QColor color = m_colors[i % m_colors.size()];
  double percent = it.value() / sum;

  p.setBrush(color);
  p.setPen(QPen(QColor("#888"), 1));
  p.drawRect(legendX, legendY + i * 22, 14, 14);

  p.setPen(QColor("#ffffff"));
  p.drawText(legendX + 20, legendY + i * 22 + 12,
   QString("%1 (%2% — %3)").arg(it.key()).arg(qRound(percent * 100)).arg(qRound(it.value())));
  i++;
 }
}

void ChartWidget::drawBar(QPainter &p) {
 double maxVal = 0;
 for (double v : m_data.values()) {
  if (v > maxVal) maxVal = v;
 }
 if (maxVal <= 0) return;

 int count = m_data.size();
 int marginL = 60, marginR = 30, marginT = 40, marginB = 60;
 int plotW = width() - marginL - marginR;
 int plotH = height() - marginT - marginB;
 int barW = qMin(plotW / count - 10, 60);
 int gap = (plotW - barW * count) / (count + 1);

 // Оси
 p.setPen(QPen(QColor("#aaaaaa"), 1));
 p.drawLine(marginL, marginT, marginL, height() - marginB);
 p.drawLine(marginL, height() - marginB, width() - marginR, height() - marginB);

 // Подписи осей
 p.setFont(QFont("Arial", 8));
 p.setPen(QColor("#aaaaaa"));

 int gridLines = 5;
 for (int g = 0; g <= gridLines; g++) {
  int y = marginT + (plotH * g) / gridLines;
  double val = maxVal * (1.0 - (double)g / gridLines);
  p.drawLine(marginL - 4, y, width() - marginR, y);
  p.drawText(0, y - 8, marginL - 6, 16, Qt::AlignRight | Qt::AlignVCenter, QString::number(qRound(val)));
 }

 // Столбцы
 int i = 0;
 p.setFont(QFont("Arial", 9));
 for (auto it = m_data.begin(); it != m_data.end(); ++it) {
  int x = marginL + gap + i * (barW + gap);
  int barH = (int)(plotH * it.value() / maxVal);
  int y = height() - marginB - barH;

  p.setBrush(m_colors[i % m_colors.size()]);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(x, y, barW, barH, 3, 3);

  // Значение над столбцом
  p.setPen(QColor("#ffffff"));
  p.drawText(x, y - 16, barW, 16, Qt::AlignCenter, QString::number(qRound(it.value())));

  // Подпись снизу
  p.setPen(QColor("#cccccc"));
  p.drawText(x - 10, height() - marginB + 4, barW + 20, 20, Qt::AlignCenter | Qt::TextWrapAnywhere, it.key());

  i++;
 }
}

void ChartWidget::drawLine(QPainter &p) {
 double maxVal = 0;
 for (double v : m_data.values()) {
  if (v > maxVal) maxVal = v;
 }
 if (maxVal <= 0) return;

 int count = m_data.size();
 if (count < 1) return;

 int marginL = 60, marginR = 30, marginT = 40, marginB = 60;
 int plotW = width() - marginL - marginR;
 int plotH = height() - marginT - marginB;
 int stepX = plotW / (count + 1);

 // Оси
 p.setPen(QPen(QColor("#aaaaaa"), 1));
 p.drawLine(marginL, marginT, marginL, height() - marginB);
 p.drawLine(marginL, height() - marginB, width() - marginR, height() - marginB);

 // Горизонтальная сетка
 p.setFont(QFont("Arial", 8));
 p.setPen(QColor("#555555"));

 int gridLines = 5;
 for (int g = 0; g <= gridLines; g++) {
  int y = marginT + (plotH * g) / gridLines;
  double val = maxVal * (1.0 - (double)g / gridLines);
  p.drawLine(marginL, y, width() - marginR, y);
  p.setPen(QColor("#aaaaaa"));
  p.drawText(0, y - 8, marginL - 6, 16, Qt::AlignRight | Qt::AlignVCenter, QString::number(qRound(val)));
  p.setPen(QColor("#555555"));
 }

 // Точки и линия
 QVector<QPoint> points;
 int i = 1;
 for (auto it = m_data.begin(); it != m_data.end(); ++it) {
  int x = marginL + i * stepX;
  int y = height() - marginB - (int)(plotH * it.value() / maxVal);
  points.append(QPoint(x, y));
  i++;
 }

 // Линия
 p.setPen(QPen(QColor("#5B9BD5"), 2));
 if (points.size() > 1) {
  for (int j = 0; j < points.size() - 1; j++) {
   p.drawLine(points[j], points[j + 1]);
  }
 }

 // Заливка под линией
 if (points.size() > 1) {
  QPainterPath fillPath;
  fillPath.moveTo(points[0].x(), height() - marginB);
  for (int j = 0; j < points.size(); j++) {
   fillPath.lineTo(points[j]);
  }
  fillPath.lineTo(points.last().x(), height() - marginB);
  fillPath.closeSubpath();
  QColor fillColor("#5B9BD5");
  fillColor.setAlpha(40);
  p.setPen(Qt::NoPen);
  p.setBrush(fillColor);
  p.drawPath(fillPath);
 }

 // Точки и подписи
 i = 0;
 p.setFont(QFont("Arial", 9));
 for (auto it = m_data.begin(); it != m_data.end(); ++it) {
  if (i < points.size()) {
   p.setBrush(m_colors[i % m_colors.size()]);
   p.setPen(QPen(QColor("#ffffff"), 1));
   p.drawEllipse(points[i], 5, 5);

   p.setPen(QColor("#ffffff"));
   p.drawText(points[i].x() - 20, points[i].y() - 14, 40, 14, Qt::AlignCenter, QString::number(qRound(it.value())));

   p.setPen(QColor("#cccccc"));
   p.drawText(points[i].x() - 30, height() - marginB + 4, 60, 20, Qt::AlignCenter | Qt::TextWrapAnywhere, it.key());
  }
  i++;
 }
}
