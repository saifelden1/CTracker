#include "courses/CourseDetailView.h"

#include "courses/UnitCard.h"
#include "courses/UnitSessionsView.h"
#include "core/DatabaseManager.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QFrame>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QCursor>

// ============================================================
//  CourseDetailView — Task 7.5 + 7.5a
//
//  Course-specific detail page. Extends the shared EntityDetailView
//  scaffolding with:
//
//    1. Title-bar chrome (status badge, Pause/Resume toggle,
//       session-count subtitle) — same as before.
//
//    2. NEW (Task 7.5a): the content area is now an inner
//       QStackedWidget. Page 0 holds a responsive grid of UnitCard
//       widgets (plus a "+ New Unit" tile); Page 1 holds a
//       UnitSessionsView focused on one unit.
//
//  The base class's m_scrollArea + m_unitsContainer are hidden — the
//  base layout is reused as a parent surface, but its scroll-area-of-
//  UnitExpandableWidgets pathway is bypassed for courses. (Projects
//  still use it through ProjectDetailView.)
// ============================================================

namespace {

// "+ New Unit" tile — a QPushButton subclass styled as a card.
// QPushButton already has clicked(); we just customise the visual.
// No Q_OBJECT needed in this translation unit because we only use
// the existing clicked() signal of the QPushButton base class.
class AddUnitTile : public QPushButton {
public:
    explicit AddUnitTile(QWidget* parent = nullptr) : QPushButton(parent) {
        setObjectName("addUnitTile");
        setFlat(true);
        setFixedSize(180, 200);
        setCursor(Qt::PointingHandCursor);
        // The "+ New Unit" text + glyph live in the button's own
        // rich-text-like label via setText with HTML so we don't need
        // a separate QLabel child (and don't need to override paint).
        setText(QStringLiteral(
            "<div style='text-align:center;'>"
            "<div style='color:#10b981; font-size:36px; font-weight:300;'>+</div>"
            "<div style='color:#10b981; font-size:13px; font-weight:500; margin-top:4px;'>"
            "New Unit"
            "</div>"
            "</div>"));
        setStyleSheet(QStringLiteral(
            "QPushButton#addUnitTile {"
            "  background: rgba(16, 185, 129, 0.04);"
            "  border: 1px dashed #10b981;"
            "  border-radius: 8px;"
            "  text-align: center;"
            "}"
            "QPushButton#addUnitTile:hover {"
            "  background: rgba(16, 185, 129, 0.10);"
            "  border-color: #059669;"
            "}"
        ));
    }
};

} // namespace

// ------------------------------------------------------------

CourseDetailView::CourseDetailView(QWidget* parent)
    : EntityDetailView(EntityCard::EntityType::Course, parent) {
    setupCourseChrome();
    buildInnerStack();
}

// ── Title-bar chrome ────────────────────────────────────────
void CourseDetailView::setupCourseChrome() {
    // ── Status badge (inserted after title label) ──
    m_statusBadge = new QLabel(m_titleBar);
    m_statusBadge->setObjectName("courseStatusBadge");
    m_statusBadge->setFixedHeight(24);
    m_statusBadge->setAlignment(Qt::AlignCenter);
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#courseStatusBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: #10b981;"
            "}"));
    m_titleLayout->insertWidget(2, m_statusBadge);

    // ── Pause/Resume toggle button ──
    m_statusToggleBtn = new QPushButton(tr("Pause Course"), m_titleBar);
    m_statusToggleBtn->setObjectName("statusToggleBtn");
    m_statusToggleBtn->setStyleSheet(
        QStringLiteral(
            "QPushButton#statusToggleBtn {"
            "  padding: 6px 16px;"
            "  border-radius: 6px;"
            "  border: 1px solid #2d323d;"
            "  background: #252932;"
            "  color: #e4e6eb;"
            "  font-weight: 500;"
            "}"
            "QPushButton#statusToggleBtn:hover {"
            "  background: #2d323d;"
            "}"));
    m_titleLayout->insertWidget(3, m_statusToggleBtn);

    connect(m_statusToggleBtn, &QPushButton::clicked,
            this, &CourseDetailView::onStatusToggleClicked);

    // ── Session count subtitle ──
    m_sessionCountLabel = new QLabel(m_titleBar);
    m_sessionCountLabel->setObjectName("sessionCountLabel");
    m_sessionCountLabel->setStyleSheet(
        QStringLiteral("QLabel#sessionCountLabel { color: #9ca3af; font-size: 13px; }"));
    m_outerLayout->insertWidget(1, m_sessionCountLabel);

    // ── Hide the base class's "+ Unit" / "+ Session" buttons ──
    // We replace them with the tile + sub-view's "+ Session" button.
    if (m_addUnitBtn)    m_addUnitBtn->setVisible(false);
    if (m_addSessionBtn) m_addSessionBtn->setVisible(false);
}

// ── Inner QStackedWidget ────────────────────────────────────
void CourseDetailView::buildInnerStack() {
    // Hide the base class's scroll-area-of-UnitExpandableWidgets path —
    // we don't use it. We leave it parented for ProjectDetailView's
    // sake; CourseDetailView simply doesn't render it.
    if (m_scrollArea) {
        m_scrollArea->setVisible(false);
        m_outerLayout->removeWidget(m_scrollArea);
    }

    m_innerStack = new QStackedWidget(this);
    m_outerLayout->addWidget(m_innerStack, 1);

    // ── Page 0: Units grid ─────────────────────────────────
    m_unitsPage = new QWidget(m_innerStack);
    auto* pageLayout = new QVBoxLayout(m_unitsPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    m_unitsScroll = new QScrollArea(m_unitsPage);
    m_unitsScroll->setWidgetResizable(true);
    m_unitsScroll->setFrameShape(QFrame::NoFrame);

    m_unitsContainerInner = new QWidget(m_unitsScroll);
    m_unitsGrid = new QGridLayout(m_unitsContainerInner);
    m_unitsGrid->setContentsMargins(4, 4, 4, 4);
    m_unitsGrid->setHorizontalSpacing(16);
    m_unitsGrid->setVerticalSpacing(16);
    m_unitsGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_unitsScroll->setWidget(m_unitsContainerInner);
    pageLayout->addWidget(m_unitsScroll, 1);

    m_innerStack->addWidget(m_unitsPage);

    // ── Page 1: Sessions sub-view ──────────────────────────
    m_sessionsView = new UnitSessionsView(m_innerStack);
    m_innerStack->addWidget(m_sessionsView);

    connect(m_sessionsView, &UnitSessionsView::backRequested,
            this, &CourseDetailView::onSubViewBackRequested);
    connect(m_sessionsView, &UnitSessionsView::sessionProgressChanged,
            this, &CourseDetailView::onSubViewSessionProgressChanged);
    connect(m_sessionsView, &UnitSessionsView::sessionRenamed,
            this, &CourseDetailView::onSubViewSessionRenamed);
    // sessionAdded / unitRenamed / unitRemoved arrive as dataChanged()
    // from the DB layer (the sub-view's add/rename/delete actions call
    // through DatabaseManager which emits dataChanged); onDataChanged
    // below rebuilds the grid and refreshes the sub-view in turn.

    m_innerStack->setCurrentWidget(m_unitsPage);
}

// ── EntityDetailView overrides ──────────────────────────────
void CourseDetailView::loadEntity(int entityId) {
    EntityDetailView::loadEntity(entityId);

    if (m_entityId < 0) {
        m_courseStatus = "active";
        refreshStatusUI();
        clearUnitCards();
        showUnitsPage();
        return;
    }

    auto* db = DatabaseManager::instance();
    m_courseStatus = db->getCourseStatus(m_entityId);
    refreshStatusUI();
    rebuildUnitsGrid();
    // Always return to the grid when loading a new entity — the
    // previously-selected unit may not even belong to this course.
    showUnitsPage();
}

void CourseDetailView::onDataChanged() {
    EntityDetailView::onDataChanged();

    if (m_entityId < 0) {
        m_courseStatus = "active";
        refreshStatusUI();
        clearUnitCards();
        showUnitsPage();
        return;
    }

    auto* db = DatabaseManager::instance();
    m_courseStatus = db->getCourseStatus(m_entityId);
    refreshStatusUI();
    rebuildUnitsGrid();

    // If we're inside the sessions sub-view, ask it to refresh its
    // own slice. refresh() will bail back to the grid if the unit
    // has been removed externally.
    if (m_innerStack && m_innerStack->currentWidget() == m_sessionsView
        && m_sessionsView) {
        m_sessionsView->refresh();
    }
}

// ── Status badge / Pause toggle ─────────────────────────────
void CourseDetailView::onStatusToggleClicked() {
    if (m_entityId < 0) {
        return;
    }
    QString newStatus;
    if (m_courseStatus == "active") {
        newStatus = "paused";
    } else if (m_courseStatus == "paused") {
        newStatus = "active";
    } else {
        newStatus = "active";
    }
    auto* db = DatabaseManager::instance();
    if (db->setCourseStatus(m_entityId, newStatus)) {
        m_courseStatus = newStatus;
        refreshStatusUI();
    }
}

void CourseDetailView::refreshStatusUI() {
    // ── Status badge ──
    QString badgeText, badgeColor;
    if (m_courseStatus == "active") {
        badgeText  = tr("Active");
        badgeColor = QStringLiteral("#10b981");
    } else if (m_courseStatus == "paused") {
        badgeText  = tr("Paused");
        badgeColor = QStringLiteral("#f59e0b");
    } else {
        badgeText  = tr("Completed");
        badgeColor = QStringLiteral("#3b82f6");
    }
    m_statusBadge->setText(badgeText);
    m_statusBadge->setStyleSheet(
        QStringLiteral(
            "QLabel#courseStatusBadge {"
            "  padding: 2px 8px;"
            "  border-radius: 4px;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  color: #e4e6eb;"
            "  background: %1;"
            "}").arg(badgeColor));

    // ── Toggle button text ──
    if (m_courseStatus == "active") {
        m_statusToggleBtn->setText(tr("Pause Course"));
    } else if (m_courseStatus == "paused") {
        m_statusToggleBtn->setText(tr("Resume Course"));
    } else {
        m_statusToggleBtn->setText(tr("Re-activate Course"));
    }

    // ── Session count subtitle ──
    if (m_entityId < 0) {
        m_sessionCountLabel->setText(QString());
        return;
    }
    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_entityId);
    int totalSessions = 0;
    for (const UnitData& u : units) {
        totalSessions += db->getSessionTasksForUnit(u.id).size();
    }
    m_sessionCountLabel->setText(
        tr("%1 sessions across %2 units")
            .arg(totalSessions)
            .arg(units.size()));
}

// ── Units grid (Page 0) ─────────────────────────────────────
void CourseDetailView::clearUnitCards() {
    for (UnitCard* c : m_unitCards) {
        if (!c) continue;
        disconnect(c, nullptr, this, nullptr);
        m_unitsGrid->removeWidget(c);
        c->deleteLater();
    }
    m_unitCards.clear();
    if (m_addUnitTile) {
        m_unitsGrid->removeWidget(m_addUnitTile);
        m_addUnitTile->deleteLater();
        m_addUnitTile = nullptr;
    }
}

void CourseDetailView::rebuildUnitsGrid() {
    clearUnitCards();
    if (m_entityId < 0) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const QList<UnitData> units = db->getUnitsForParent(m_entityId);

    for (const UnitData& u : units) {
        auto* card = new UnitCard(u.id, u.name, m_unitsContainerInner);

        // Compute progress + counts from the unit's sessions
        const QList<SessionTaskData> sessions = db->getSessionTasksForUnit(u.id);
        int sum = 0, completed = 0;
        for (const SessionTaskData& s : sessions) {
            sum += s.progress;
            if (s.progress >= 100) ++completed;
        }
        const int progress = sessions.isEmpty() ? 0 : sum / sessions.size();
        card->setProgress(progress);
        card->setSessionCounts(sessions.size(), completed);
        card->setLastActivity(db->lastActivityForUnit(u.id));

        connect(card, &UnitCard::clicked,
                this, &CourseDetailView::onUnitCardClicked);

        m_unitCards.append(card);
    }

    // Append the "+ New Unit" tile at the end.
    auto* tile = new AddUnitTile(m_unitsContainerInner);
    m_addUnitTile = tile;
    connect(tile, &QPushButton::clicked,
            this, &CourseDetailView::onAddUnitTileClicked);

    updateUnitColumnCount();   // also lays out cards into the grid
}

void CourseDetailView::updateUnitColumnCount() {
    if (!m_unitsContainerInner) {
        return;
    }
    const int w = m_unitsScroll ? m_unitsScroll->viewport()->width() : width();
    int cols = 4;
    if      (w < 480)  cols = 1;
    else if (w < 720)  cols = 2;
    else if (w < 1000) cols = 3;
    else               cols = 4;
    m_unitColumnCount = cols;

    // Re-pack the grid. Remove without delete; we own the widgets.
    while (m_unitsGrid->count() > 0) {
        QLayoutItem* item = m_unitsGrid->takeAt(0);
        if (item) {
            // Item just goes; the widget pointers are still alive on
            // their parent (m_unitsContainerInner).
            delete item;
        }
    }
    int row = 0, col = 0;
    auto place = [&](QWidget* w) {
        m_unitsGrid->addWidget(w, row, col);
        if (++col >= cols) { col = 0; ++row; }
    };
    for (UnitCard* c : m_unitCards) place(c);
    if (m_addUnitTile) place(m_addUnitTile);
}

void CourseDetailView::resizeEvent(QResizeEvent* event) {
    EntityDetailView::resizeEvent(event);
    updateUnitColumnCount();
}

void CourseDetailView::showUnitsPage() {
    if (m_innerStack) {
        m_innerStack->setCurrentWidget(m_unitsPage);
    }
}

void CourseDetailView::showSessionsPage(int unitId) {
    if (!m_sessionsView || !m_innerStack) {
        return;
    }
    m_sessionsView->loadUnit(unitId);
    m_innerStack->setCurrentWidget(m_sessionsView);
}

void CourseDetailView::onUnitCardClicked(int unitId) {
    showSessionsPage(unitId);
}

void CourseDetailView::onAddUnitTileClicked() {
    if (m_entityId < 0) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(
        this, tr("New Unit"), tr("Unit name:"),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }
    auto* db = DatabaseManager::instance();
    const int unitId = db->addUnit(m_entityId, trimmed);
    if (unitId < 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to add unit"));
        return;
    }
    // dataChanged → onDataChanged → rebuildUnitsGrid (new card appears).
}

// ── Sub-view re-emission ────────────────────────────────────
void CourseDetailView::onSubViewBackRequested() {
    showUnitsPage();
}

void CourseDetailView::onSubViewSessionProgressChanged(int sessionId,
                                                      int oldValue,
                                                      int newValue) {
    Q_UNUSED(oldValue);
    // Same hot path the old UnitExpandableWidget triggered via
    // EntityDetailView::onSessionProgressChanged. We call straight
    // into the DB; the resulting dataChanged() refreshes everyone.
    DatabaseManager::instance()->updateSessionTaskProgress(sessionId, newValue);
}

void CourseDetailView::onSubViewSessionRenamed(int sessionId,
                                              const QString& newName) {
    DatabaseManager::instance()->renameSessionTask(sessionId, newName);
}
