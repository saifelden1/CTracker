#include "SideNavigationBar.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QStyle>
#include <QSpacerItem>

SideNavigationBar::SideNavigationBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName("sideNavigationBar");
    setFixedWidth(60);
    setupButtons();
}

QPushButton* SideNavigationBar::makeNavButton(const QString& glyph, const QString& tooltip) {
    auto* btn = new QPushButton(glyph, this);
    btn->setObjectName("navButton");
    btn->setToolTip(tooltip);
    btn->setFlat(true);
    btn->setCheckable(true);
    btn->setFixedSize(44, 44);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void SideNavigationBar::setupButtons() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 12, 8, 12);
    layout->setSpacing(6);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    // Order matches the Page enum; glyphs are placeholders until SVG icons land in Phase 8.3.2.
    const struct { const char* glyph; const char* tip; } defs[PageCount] = {
        { "\u2302", "Home"      },   // ⌂
        { "\u2630", "Courses"   },   // ☰
        { "\u25A4", "Projects"  },   // ▤
        { "\u2261", "Analytics" },   // ≡
        { "\u2699", "Settings"  }    // ⚙
    };

    for (int i = 0; i < PageCount; ++i) {
        QPushButton* btn = makeNavButton(QString::fromUtf8(defs[i].glyph),
                                         QString::fromLatin1(defs[i].tip));
        m_buttons.append(btn);
        m_group->addButton(btn, i);
        layout->addWidget(btn, 0, Qt::AlignHCenter);
    }

    layout->addStretch(1);

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
        btn->setProperty("active", active);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}
