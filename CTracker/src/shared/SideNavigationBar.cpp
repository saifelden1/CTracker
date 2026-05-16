#include "shared/SideNavigationBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QStyle>

// Task 7.1: SideNavigationBar — 256 px fixed width, header, 7 nav buttons, footer.
// Active button uses left accent border (applied via QSS with [active="true"] selector).

SideNavigationBar::SideNavigationBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("sideNavigationBar");
    setFixedWidth(256);
    setupUi();
}

QPushButton* SideNavigationBar::makeNavButton(const QString& icon,
                                               const QString& label,
                                               const QString& tooltip) {
    auto* btn = new QPushButton(this);
    btn->setObjectName("navButton");
    btn->setToolTip(tooltip);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(48);

    // Layout: icon (left) + label (center-left)
    auto* layout = new QHBoxLayout(btn);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(12);

    auto* iconLabel = new QLabel(icon, btn);
    iconLabel->setFixedSize(24, 24);
    iconLabel->setAlignment(Qt::AlignCenter);

    auto* textLabel = new QLabel(label, btn);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel, 1);

    return btn;
}

void SideNavigationBar::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Header: CTracker logo + name ──
    m_header = new QWidget(this);
    m_header->setObjectName("sideNavHeader");
    m_header->setFixedHeight(64);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(16, 12, 16, 12);
    headerLayout->setSpacing(8);

    auto* logoLabel = new QLabel(QStringLiteral("\u2302"), m_header);  // ⌂ placeholder
    logoLabel->setObjectName("sideNavLogo");
    logoLabel->setFixedSize(32, 32);
    logoLabel->setAlignment(Qt::AlignCenter);

    auto* appNameLabel = new QLabel(QStringLiteral("CTracker"), m_header);
    appNameLabel->setObjectName("sideNavAppName");

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(appNameLabel, 1);

    layout->addWidget(m_header);

    // ── Navigation buttons ──
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Order matches the Page enum; icons are Unicode placeholders until Phase 8.4 SVG icons.
    const struct { const char* icon; const char* label; const char* tip; } defs[PageCount] = {
        { "\u2302", "Home",      "Home Dashboard"       },  // ⌂
        { "\u{1F4DA}", "Courses",   "Courses"              },  // 📚
        { "\u{1F4C1}", "Projects",  "Projects"             },  // 📁
        { "\u2611", "To-Do",     "To-Do List"           },  // ☑
        { "\u23F1", "Pomodoro",  "Pomodoro Timer"       },  // ⏱
        { "\u{1F4CA}", "Analytics", "Analytics & Reports"  },  // 📊
        { "\u2699", "Settings",  "Settings"             }   // ⚙
    };

    for (int i = 0; i < PageCount; ++i) {
        QPushButton* btn = makeNavButton(QString::fromUtf8(defs[i].icon),
                                         QString::fromLatin1(defs[i].label),
                                         QString::fromLatin1(defs[i].tip));
        m_buttons.append(btn);
        m_group->addButton(btn, i);
        layout->addWidget(btn);
    }

    layout->addStretch(1);

    // ── Footer: user profile chip (placeholder) ──
    m_footer = new QWidget(this);
    m_footer->setObjectName("sideNavFooter");
    m_footer->setFixedHeight(56);
    auto* footerLayout = new QHBoxLayout(m_footer);
    footerLayout->setContentsMargins(16, 8, 16, 8);
    footerLayout->setSpacing(8);

    auto* avatarLabel = new QLabel(QStringLiteral("\u{1F464}"), m_footer);  // 👤
    avatarLabel->setFixedSize(32, 32);
    avatarLabel->setAlignment(Qt::AlignCenter);

    auto* userLabel = new QLabel(QStringLiteral("User"), m_footer);
    userLabel->setObjectName("sideNavUserName");

    footerLayout->addWidget(avatarLabel);
    footerLayout->addWidget(userLabel, 1);

    layout->addWidget(m_footer);

    // ── Wire signals ──
    connect(m_group, &QButtonGroup::idClicked,
            this, [this](int id) {
                setActiveButton(id);
                emit navigationRequested(id);
            });

    setActiveButton(HomePage);
}

void SideNavigationBar::setActiveButton(int index) {
    if (index < 0 || index >= m_buttons.size()) {
        return;
    }
    m_currentIndex = index;
    for (int i = 0; i < m_buttons.size(); ++i) {
        QPushButton* btn = m_buttons[i];
        const bool active = (i == index);
        if (btn->isChecked() != active) {
            QSignalBlocker blocker(btn);
            btn->setChecked(active);
        }
        // Property-based styling: QSS can target QPushButton[active="true"]
        btn->setProperty("active", active);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}
