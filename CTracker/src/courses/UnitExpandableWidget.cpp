#include "courses/UnitExpandableWidget.h"

#include "courses/SessionTaskRow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

UnitExpandableWidget::UnitExpandableWidget(int unitId,
                                           const QString& name,
                                           QWidget* parent)
    : QWidget(parent),
      m_unitId(unitId),
      m_name(name) {
    setupUi();
    setExpanded(false);
}

void UnitExpandableWidget::setupUi() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* header = new QWidget(this);
    header->setObjectName("unitHeader");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 6, 8, 6);
    headerLayout->setSpacing(8);

    m_expandButton = new QPushButton(QStringLiteral("\u25B6"), header);   // ▶
    m_expandButton->setFlat(true);
    m_expandButton->setFixedWidth(24);

    m_nameLabel     = new QLabel(m_name, header);
    m_progressLabel = new QLabel("0%", header);
    m_progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_progressLabel->setMinimumWidth(48);

    headerLayout->addWidget(m_expandButton);
    headerLayout->addWidget(m_nameLabel, 1);
    headerLayout->addWidget(m_progressLabel);

    m_content = new QWidget(this);
    m_contentLayout = new QVBoxLayout(m_content);
    m_contentLayout->setContentsMargins(24, 0, 8, 8);
    m_contentLayout->setSpacing(2);

    outer->addWidget(header);
    outer->addWidget(m_content);

    connect(m_expandButton, &QPushButton::clicked,
            this, &UnitExpandableWidget::onExpandClicked);
}

void UnitExpandableWidget::setExpanded(bool expanded) {
    if (m_expanded == expanded) {
        return;  // Already in the desired state
    }
    m_expanded = expanded;
    m_content->setVisible(expanded);
    m_expandButton->setText(expanded ? QStringLiteral("\u25BC")   // ▼
                                     : QStringLiteral("\u25B6")); // ▶
    emit expandStateChanged(m_expanded);
}

void UnitExpandableWidget::setName(const QString& newName) {
    m_name = newName;
    m_nameLabel->setText(newName);
}

void UnitExpandableWidget::onExpandClicked() {
    setExpanded(!m_expanded);
}

void UnitExpandableWidget::addSessionTask(int sessionId,
                                          const QString& name,
                                          int progress) {
    if (m_rows.contains(sessionId)) {
        updateSessionTaskProgress(sessionId, progress);
        return;
    }
    auto* row = new SessionTaskRow(sessionId, name, progress, m_content);
    m_contentLayout->addWidget(row);
    m_rows.insert(sessionId, row);

    connect(row, &SessionTaskRow::progressChanged,
            this, &UnitExpandableWidget::onChildProgressChanged);
    connect(row, &SessionTaskRow::nameChanged,
            this, &UnitExpandableWidget::onChildNameChanged);

    refreshOverall();
}

void UnitExpandableWidget::removeSessionTask(int sessionId) {
    auto it = m_rows.find(sessionId);
    if (it == m_rows.end()) {
        return;
    }
    SessionTaskRow* row = it.value();
    if (row) {
        disconnect(row, nullptr, this, nullptr);  // Disconnect signals first
        m_contentLayout->removeWidget(row);
        row->deleteLater();
    }
    m_rows.erase(it);
    refreshOverall();
}

void UnitExpandableWidget::updateSessionTaskProgress(int sessionId, int progress) {
    auto it = m_rows.constFind(sessionId);
    if (it == m_rows.constEnd()) {
        return;
    }
    it.value()->setProgress(progress);
    refreshOverall();
}

int UnitExpandableWidget::calculateOverallProgress() const {
    if (m_rows.isEmpty()) {
        return 0;
    }
    int total = 0;
    for (auto* row : m_rows) {
        total += row->progress();
    }
    return total / m_rows.size();
}

void UnitExpandableWidget::refreshOverall() {
    m_progressLabel->setText(QString("%1%").arg(calculateOverallProgress()));
}

void UnitExpandableWidget::onChildProgressChanged(int oldValue, int newValue) {
    auto* row = qobject_cast<SessionTaskRow*>(sender());
    if (!row) {
        return;
    }
    refreshOverall();
    emit sessionTaskProgressChanged(row->sessionId(), oldValue, newValue);
}

void UnitExpandableWidget::onChildNameChanged(const QString& newName) {
    auto* row = qobject_cast<SessionTaskRow*>(sender());
    if (!row) {
        return;
    }
    emit sessionTaskRenamed(row->sessionId(), newName);
}
