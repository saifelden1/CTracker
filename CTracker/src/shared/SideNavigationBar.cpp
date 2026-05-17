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

QPushButton* SideNavigationBar::makeNavButton(const QString& iconPath,
                                               const QString& activeIconPath,
                                               const QString& label,
                                               const QString& tooltip) {
    auto* btn = new QPushButton(this);
    btn->setObjectName("navButton");
    btn->setToolTip(tooltip);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(48);

    // Save the icon paths as custom properties for dynamic updates
    btn->setProperty("iconPath", iconPath);
    btn->setProperty("activeIconPath", activeIconPath);

    // Layout: icon (left) + label (center-left)
    auto* layout = new QHBoxLayout(btn);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(12);

    auto* iconLabel = new QLabel(btn);
    iconLabel->setObjectName("navIconLabel");
    iconLabel->setFixedSize(24, 24);
    iconLabel->setAlignment(Qt::AlignCenter);
    QIcon qicon(iconPath);
    iconLabel->setPixmap(qicon.pixmap(24, 24));

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
    headerLayout->setSpacing(12);

    // Logo with SVG icon
    auto* logoLabel = new QLabel(m_header);
    logoLabel->setObjectName("sideNavLogo");
    logoLabel->setFixedSize(40, 40);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setScaledContents(true);
    QIcon logoIcon(":/icons/logo.svg");
    logoLabel->setPixmap(logoIcon.pixmap(40, 40));

    auto* appNameLabel = new QLabel(QStringLiteral("CTracker"), m_header);
    appNameLabel->setObjectName("sideNavAppName");

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(appNameLabel, 1);

    layout->addWidget(m_header);

    // ── Navigation buttons ──
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Order matches the Page enum
    const struct { const char* icon; const char* activeIcon; const char* label; const char* tip; } defs[PageCount] = {
        { ":/icons/lucide/home.svg",        ":/icons/lucide/home-active.svg",        "Home",      "Home Dashboard"       },
        { ":/icons/lucide/book-open.svg",   ":/icons/lucide/book-open-active.svg",   "Courses",   "Courses"              },
        { ":/icons/lucide/folder-kanban.svg", ":/icons/lucide/folder-kanban-active.svg", "Projects",  "Projects"             },
        { ":/icons/lucide/check-square.svg", ":/icons/lucide/check-square-active.svg", "To-Do",     "To-Do List"           },
        { ":/icons/lucide/timer.svg",        ":/icons/lucide/timer-active.svg",        "Pomodoro",  "Pomodoro Timer"       },
        { ":/icons/lucide/bar-chart-3.svg",  ":/icons/lucide/bar-chart-3-active.svg",  "Analytics", "Analytics & Reports"  },
        { ":/icons/lucide/settings.svg",     ":/icons/lucide/settings-active.svg",     "Settings",  "Settings"             }
    };

    for (int i = 0; i < PageCount; ++i) {
        QPushButton* btn = makeNavButton(QString::fromLatin1(defs[i].icon),
                                         QString::fromLatin1(defs[i].activeIcon),
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
        
        // Update the icon to active/inactive version
        auto* iconLabel = btn->findChild<QLabel*>("navIconLabel");
        if (iconLabel) {
            QString path = active ? btn->property("activeIconPath").toString() : btn->property("iconPath").toString();
            QIcon qicon(path);
            iconLabel->setPixmap(qicon.pixmap(24, 24));
        }

        // Property-based styling: QSS can target QPushButton[active="true"]
        btn->setProperty("active", active);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}
