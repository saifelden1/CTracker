#pragma once

#include <QWidget>
#include <QString>
#include <QColor>

class QLabel;

struct CategoryData;

// Small horizontal pill: 8 px colored dot + category name.
// Background = category color at 15% alpha; sm (4 px) border-radius.
// setVisible(false) when clearCategory() is called (categoryId == -1).
class CategoryPill : public QWidget {
    Q_OBJECT
public:
    explicit CategoryPill(QWidget* parent = nullptr);

    void setCategory(const CategoryData& c);   // colored dot + name; semi-transparent bg
    void clearCategory();                       // hide pill when categoryId == -1

private:
    void applyStyle(const QColor& color);

    QLabel* m_dotLabel  = nullptr;   // 8×8 px colored circle
    QLabel* m_nameLabel = nullptr;   // category name text
};