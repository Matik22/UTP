#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QMap>

class ChartWidget : public QWidget
{
 Q_OBJECT

public:
 enum ChartType { Pie, Bar, Line };
 enum DataMode { ByGenre, ByAvailability };

 explicit ChartWidget(QWidget *parent = nullptr);
 void setData(const QMap<QString, double> &data);
 void setChartType(ChartType type);
 void setDataMode(DataMode mode);

 ChartType chartType() const { return m_chartType; }
 DataMode dataMode() const { return m_dataMode; }

protected:
 void paintEvent(QPaintEvent *) override;

private:
 QMap<QString, double> m_data;
 ChartType m_chartType;
 DataMode m_dataMode;

 void drawPie(QPainter &p);
 void drawBar(QPainter &p);
 void drawLine(QPainter &p);

 QList<QColor> m_colors;
};

#endif
