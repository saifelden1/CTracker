#pragma once

#include <QWidget>
#include <QDate>

class QLabel;
class QPushButton;
class ContributionHeatmap;
class ActivityLogModel;

class AnalyticsView : public QWidget {
    Q_OBJECT
public:
    explicit AnalyticsView(ActivityLogModel* model, QWidget* parent = nullptr);

    void loadYear(int year);
    int  currentYear() const { return m_year; }

public slots:
    void onDataChanged();

private slots:
    void onPreviousYear();
    void onNextYear();

private:
    void setupUi();
    void rebuildLegend(QWidget* container);

    ActivityLogModel*    m_model     = nullptr;
    int                  m_year      = QDate::currentDate().year();
    QPushButton*         m_prevBtn   = nullptr;
    QPushButton*         m_nextBtn   = nullptr;
    QLabel*              m_yearLabel = nullptr;
    ContributionHeatmap* m_heatmap   = nullptr;
};
