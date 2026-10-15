#pragma once

#include <QDate>
#include <QWidget>

class MemoRepository;
class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class MemoPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MemoPanel(MemoRepository *repository, QWidget *parent = nullptr);
    void refresh();

signals:
    void closeRequested();
    void pointerLeft();
    void memoChanged();

protected:
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void showPreviousDay();
    void showNextDay();
    void showToday();
    void addMemo();
    void editMemo(qint64 id);
    void deleteMemo(qint64 id);
    void setMemoCompleted(qint64 id, bool completed);

private:
    bool event(QEvent *event) override;
    void clearList();
    void updateDateControls();
    void showError(const QString &operation);

    MemoRepository *m_repository;
    QDate m_selectedDate;
    QLabel *m_titleLabel;
    QLabel *m_dateLabel;
    QPushButton *m_previousDayButton;
    QPushButton *m_nextDayButton;
    QPushButton *m_todayButton;
    QScrollArea *m_scrollArea;
    QWidget *m_listContainer;
    QVBoxLayout *m_listLayout;
    QLabel *m_emptyLabel;
};
