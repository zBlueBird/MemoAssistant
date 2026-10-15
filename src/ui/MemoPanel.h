#pragma once

#include <QDate>
#include <QWidget>

class MemoRepository;
class QLabel;
class QPushButton;
class QButtonGroup;
class QScrollArea;
class QStackedWidget;
class QToolButton;
class QVBoxLayout;
class TaskCalendarWidget;
class WeekOverviewWidget;

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
    void showPreviousPeriod();
    void showNextPeriod();
    void showToday();
    void setDayView();
    void setWeekView();
    void setMonthView();
    void selectCalendarDate(const QDate &date);
    void addMemo();
    void editMemo(qint64 id);
    void deleteMemo(qint64 id);
    void setMemoCompleted(qint64 id, bool completed);

private:
    enum class ViewMode { Day, Week, Month };

    bool event(QEvent *event) override;
    void clearList();
    void updateDateControls();
    void updateCalendar();
    QPair<QDate, QDate> visibleRange() const;
    void setViewMode(ViewMode mode);
    void showError(const QString &operation);

    MemoRepository *m_repository;
    QDate m_selectedDate;
    ViewMode m_viewMode = ViewMode::Day;
    QLabel *m_titleLabel;
    QLabel *m_dateLabel;
    QPushButton *m_previousDayButton;
    QPushButton *m_nextDayButton;
    QPushButton *m_todayButton;
    QPushButton *m_dayViewButton;
    QPushButton *m_weekViewButton;
    QPushButton *m_monthViewButton;
    QStackedWidget *m_contentStack;
    WeekOverviewWidget *m_weekOverview;
    TaskCalendarWidget *m_calendar;
    QScrollArea *m_scrollArea;
    QWidget *m_listContainer;
    QVBoxLayout *m_listLayout;
    QLabel *m_emptyLabel;
};
