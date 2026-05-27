#include "projects/TaskBoardCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QDate>

// ============================================================
//  Local helpers
// ============================================================
namespace {

// Left-edge stripe color (matches the design palette / mockup).
QString stripeColorForPriority(const QString& p) {
    if (p == "high")   return "#ef4444";
    if (p == "medium") return "#f59e0b";
    return "#6b7280";   // low / unknown
}

// Background + foreground for the due-date pill.
struct PillStyle { QString bg; QString fg; QString text; };

PillStyle dueStyle(const QString& status, const QDate& due) {
    if (status == "done") {
        return { "rgba(107, 114, 128, 0.20)", "#9ca3af", "done" };
    }
    if (!due.isValid()) {
        return { "rgba(107, 114, 128, 0.20)", "#9ca3af", "No date" };
    }
    const int daysLeft = QDate::currentDate().daysTo(due);
    QString label;
    if      (daysLeft <  0) label = QStringLiteral("%1d overdue").arg(-daysLeft);
    else if (daysLeft == 0) label = QStringLiteral("today");
    else                    label = QStringLiteral("in %1d").arg(daysLeft);

    if (daysLeft <= 3) return { "rgba(239, 68, 68, 0.15)",  "#ef4444", label };
    if (daysLeft <= 7) return { "rgba(245, 158, 11, 0.15)", "#f59e0b", label };
    return                       { "rgba(107, 114, 128, 0.20)", "#9ca3af", label };
}

} // namespace

// ============================================================
//  TaskBoardCard
// ============================================================
TaskBoardCard::TaskBoardCard(int taskId, QWidget* parent)
    : QFrame(parent),
      m_taskId(taskId) {
    setObjectName("taskBoardCard");
    setFrameShape(QFrame::StyledPanel);
    setCursor(Qt::PointingHandCursor);
    // Slim, dense kanban card — explicit min height so empty cards still look like cards.
    setMinimumHeight(64);
    setupUi();
    applyStripeColor();
    refreshDuePill();
}

void TaskBoardCard::setupUi() {
    auto* layout = new QVBoxLayout(this);
    // Extra left padding so the colored stripe (drawn via stylesheet
    // border-left) doesn't overlap the title text.
    layout->setContentsMargins(12, 8, 10, 8);
    layout->setSpacing(6);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("taskCardTitle");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet(
        "QLabel#taskCardTitle { color: #e4e6eb; font-size: 13px; font-weight: 500; }");
    layout->addWidget(m_titleLabel);

    // Tiny one-line description preview shown below the title when the
    // task carries a description. Helps users scan a column without
    // opening each task. Hidden by default — set by setDescription().
    m_descLabel = new QLabel(this);
    m_descLabel->setObjectName("taskCardDesc");
    m_descLabel->setStyleSheet(
        "QLabel#taskCardDesc { color: #9ca3af; font-size: 11px; }");
    m_descLabel->setVisible(false);
    layout->addWidget(m_descLabel);

    auto* metaRow = new QHBoxLayout();
    metaRow->setContentsMargins(0, 0, 0, 0);
    metaRow->setSpacing(6);

    m_unitChip = new QLabel(this);
    m_unitChip->setObjectName("taskCardUnit");
    m_unitChip->setStyleSheet(
        "QLabel#taskCardUnit {"
        "  background: #1a1d24; color: #9ca3af;"
        "  padding: 2px 6px; border-radius: 4px; font-size: 10px;"
        "}");
    m_unitChip->setVisible(false);

    m_duePill = new QLabel(this);
    m_duePill->setObjectName("taskCardDue");

    metaRow->addWidget(m_unitChip);
    metaRow->addStretch();
    metaRow->addWidget(m_duePill);
    layout->addLayout(metaRow);
}

void TaskBoardCard::setTitle(const QString& title) {
    if (m_titleLabel) m_titleLabel->setText(title);
}

void TaskBoardCard::setUnitName(const QString& unit) {
    if (!m_unitChip) return;
    if (unit.isEmpty()) {
        m_unitChip->setVisible(false);
    } else {
        m_unitChip->setText(unit);
        m_unitChip->setVisible(true);
    }
}

void TaskBoardCard::setStatus(const QString& status) {
    m_status = status;
    refreshDuePill();
}

void TaskBoardCard::setDueDate(const QDate& due) {
    m_dueDate = due;
    refreshDuePill();
}

void TaskBoardCard::setPriority(const QString& priority) {
    m_priority = priority;
    applyStripeColor();
}

void TaskBoardCard::setDescription(const QString& description) {
    if (!m_descLabel) return;
    
    // Grab the first line of the description, trimming whitespace
    QString firstLine = description.trimmed().section('\n', 0, 0);
    
    if (firstLine.isEmpty()) {
        m_descLabel->setVisible(false);
    } else {
        // Optional: truncate if extremely long, though UI usually elides it via rich text or fixed widths
        // But let's just show it and let the layout/stylesheet handle it. We can manually elide it if needed.
        // Doing a simple manual elide:
        QFontMetrics fm(m_descLabel->font());
        // Since we don't know the exact width until layout, just set it and let word wrap or generic truncation happen.
        m_descLabel->setText(firstLine);
        m_descLabel->setVisible(true);
    }
}

void TaskBoardCard::applyStripeColor() {
    const QString stripe = stripeColorForPriority(m_priority);
    setStyleSheet(QStringLiteral(
        "QFrame#taskBoardCard {"
        "  background: #252932;"
        "  border: 1px solid #2d323d;"
        "  border-left: 3px solid %1;"
        "  border-radius: 8px;"
        "}"
        "QFrame#taskBoardCard[hover=\"true\"] {"
        "  border-color: #10b981;"
        "  border-left: 3px solid %1;"
        "}"
    ).arg(stripe));
}

void TaskBoardCard::refreshDuePill() {
    if (!m_duePill) return;
    const PillStyle p = dueStyle(m_status, m_dueDate);
    m_duePill->setText(p.text);
    m_duePill->setStyleSheet(QStringLiteral(
        "QLabel#taskCardDue {"
        "  background: %1; color: %2;"
        "  padding: 2px 6px; border-radius: 4px; font-size: 11px;"
        "}").arg(p.bg, p.fg));
}

void TaskBoardCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_taskId);
    }
    QFrame::mousePressEvent(event);
}

void TaskBoardCard::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);

    // Quick status switcher
    auto* moveMenu = menu.addMenu(tr("Move to"));
    struct Entry { const char* label; const char* status; };
    const Entry entries[] = {
        { "Todo",        "todo" },
        { "In progress", "in_progress" },
        { "Review",      "review" },
        { "Done",        "done" },
    };
    for (const auto& e : entries) {
        QAction* act = moveMenu->addAction(tr(e.label));
        act->setCheckable(true);
        act->setChecked(m_status == QLatin1String(e.status));
        const QString target = QString::fromLatin1(e.status);
        connect(act, &QAction::triggered, this, [this, target] {
            emit statusChangeRequested(m_taskId, target);
        });
    }

    menu.addSeparator();
    QAction* deleteAct = menu.addAction(tr("Delete task"));
    connect(deleteAct, &QAction::triggered, this, [this] {
        emit deleteRequested(m_taskId);
    });

    menu.exec(event->globalPos());
}

void TaskBoardCard::enterEvent(QEnterEvent* event) {
    setProperty("hover", true);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::enterEvent(event);
}

void TaskBoardCard::leaveEvent(QEvent* event) {
    setProperty("hover", false);
    style()->unpolish(this);
    style()->polish(this);
    QFrame::leaveEvent(event);
}
