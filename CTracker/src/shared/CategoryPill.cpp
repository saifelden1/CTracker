#include "shared/CategoryPill.h"

#include "core/DataStructures.h"

#include <QHBoxLayout>
#include <QLabel>

CategoryPill::CategoryPill(QWidget* parent)
    : QWidget(parent) {
    setObjectName("categoryPill");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    m_dotLabel = new QLabel(this);
    m_dotLabel->setFixedSize(8, 8);
    m_dotLabel->setAlignment(Qt::AlignCenter);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(m_dotLabel);
    layout->addWidget(m_nameLabel);

    // Start hidden — caller shows via setCategory()
    setVisible(false);
}

void CategoryPill::setCategory(const CategoryData& c) {
    m_nameLabel->setText(c.name);
    applyStyle(c.color);
    setVisible(true);
}

void CategoryPill::clearCategory() {
    m_nameLabel->clear();
    m_dotLabel->setStyleSheet({});
    setStyleSheet({});
    setVisible(false);
}

void CategoryPill::applyStyle(const QColor& color) {
    // 8 px colored dot — circular via border-radius = 4
    m_dotLabel->setStyleSheet(
        QStringLiteral(
            "QLabel {"
            "  background-color: %1;"
            "  border-radius: 4px;"
            "}"
        ).arg(color.name()));

    // Pill background = category color at 15% alpha, sm radius
    // QSS rgba() requires integer components: r, g, b, a where a is 0–255.
    // 15% of 255 ≈ 38.
    const int alpha15 = static_cast<int>(255 * 0.15);  // ≈ 38
    const QString rgbaBg = QStringLiteral("rgba(%1, %2, %3, %4)")
                               .arg(color.red())
                               .arg(color.green())
                               .arg(color.blue())
                               .arg(alpha15);

    setStyleSheet(
        QStringLiteral(
            "QWidget#categoryPill {"
            "  background-color: %1;"
            "  border-radius: 4px;"
            "}"
            "QLabel { color: %2; }"   // name label inherits pill text color
        ).arg(rgbaBg, color.name()));
}