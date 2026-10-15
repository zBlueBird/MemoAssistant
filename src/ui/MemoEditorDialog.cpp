#include "ui/MemoEditorDialog.h"

#include <QCheckBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTimeEdit>
#include <QTimeZone>
#include <QVBoxLayout>

MemoEditorDialog::MemoEditorDialog(QWidget *parent)
    : QDialog(parent),
      m_titleEdit(new QLineEdit(this)),
      m_contentEdit(new QTextEdit(this)),
      m_scheduledDateEdit(new QDateEdit(this)),
      m_reminderCheck(new QCheckBox(tr("启用提醒"), this)),
      m_reminderDateEdit(new QDateEdit(this)),
      m_reminderTimeEdit(new QTimeEdit(this))
{
    setObjectName(QStringLiteral("memoEditorDialog"));
    setWindowTitle(tr("备忘录"));
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(500, 465);
    resize(500, 465);

    auto *surface = new QFrame(this);
    surface->setObjectName(QStringLiteral("editorSurface"));

    auto *iconLabel = new QLabel(QStringLiteral("▤"), surface);
    iconLabel->setObjectName(QStringLiteral("editorTitleIcon"));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(22, 22);

    auto *titleLabel = new QLabel(tr("备忘录"), surface);
    titleLabel->setObjectName(QStringLiteral("editorTitle"));

    auto *closeButton = new QPushButton(QStringLiteral("×"), surface);
    closeButton->setObjectName(QStringLiteral("editorCloseButton"));
    closeButton->setFixedSize(34, 32);
    closeButton->setToolTip(tr("关闭"));

    auto *titleBar = new QHBoxLayout;
    titleBar->setContentsMargins(14, 7, 7, 7);
    titleBar->setSpacing(6);
    titleBar->addWidget(iconLabel);
    titleBar->addWidget(titleLabel);
    titleBar->addStretch();
    titleBar->addWidget(closeButton);

    m_titleEdit->setPlaceholderText(tr("要记住什么？"));
    m_titleEdit->setMaxLength(200);
    m_titleEdit->setObjectName(QStringLiteral("editorTitleInput"));
    m_titleEdit->setMinimumHeight(36);
    m_contentEdit->setPlaceholderText(tr("补充说明（可选）"));
    m_contentEdit->setObjectName(QStringLiteral("editorContentInput"));
    m_contentEdit->setFixedHeight(118);

    m_scheduledDateEdit->setObjectName(QStringLiteral("editorScheduledDate"));
    m_scheduledDateEdit->setCalendarPopup(true);
    m_scheduledDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_scheduledDateEdit->setDate(QDate::currentDate());
    m_scheduledDateEdit->setMinimumDate(QDate(2000, 1, 1));
    m_scheduledDateEdit->setMaximumDate(QDate::currentDate().addYears(20));
    m_scheduledDateEdit->setToolTip(tr("备忘录将在所选日期的日、周和月视图中显示"));

    m_reminderCheck->setObjectName(QStringLiteral("editorReminderCheck"));
    m_reminderDateEdit->setObjectName(QStringLiteral("editorReminderDate"));
    m_reminderTimeEdit->setObjectName(QStringLiteral("editorReminderTime"));

    QDateTime defaultReminder = QDateTime::currentDateTime().addSecs(3600);
    defaultReminder.setTime(QTime(defaultReminder.time().hour(), defaultReminder.time().minute()));
    m_reminderDateEdit->setCalendarPopup(true);
    m_reminderDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    m_reminderDateEdit->setMinimumWidth(154);
    m_reminderDateEdit->setDate(defaultReminder.date());
    m_reminderDateEdit->setMinimumDate(QDate::currentDate());
    m_reminderDateEdit->setEnabled(false);

    m_reminderTimeEdit->setDisplayFormat(QStringLiteral("HH:mm"));
    m_reminderTimeEdit->setMinimumWidth(112);
    m_reminderTimeEdit->setTime(defaultReminder.time());
    m_reminderTimeEdit->setToolTip(tr("点击小时或分钟后输入，也可使用方向键调整"));
    m_reminderTimeEdit->setEnabled(false);

    auto *form = new QFormLayout;
    form->setContentsMargins(22, 14, 22, 0);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(14);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);

    auto *titleFieldLabel = new QLabel(tr("标题"), surface);
    auto *contentFieldLabel = new QLabel(tr("内容"), surface);
    auto *scheduledDateFieldLabel = new QLabel(tr("计划日期"), surface);
    auto *reminderFieldLabel = new QLabel(tr("提醒"), surface);
    for (QLabel *label : {titleFieldLabel, contentFieldLabel, scheduledDateFieldLabel, reminderFieldLabel}) {
        label->setObjectName(QStringLiteral("editorFieldLabel"));
        label->setMinimumWidth(42);
    }
    form->addRow(titleFieldLabel, m_titleEdit);
    form->addRow(contentFieldLabel, m_contentEdit);
    form->addRow(scheduledDateFieldLabel, m_scheduledDateEdit);

    auto *dateLabel = new QLabel(tr("日期"), surface);
    auto *timeLabel = new QLabel(tr("时间"), surface);
    dateLabel->setObjectName(QStringLiteral("editorInlineLabel"));
    timeLabel->setObjectName(QStringLiteral("editorInlineLabel"));

    auto *dateTimeRow = new QHBoxLayout;
    dateTimeRow->setContentsMargins(0, 0, 0, 0);
    dateTimeRow->setSpacing(8);
    dateTimeRow->addWidget(dateLabel);
    dateTimeRow->addWidget(m_reminderDateEdit);
    dateTimeRow->addSpacing(4);
    dateTimeRow->addWidget(timeLabel);
    dateTimeRow->addWidget(m_reminderTimeEdit);
    dateTimeRow->addStretch();

    auto *reminderLayout = new QVBoxLayout;
    reminderLayout->setContentsMargins(0, 0, 0, 0);
    reminderLayout->setSpacing(8);
    reminderLayout->addWidget(m_reminderCheck);
    reminderLayout->addLayout(dateTimeRow);
    form->addRow(reminderFieldLabel, reminderLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, surface);
    buttons->setObjectName(QStringLiteral("editorButtons"));
    buttons->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("editorSaveButton"));
    buttons->button(QDialogButtonBox::Cancel)->setObjectName(QStringLiteral("editorCancelButton"));
    buttons->button(QDialogButtonBox::Save)->setText(tr("保存"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    buttons->button(QDialogButtonBox::Save)->setMinimumSize(78, 34);
    buttons->button(QDialogButtonBox::Cancel)->setMinimumSize(78, 34);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setContentsMargins(22, 8, 22, 16);
    buttonRow->addStretch();
    buttonRow->addWidget(buttons);

    auto *surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(0, 0, 0, 0);
    surfaceLayout->setSpacing(0);
    surfaceLayout->addLayout(titleBar);
    surfaceLayout->addLayout(form, 1);
    surfaceLayout->addLayout(buttonRow);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(surface);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_reminderCheck, &QCheckBox::toggled,
            this, &MemoEditorDialog::updateReminderState);
    connect(buttons, &QDialogButtonBox::accepted,
            this, &MemoEditorDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void MemoEditorDialog::setScheduledDate(const QDate &date)
{
    m_scheduledDateEdit->setDate(date.isValid() ? date : QDate::currentDate());
}

void MemoEditorDialog::setMemo(const Memo &memo)
{
    m_memo = memo;
    m_titleEdit->setText(memo.title);
    m_contentEdit->setPlainText(memo.content);
    setScheduledDate(memo.scheduledDate);
    m_reminderCheck->setChecked(memo.reminderAt.isValid());
    if (memo.reminderAt.isValid()) {
        const QDateTime localReminder = memo.reminderAt.toLocalTime();
        m_reminderDateEdit->setDate(localReminder.date());
        m_reminderTimeEdit->setTime(QTime(localReminder.time().hour(),
                                          localReminder.time().minute()));
    }
    m_titleEdit->selectAll();
}

Memo MemoEditorDialog::memo() const
{
    Memo result = m_memo;
    result.title = m_titleEdit->text().trimmed();
    result.content = m_contentEdit->toPlainText().trimmed();
    result.scheduledDate = m_scheduledDateEdit->date();
    const QTime selectedTime(m_reminderTimeEdit->time().hour(),
                             m_reminderTimeEdit->time().minute());
    result.reminderAt = m_reminderCheck->isChecked()
                            ? QDateTime(m_reminderDateEdit->date(),
                                        selectedTime,
                                        QTimeZone::systemTimeZone()).toUTC()
                            : QDateTime();
    result.reminderSent = false;
    return result;
}

void MemoEditorDialog::validateAndAccept()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("无法保存"), tr("请输入备忘录标题。"));
        m_titleEdit->setFocus();
        return;
    }
    const QDateTime selectedReminder(m_reminderDateEdit->date(),
                                     m_reminderTimeEdit->time(),
                                     QTimeZone::systemTimeZone());
    if (m_reminderCheck->isChecked()
        && selectedReminder < QDateTime::currentDateTime().addSecs(-60)) {
        QMessageBox::warning(this, tr("提醒时间无效"), tr("提醒时间不能早于当前时间。"));
        m_reminderTimeEdit->setFocus();
        return;
    }
    accept();
}

void MemoEditorDialog::updateReminderState(bool enabled)
{
    m_reminderDateEdit->setEnabled(enabled);
    m_reminderTimeEdit->setEnabled(enabled);

    const QDateTime selectedReminder(m_reminderDateEdit->date(),
                                     m_reminderTimeEdit->time(),
                                     QTimeZone::systemTimeZone());
    if (enabled && selectedReminder < QDateTime::currentDateTime()) {
        QDateTime defaultReminder = QDateTime::currentDateTime().addSecs(3600);
        defaultReminder.setTime(QTime(defaultReminder.time().hour(),
                                      defaultReminder.time().minute()));
        m_reminderDateEdit->setDate(defaultReminder.date());
        m_reminderTimeEdit->setTime(defaultReminder.time());
    }
}
