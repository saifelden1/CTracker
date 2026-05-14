#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QSlider;
class QLineEdit;
class QStackedWidget;

class SessionTaskRow : public QWidget {
    Q_OBJECT
public:
    explicit SessionTaskRow(int sessionId,
                            const QString& name,
                            int progress,
                            QWidget* parent = nullptr);

    int     sessionId() const { return m_sessionId; }
    int     progress()  const;
    QString name()      const { return m_name; }

    void setProgress(int value);
    void setName(const QString& newName);

signals:
    void progressChanged(int oldValue, int newValue);
    void nameChanged(const QString& newName);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSliderPressed();
    void onSliderReleased();
    void onSliderValueChanged(int value);
    void onNameEditingFinished();

private:
    void setupUi();
    void enterEditMode();
    void exitEditMode(bool commit);

    int     m_sessionId    = -1;
    QString m_name;
    int     m_oldProgress  = 0;
    bool    m_sliderPressed = false;

    QStackedWidget* m_nameStack = nullptr;
    QLabel*         m_nameLabel = nullptr;
    QLineEdit*      m_nameEdit  = nullptr;
    QSlider*        m_slider    = nullptr;
    QLabel*         m_percentLabel = nullptr;
};
