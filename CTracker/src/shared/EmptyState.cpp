#include "shared/EmptyState.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFont>

EmptyState::EmptyState(QWidget* parent)
    : QWidget(parent) {
    setObjectName("emptyState");
    setupUi();
}

void EmptyState::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(8);

    // Icon circle — a QLabel styled as a 64×64 rounded circle with muted bg
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("emptyStateIcon");
    m_iconLabel->setFixedSize(64, 64);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(
        QStringLiteral(
            "QLabel#emptyStateIcon {"
            "  background-color: #2d323d;"        // surface-hover
            "  border-radius: 32px;"
            "  color: #6b7280;"                    // subtle text
            "  font-size: 28px;"
            "}"
        ));
    // Use a simple folder-like Unicode character as default icon
    // (C++17 \u escapes cannot encode surrogate pairs; use fromUtf8 for emoji)
    m_iconLabel->setText(QString::fromUtf8("\xF0\x9F\x93\x82"));  // 📂 folder icon

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("emptyStateTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setWeight(QFont::Medium);
    m_titleLabel->setFont(titleFont);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setObjectName("emptyStateDescription");
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setMaximumWidth(400);
    m_descriptionLabel->setStyleSheet(
        QStringLiteral("QLabel#emptyStateDescription { color: #9ca3af; }"));

    m_actionBtn = new QPushButton(this);
    m_actionBtn->setObjectName("emptyStateAction");
    m_actionBtn->setVisible(false);

    layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_descriptionLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addWidget(m_actionBtn, 0, Qt::AlignHCenter);

    connect(m_actionBtn, &QPushButton::clicked,
            this, &EmptyState::actionRequested);
}

void EmptyState::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}

void EmptyState::setDescription(const QString& description) {
    m_descriptionLabel->setText(description);
}

void EmptyState::setActionLabel(const QString& label) {
    m_actionBtn->setText(label);
    m_actionBtn->setVisible(!label.isEmpty());
}

void EmptyState::setShowAction(bool show) {
    m_actionBtn->setVisible(show);
}