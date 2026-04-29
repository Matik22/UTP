#ifndef PIECHARTWIDGET_H
#define PIECHARTWIDGET_H

#include <QWidget>
#include <QMap>

class PieChartWidget : public QWidget
{

public:
    explicit PieChartWidget(QWidget *parent = nullptr);
    void setData(const QMap<QString, double> &data);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QMap<QString, double> m_data;
};

#endif