#pragma once

#include <QWidget>

class MemoRepository;
class QLabel;
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
    void addMemo();
    void editMemo(qint64 id);
    void deleteMemo(qint64 id);
    void setMemoCompleted(qint64 id, bool completed);

private:
    bool event(QEvent *event) override;
    void clearList();
    void showError(const QString &operation);

    MemoRepository *m_repository;
    QLabel *m_dateLabel;
    QScrollArea *m_scrollArea;
    QWidget *m_listContainer;
    QVBoxLayout *m_listLayout;
    QLabel *m_emptyLabel;
};
