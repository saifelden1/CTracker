#include "AnalyticsView.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QMap>
#include <algorithm>
#include <cmath>

#include "ContributionHeatmap.h"
#include "ActivityLogModel.h"
#include "DatabaseManager.h"

AnalyticsView::AnalyticsView(ActivityLogModel* model, QWidget* parent)
    : QWidget(parent),
      m_model(model) {
    setupUi();
    if (auto* db = DatabaseManager::instance()) {
        connect(db, &DatabaseManager::dataChanged,
                this, &AnalyticsView::onDataChanged);
    }
    loadYear(m_year);
}

void AnalyticsView::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);
    outer->setSpacing(12);

    auto* title = new QLabel(tr("Analytics"), this);
    title->setObjectName("analyticsTitle");
    outer->addWidget(title);

    // ---- Year navigation row ----
    auto* navRow = new QWidget(this);
    auto* navLayout = new QHBoxLayout(navRow);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(8);

    m_prevBtn   = new QPushButton(QStringLiteral("\u25C0"), navRow);   // ◀
    m_nextBtn   = new QPushButton(QStringLiteral("\u25B6"), navRow);   // ▶
    m_yearLabel = new QLabel(QString::number(m_year), navRow);
    m_yearLabel->setObjectName("yearLabel");
    m_yearLabel->setAlignment(Qt::AlignCenter);
    m_yearLabel->setMinimumWidth(80);

    m_prevBtn->setFlat(true);
    m_nextBtn->setFlat(true);

    navLayout->addStretch(1);
    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_yearLabel);
    navLayout->addWidget(m_nextBtn);
    navLayout->addStretch(1);

    outer->addWidget(navRow);

    // ---- Heatmap ----
    m_heatmap = new ContributionHeatmap(this);
    outer->addWidget(m_heatmap, 0, Qt::AlignHCenter);

    // ---- Legend ----
    auto* legend = new QFrame(this);
    legend->setObjectName("heatmapLegend");
    rebuildLegend(legend);
    outer->addWidget(legend, 0, Qt::AlignHCenter);

    outer->addStretch(1);

    connect(m_prevBtn, &QPushButton::clicked, this, &AnalyticsView::onPreviousYear);
    connect(m_nextBtn, &QPushButton::clicked, this, &AnalyticsView::onNextYear);
}

void AnalyticsView::rebuildLegend(QWidget* container) {
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(6);

    layout->addStretch(1);
    layout->addWidget(new QLabel(tr("Less"), container));

    static const char* colors[5] = {
        "#161b22", "#0e4429", "#006d32", "#26a641", "#39d353"
    };
    for (const char* hex : colors) {
        auto* swatch = new QLabel(container);
        swatch->setFixedSize(12, 12);
        swatch->setStyleSheet(QString("background: %1; border-radius: 2px;").arg(hex));
        layout->addWidget(swatch);
    }
    layout->addWidget(new QLabel(tr("More"), container));
    layout->addStretch(1);
}

void AnalyticsView::loadYear(int year) {
    m_year = year;
    m_yearLabel->setText(QString::number(year));
    m_heatmap->setYear(year);

    if (!m_model) {
        return;
    }

    const QDate from(year, 1, 1);
    const QDate to(year, 12, 31);

    const QMap<QDate, int> totals = m_model->getDailyProgressTotals(from, to);
    const QMap<QDate, int> counts = m_model->getDailyActivityCounts(from, to);

    int maxTotal = 0;
    for (int v : totals) {
        if (v > maxTotal) {
            maxTotal = v;
        }
    }

    QMap<QDate, ContributionHeatmap::DayData> data;
    for (auto it = totals.constBegin(); it != totals.constEnd(); ++it) {
        ContributionHeatmap::DayData d;
        d.date = it.key();
        d.totalProgress  = it.value();
        d.activityCount  = counts.value(it.key(), 0);
        d.intensityLevel = 0;
        if (maxTotal > 0 && d.totalProgress > 0) {
            d.intensityLevel = std::clamp(
                static_cast<int>(std::floor((double(d.totalProgress) / double(maxTotal)) * 4.0)),
                0, 4);
            if (d.intensityLevel == 0) {
                d.intensityLevel = 1; // any activity is at least bucket 1
            }
        }
        data.insert(d.date, d);
    }

    m_heatmap->setData(data);
}

void AnalyticsView::onPreviousYear() {
    loadYear(m_year - 1);
}

void AnalyticsView::onNextYear() {
    loadYear(m_year + 1);
}

void AnalyticsView::onDataChanged() {
    if (m_model) {
        m_model->refresh();
    }
    loadYear(m_year);
}
