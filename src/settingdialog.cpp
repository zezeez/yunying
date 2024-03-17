#include "settingdialog.h"
#include "userdata.h"
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QTimeEdit>
#include <QComboBox>
#include <QSettings>
#include <QSpinBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QTimer>

#define _ QStringLiteral

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent)
{
    dialog = new QDialog(parent);
    trainTypeShowVec.resize(128);
}

SettingDialog::~SettingDialog()
{
    delete dialog;
}

void SettingDialog::show()
{
    dialog->show();
}

void SettingDialog::hide()
{
    dialog->hide();
}

void SettingDialog::commonSetting(QTabWidget *tab)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    QCheckBox *cbAll = new QCheckBox(tr("全选"));
    QCheckBox *cbFuxingTrain = new QCheckBox(tr("复兴"));
    QCheckBox *cbZiNengTrain = new QCheckBox(tr("智能"));
    QCheckBox *cbDongGanTrain = new QCheckBox(tr("动感"));
    QCheckBox *cbGTrain = new QCheckBox(tr("G-高铁"));
    QCheckBox *cbDTrain = new QCheckBox(tr("D-动车"));
    QCheckBox *cbCTrain = new QCheckBox(tr("C-城际"));
    QCheckBox *cbZTrain = new QCheckBox(tr("Z-直达"));
    QCheckBox *cbTTrain = new QCheckBox(tr("T-特快"));
    QCheckBox *cbKTrain = new QCheckBox(tr("K-普快"));
    QCheckBox *cbOTrain = new QCheckBox(tr("其他"));
    connect(cbOTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['O'] = state;
    });
    connect(cbKTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['K'] = state;
    });
    connect(cbTTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['T'] = state;
    });
    connect(cbZTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['Z'] = state;
    });
    connect(cbCTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['C'] = state;
    });
    connect(cbDTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['D'] = state;
    });
    connect(cbGTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['G'] = state;
    });
    connect(cbZiNengTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['5'] = state;
    });
    connect(cbFuxingTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['1'] = state;
    });
    connect(cbDongGanTrain, &QCheckBox::stateChanged, this, [=](int state) {
        trainTypeShowVec['d'] = state;
    });
    connect(cbAll, &QCheckBox::stateChanged, this, [=](int state) {
        cbFuxingTrain->setChecked(state);
        cbZiNengTrain->setChecked(state);
        cbDongGanTrain->setChecked(state);
        cbGTrain->setChecked(state);
        cbDTrain->setChecked(state);
        cbCTrain->setChecked(state);
        cbZTrain->setChecked(state);
        cbTTrain->setChecked(state);
        cbKTrain->setChecked(state);
        cbOTrain->setChecked(state);
    });
    cbAll->setChecked(true);
    hLayout->addWidget(cbAll);
    hLayout->addWidget(cbFuxingTrain);
    hLayout->addWidget(cbZiNengTrain);
    hLayout->addWidget(cbDongGanTrain);
    hLayout->addWidget(cbGTrain);
    hLayout->addWidget(cbDTrain);
    hLayout->addWidget(cbCTrain);
    hLayout->addWidget(cbZTrain);
    hLayout->addWidget(cbTTrain);
    hLayout->addWidget(cbKTrain);
    hLayout->addWidget(cbOTrain);
    hLayout->setSpacing(10);
    //hLayout->setMargin(5);
    hLayout->addStretch(1);

    QGroupBox *group = new QGroupBox;
    group->setLayout(hLayout);
    group->setTitle(tr("筛选"));
    vLayout->addWidget(group);

    QSettings setting;
    QCheckBox *cb = new QCheckBox(tr("自动同步12306服务器时间"));
    connect(cb, &QCheckBox::toggled, this, [=] (bool checked) {
        UserData *ud = UserData::instance();
        ud->generalSetting.autoSyncServerTime = checked;
        QSettings setting;
        setting.setValue(_("setting/auto_sync_server_time"), checked);
    });
    bool checked = setting.value(_("setting/auto_sync_server_time"), true).value<bool>();
    cb->setChecked(checked);
    QGroupBox *groupBox = new QGroupBox;
    QVBoxLayout *vLayout1 = new QVBoxLayout;
    vLayout1->addWidget(cb);
    groupBox->setLayout(vLayout1);

    vLayout->addWidget(groupBox);
    vLayout->addStretch();
    widget->setLayout(vLayout);

    tab->addTab(widget, tr("一般设置"));
}

void SettingDialog::grabTicketSetting(QTabWidget *tab)
{
    QWidget *widget = new QWidget;
    //QHBoxLayout *hLayout = new QHBoxLayout;
    //QVBoxLayout *vLayout = new QVBoxLayout;
    QSettings setting;
    QStringList dates;
    //QGridLayout *gLayout = new QGridLayout;
    QCheckBox *grabTicketCB = new QCheckBox(tr("定时抢票"));
    QComboBox *cbox = new QComboBox;
    QDate curDate = QDate::currentDate();
    for (int i = 0; i < 5; i++) {
        dates.append(curDate.addDays(i).toString(_("MM-dd")));
    }
    cbox->addItems(dates);
    int index = setting.value(_("grab_setting/grab_ticket_date"), 0).value<int>();
    connect(cbox, &QComboBox::currentIndexChanged, this, [] (int index) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabTicketDate = index;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_ticket_date"), index);
    });
    cbox->setCurrentIndex(index);
    QTimeEdit *timeEdit = new QTimeEdit;
    connect(grabTicketCB, &QCheckBox::toggled, this, [=] (bool checked) {
        timeEdit->setEnabled(checked);
        cbox->setEnabled(checked);
        QSettings setting;
        setting.setValue(_("grab_setting/grab_fixed_time"), checked);
    });
    connect(timeEdit, &QTimeEdit::timeChanged, this, [=] (QTime time) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabTicketHour = time.hour();
        ud->grabSetting.grabTicketMinute = time.minute();
        ud->grabSetting.grabTicketSecond = time.second();
        QSettings setting;
        setting.setValue(_("grab_setting/grab_ticket_time"), time.toString(_("hh:mm:ss")));
    });
    QString timeStr = setting.value(_("grab_setting/grab_ticket_time"), _("")).value<QString>();
    QTime setTime = QTime::fromString(timeStr, _("hh:mm:ss"));
    QTime curTime = QTime::currentTime();
    if (!setTime.isValid() || (index == 0 && setTime < curTime)) {
        timeEdit->setTime(curTime.addSecs(3600));
    } else {
        timeEdit->setTime(setTime);
    }

    bool checked = setting.value(_("grab_setting/grab_fixed_time"), false).value<bool>();
    grabTicketCB->setChecked(checked);
    cbox->setEnabled(checked);
    timeEdit->setEnabled(checked);
    timeEdit->setDisplayFormat(_("hh:mm:ss"));
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(grabTicketCB);
    hlayout->addWidget(cbox);
    hlayout->addWidget(timeEdit);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);

    QCheckBox *cb = new QCheckBox(tr("自动冻结"));
    cb->setToolTip(tr("由于cdn存在缓存数据的原因，可能后台服务器已经没有票了，但是\n"
                      "cdn缓存显示有票，这时候提交请求到后台服务器是返回无票的（或者\n"
                      "是排队人数大于余票张数），遇到这种情况标记该车次数据为缓存并\n"
                      "冻结该车次一段时间内不再提交"));
    QSpinBox *sbox = new QSpinBox;
    sbox->setMinimum(30);
    sbox->setMaximum(3600);
    connect(sbox, &QSpinBox::valueChanged, this, [] (int value) {
        UserData::instance()->grabSetting.frozenSeconds = value;
        QSettings setting;
        setting.setValue(_("grab_setting/frozen_seconds"), value);
    });
    int secs = setting.value(_("grab_setting/frozen_seconds"), 180).value<int>();
    sbox->setValue(secs);
    sbox->setEnabled(checked);
    connect(cb, &QCheckBox::toggled, this, [sbox] (bool checked) {
        UserData::instance()->grabSetting.autoFrozenTrain = checked;
        QSettings setting;
        setting.setValue(_("grab_setting/auto_frozen_train"), checked);
        sbox->setEnabled(checked);
    });
    checked = setting.value(_("grab_setting/auto_frozen_train"), true).value<bool>();
    cb->setChecked(checked);
    hlayout = new QHBoxLayout;
    hlayout->addWidget(cb);
    hlayout->addWidget(sbox);
    QLabel *label = new QLabel;
    label->setText(tr("秒"));
    hlayout->addWidget(label);
    hlayout->addStretch();
    QGroupBox *gbox = new QGroupBox(tr("刷票模式"));
    QVBoxLayout *vlayout1 = new QVBoxLayout;
    QRadioButton *rb = new QRadioButton(tr("短间隔模式(3秒)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = ESHORTINTERVAL;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_short"), checked);
    });
    checked = setting.value(_("grab_setting/grab_short"), false).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);
    rb = new QRadioButton(tr("长间隔模式(30秒)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = ELONGINTERVAL;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_long"), checked);
    });
    checked = setting.value(_("grab_setting/grab_long"), false).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);
    rb = new QRadioButton(tr("长短交叉模式(5次短间隔+1次长间隔)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = ESHORTANDLONG;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_short_long"), checked);
    });
    checked = setting.value(_("grab_setting/grab_short_long"), true).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);
    rb = new QRadioButton(tr("随机模式(2-6之间随机)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = ERANDOM;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_random"), checked);
    });
    checked = setting.value(_("grab_setting/grab_random"), false).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);
    rb = new QRadioButton(tr("定时抢票模式(间隔1秒直到有订单提交或超过30秒之后切换默认模式)"));
    connect(rb, &QRadioButton::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = EFIXEDTIME;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_fixed_time"), checked);
    });
    checked = setting.value(_("grab_setting/grab_fixed_time"), false).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);
    sbox = new QSpinBox;
    rb = new QRadioButton(tr("自定义(秒)"));
    connect(rb, &QRadioButton::toggled, this, [sbox] (bool checked) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabMode = ECUSTOM;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_custom"), checked);
        sbox->setEnabled(checked);
    });
    checked = setting.value(_("grab_setting/grab_custom"), false).value<bool>();
    rb->setChecked(checked);
    vlayout1->addWidget(rb);

    connect(sbox, &QSpinBox::valueChanged, this, [] (int value) {
        UserData *ud = UserData::instance();
        ud->grabSetting.grabIntervalSeconds = value;
        QSettings setting;
        setting.setValue(_("grab_setting/grab_seconds"), value);
    });
    sbox->setMinimum(1);
    sbox->setMaximum(3600);
    secs = setting.value(_("grab_setting/grab_seconds"), 1).value<int>();
    sbox->setValue(secs);
    sbox->setEnabled(checked);
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addWidget(sbox);
    hlayout1->addStretch();
    vlayout1->addLayout(hlayout1);
    gbox->setLayout(vlayout1);
    vlayout->addLayout(hlayout);
    vlayout->addStretch();
    vlayout->addWidget(gbox);
    widget->setLayout(vlayout);
    tab->addTab(widget, tr("抢票设置"));
}

void SettingDialog::candidateSetting(QTabWidget *tab)
{
    QSettings setting;
    QGroupBox *groupBox = new QGroupBox;
    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    QCheckBox *candidateCB = new QCheckBox(tr("候补购票"));

    QCheckBox *onlyCandidateCB = new QCheckBox(tr("只候补不刷票"));
    connect(onlyCandidateCB, &QCheckBox::toggled, this, [=] (bool checked) {
        UserData::instance()->candidateSetting.onlyCandidate = checked;
        QSettings setting;
        setting.setValue(_("candidate_setting/only_candidate"), checked);
    });
    bool checked = setting.value(_("candidate_setting/only_candidate"), false).value<bool>();
    onlyCandidateCB->setChecked(checked);
    QCheckBox *prioCandidateCB = new QCheckBox(tr("优先候补"));
    connect(prioCandidateCB, &QCheckBox::toggled, this, [] (bool checked) {
        UserData::instance()->candidateSetting.prioCandidate = checked;
        QSettings setting;
        setting.setValue(_("candidate_setting/prio_candidate"), checked);
    });
    checked = setting.value(_("candidate_setting/prio_candidate"), false).value<bool>();
    prioCandidateCB->setChecked(checked);
    QCheckBox *noSeatCB = new QCheckBox(tr("接受无座"));
    connect(noSeatCB, &QCheckBox::toggled, this, [] (bool checked) {
        UserData::instance()->candidateSetting.acceptNoSeat = checked;
        QSettings setting;
        setting.setValue(_("candidate_setting/accept_no_seat"), checked);
    });
    checked = setting.value(_("candidate_setting/accept_no_seat"), false).value<bool>();
    noSeatCB->setChecked(checked);
    QCheckBox *acceptNewTrainCB = new QCheckBox(tr("接受增开列车"));

    QTimeEdit *timeEdit2 = new QTimeEdit;
    timeEdit2->setDisplayFormat(_("hh"));
    timeEdit2->setMaximumTime(QTime(22, 0, 0));
    QLabel *label = new QLabel(tr("点 - "));
    QTimeEdit *timeEdit3 = new QTimeEdit;
    timeEdit3->setDisplayFormat(_("hh"));
    timeEdit3->setMaximumTime(QTime(24, 0, 0));
    timeEdit3->setMinimumTime(QTime(1, 0, 0));
    timeEdit3->setTime(QTime(23, 0));
    QLabel *label2 = new QLabel(tr("点"));
    connect(timeEdit2, &QTimeEdit::timeChanged, this, [=] (QTime time) {
        int h = time.hour();
        QSettings setting;
        UserData *ud = UserData::instance();
        if (h >= ud->candidateSetting.extraCandidateEndHour) {
            timeEdit3->setTime(QTime(h + 1, 0, 0));
            setting.setValue(_("candidate_setting/extra_end_hour"), h + 1);
        }
        ud->candidateSetting.extraCandidateStartHour = h;
        setting.setValue(_("candidate_setting/extra_start_hour"), h);
    });
    int hour = setting.value(_("candidate_setting/extra_start_hour"), 0).value<int>();
    timeEdit2->setTime(QTime(hour, 0, 0));
    connect(timeEdit3, &QTimeEdit::timeChanged, this, [=] (QTime time) {
        int h = time.hour();
        QSettings setting;
        UserData *ud = UserData::instance();
        if (h <= ud->candidateSetting.extraCandidateStartHour) {
            timeEdit2->setTime(QTime(h - 1, 0, 0));
            setting.setValue(_("candidate_setting/extran_start_hour"), h - 1);
        }
        ud->candidateSetting.extraCandidateEndHour = h;
        setting.setValue(_("candidate_setting/extra_end_hour"), h);
    });
    hour = setting.value(_("candidate_setting/extra_end_hour"), 0).value<int>();
    timeEdit3->setTime(QTime(hour, 0, 0));

    connect(acceptNewTrainCB, &QCheckBox::toggled, this, [=] (bool checked) {
        UserData::instance()->candidateSetting.acceptNewTrain = checked;
        timeEdit2->setEnabled(checked && candidateCB->isChecked());
        timeEdit3->setEnabled(checked && candidateCB->isChecked());
        QSettings setting;
        setting.setValue(_("candidate_setting/accept_new_train"), checked);
    });

    QCheckBox *extraCandidateDateCB = new QCheckBox(tr("额外的候补日期："));
    QDateEdit *extraCandidateDate = new QDateEdit;
    QDate curDate = QDate::currentDate();
    QDate setDate;
    extraCandidateDate->setDisplayFormat(_("yyyy-MM-dd"));
    connect(extraCandidateDate, &QDateEdit::dateChanged, this, [] (QDate date) {
        QSettings setting;
        QString s = date.toString(_("yyyy-MM-dd"));
        UserData::instance()->candidateSetting.extraCandidateDate = s;
        setting.setValue(_("candidate_setting/extra_date"), s);
    });
    QString extra_date = setting.value(_("candidate_setting/extra_date"), curDate.toString(_("yyy-MM-dd"))).value<QString>();
    setDate = QDate::fromString(extra_date, _("yyyy-MM-dd"));
    if (!setDate.isValid() || setDate < curDate) {
        extraCandidateDate->setDate(curDate);
    } else {
        extraCandidateDate->setDate(setDate);
    }

    connect(extraCandidateDateCB, &QCheckBox::toggled, this, [=] (bool checked) {
        QSettings setting;
        extraCandidateDate->setEnabled(checked && candidateCB->isChecked());
        UserData::instance()->candidateSetting.extraCandidateDateEn = checked;
        setting.setValue(_("candidate_setting/extra_date_en"), checked);
    });
    checked = setting.value(_("candidate_setting/extra_date_en"), false).value<bool>();
    extraCandidateDateCB->setChecked(checked);
    //extraCandidateDate->setEnabled(false);
    QCheckBox *extraCandidateDateCB2 = new QCheckBox(tr("额外的候补日期："));
    QDateEdit *extraCandidateDate2 = new QDateEdit;
    extraCandidateDate2->setDisplayFormat(_("yyyy-MM-dd"));
    connect(extraCandidateDate2, &QDateEdit::dateChanged, this, [] (QDate date) {
        QSettings setting;
        QString s = date.toString(_("yyyy-MM-dd"));
        UserData::instance()->candidateSetting.extraCandidateDate2 = s;
        setting.setValue(_("candidate_setting/extra_date2"), s);
    });
    extra_date = setting.value(_("candidate_setting/extra_date2"), curDate.toString(_("yyy-MM-dd"))).value<QString>();
    setDate = QDate::fromString(extra_date, _("yyyy-MM-dd"));
    if (!setDate.isValid() || setDate < curDate) {
        extraCandidateDate2->setDate(curDate.addDays(1));
    } else {
        extraCandidateDate2->setDate(setDate);
    }

    connect(extraCandidateDateCB2, &QCheckBox::toggled, this, [=] (bool checked) {
        QSettings setting;
        extraCandidateDate2->setEnabled(checked && candidateCB->isChecked());
        UserData::instance()->candidateSetting.extraCandidateDate2En = checked;
        setting.setValue(_("candidate_setting/extra_date2_en"), checked);
    });
    checked = setting.value(_("candidate_setting/extra_date2_en"), false).value<bool>();
    extraCandidateDateCB2->setChecked(checked);

    //extraCandidateDate2->setEnabled(false);
    connect(candidateCB, &QCheckBox::toggled, this, [=] (bool checked) {
        QSettings setting;
        UserData::instance()->candidateSetting.isCandidate = checked;
        onlyCandidateCB->setEnabled(checked);
        prioCandidateCB->setEnabled(checked);
        noSeatCB->setEnabled(checked);
        acceptNewTrainCB->setEnabled(checked);
        timeEdit2->setEnabled(checked && acceptNewTrainCB->isChecked());
        timeEdit3->setEnabled(checked && acceptNewTrainCB->isChecked());
        extraCandidateDateCB->setEnabled(checked);
        extraCandidateDateCB2->setEnabled(checked);
        extraCandidateDate->setEnabled(checked && extraCandidateDateCB->isChecked());
        extraCandidateDate2->setEnabled(checked && extraCandidateDateCB2->isChecked());
        setting.setValue(_("candidate_setting/candidate_ticket"), checked);
    });

    bool candidateChecked = setting.value(_("candidate_setting/candidate_ticket"), false).value<bool>();
    candidateCB->setChecked(candidateChecked);
    onlyCandidateCB->setEnabled(candidateChecked);
    prioCandidateCB->setEnabled(candidateChecked);
    noSeatCB->setEnabled(candidateChecked);
    acceptNewTrainCB->setEnabled(candidateChecked);
    checked = setting.value(_("candidate_setting/accept_new_train"), false).value<bool>();
    acceptNewTrainCB->setChecked(checked);

    timeEdit2->setEnabled(candidateChecked && checked);
    timeEdit3->setEnabled(candidateChecked && checked);

    extraCandidateDateCB->setEnabled(candidateChecked);
    extraCandidateDateCB2->setEnabled(candidateChecked);

    checked = setting.value(_("candidate_setting/extra_date_en"), false).value<bool>();
    extraCandidateDate->setEnabled(candidateChecked && checked);
    checked = setting.value(_("candidate_setting/extra_date2_en"), false).value<bool>();
    extraCandidateDate2->setEnabled(candidateChecked && checked);

    QVBoxLayout *vlayout2 = new QVBoxLayout;
    vlayout2->addWidget(extraCandidateDateCB);
    vlayout2->addWidget(extraCandidateDate);
    vlayout2->addWidget(extraCandidateDateCB2);
    vlayout2->addWidget(extraCandidateDate2);
    vlayout2->addStretch();

    vlayout->addWidget(candidateCB);
    vlayout->addWidget(onlyCandidateCB);
    vlayout->addWidget(prioCandidateCB);
    vlayout->addWidget(noSeatCB);
    vlayout->addWidget(acceptNewTrainCB);
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(timeEdit2);
    hlayout2->addWidget(label);
    hlayout2->addWidget(timeEdit3);
    hlayout2->addWidget(label2);
    hlayout2->addStretch();
    vlayout->addLayout(hlayout2);
    vlayout->addStretch();
    hlayout->addLayout(vlayout);
    hlayout->addLayout(vlayout2);
    hlayout->addStretch();
    groupBox->setLayout(hlayout);
    hlayout = new QHBoxLayout;
    label = new QLabel(tr("候补截止兑现时间："));
    QComboBox *candidateTimeCB = new QComboBox;
    QStringList candidateEndTimeText = {
        _("开车前20分钟"),
        _("开车前1小时"),
        _("开车前2小时"),
        _("开车前3小时"),
        _("开车前6小时"),
        _("开车前12小时"),
        _("开车前1天"),
        _("开车前2天"),
        _("开车前3天"),
        _("开车前4天"),
        _("开车前5天"),
        _("开车前6天"),
        _("开车前7天")
    };
    candidateTimeCB->addItems(candidateEndTimeText);
    connect(candidateTimeCB, &QComboBox::currentIndexChanged, this, [] (int index) {
        UserData *ud = UserData::instance();
        QList<int> minutesList = {
            20,
            60,
            120,
            180,
            360,
            720,
            1440,
            2880,
            4320,
            5760,
            7200,
            8640,
            10080
        };
        Q_ASSERT(index < minutesList.size());
        ud->candidateSetting.selectedEndCandidateMinutes = minutesList[index];
        QSettings setting;
        setting.setValue(_("candidate_setting/end_candidate_index"), index);
    });
    int index = setting.value(_("candidate_setting/end_candidate_index"), 3).value<int>();
    candidateTimeCB->setCurrentIndex(index);

    hlayout->addWidget(label);
    hlayout->addWidget(candidateTimeCB);
    hlayout->addStretch();
    QVBoxLayout *outLayout = new QVBoxLayout;
    outLayout->addWidget(groupBox);
    outLayout->addLayout(hlayout);

    QWidget *widget = new QWidget;
    widget->setLayout(outLayout);
    tab->addTab(widget, tr("候补设置"));
}

void SettingDialog::sendTestEmail()
{
    UserData *ud = UserData::instance();
    if (!ud->notifySetting.emailNotify.notifyEmailEnable) {
        emailTestMsgLa->setText(_("邮件通知未启用，请先启用"));
        return;
    }
    if (ud->notifySetting.emailNotify.senderEmailAddress.isEmpty()) {
        emailTestMsgLa->setText(_("发件人未填写，请先填写发件人"));
        return;
    }
    if (ud->notifySetting.emailNotify.authCode.isEmpty()) {
        emailTestMsgLa->setText(_("授权码未填写，请先填写授权码"));
        return;
    }
    if (ud->notifySetting.emailNotify.receiverEmailAddress.isEmpty()) {
        emailTestMsgLa->setText(_("收件人未填写，请先填写收件人"));
        return;
    }
    if (ud->notifySetting.emailNotify.emailServer.isEmpty()) {
        emailTestMsgLa->setText(_("smtp域名未填写，请先填写smtp域名"));
        return;
    }
    if (!smtpEmail) {
        smtpEmail = new SimpleSmtpEmail;
        smtpEmail->setHost(ud->notifySetting.emailNotify.emailServer);
        smtpEmail->setPort(ud->notifySetting.emailNotify.emailPort);
        smtpEmail->setSsl(ud->notifySetting.emailNotify.enableSsl);
        connect(smtpEmail, &SmtpEmail::mailStateNotify, this, [this] (enum MAILSTATE state, QString error) {
            if (state == EMAILSUCCESS) {
                emailTestMsgLa->setText(_("邮件发送成功"));
            } else {
                emailTestMsgLa->setText(_("邮件发送失败，错误详情%1").arg(error));
            }
            //smtpEmail->disconnect();
            if (emailTestTimer) {
                emailTestTimer->stop();
                delete emailTestTimer;
                emailTestTimer = nullptr;
            }
            smtpEmail->deleteLater();
            //delete smtpEmail;
            smtpEmail = nullptr;
        });
    }
    if (!emailTestTimer) {
        emailTestTimer = new QTimer;
        emailTestTimer->setInterval(15 * 1000);
        connect(emailTestTimer, &QTimer::timeout, this, [=] () {
            emailTestMsgLa->setText(_("邮件发送失败，请求超时"));
            if (smtpEmail) {
                smtpEmail->disconnect();
            }
            emailTestTimer->stop();
            //delete emailTestTimer;
            emailTestTimer->deleteLater();
            emailTestTimer = nullptr;
            delete smtpEmail;
            smtpEmail = nullptr;
        });
        emailTestTimer->start();
    }
    emailTestMsgLa->setText(_(""));
    std::string msg = _("这是一条来自云映的测试通知消息，如果您收到此消息，说明您已成功配置。"
                    "若您未进行过相关操作可能是别人误填了您的邮箱地址，请忽略本消息").toStdString();
    std::vector<std::string> cclist;
    std::vector<std::string> recevier;
    for (auto &rcv : ud->notifySetting.emailNotify.receiverEmailAddress) {
        recevier.push_back(rcv.toStdString());
    }
    smtpEmail->sendEmail(ud->notifySetting.emailNotify.senderEmailAddress.toStdString(),
                         ud->notifySetting.emailNotify.authCode.toStdString(),
                         recevier,
                         _("云映运行状态通知").toStdString(),
                         msg,
                         cclist,
                         cclist);
}

void SettingDialog::notifySetting(QTabWidget *tab)
{
    QSettings setting;
    QStringList domailName = {
        _("@163.com"),
        _("@126.com"),
        _("@qq.com"),
        _("@outlook.com"),
        _("@sina.com"),
        _("@gmail.com"),
        _("@yahoo.com"),
        _("@139.com"),
        _("@189.cn"),
        _("@wo.cn"),
        _("@88.com")
    };
    emailTestTimer = nullptr;
    smtpEmail = nullptr;
    QCheckBox *notifyEmailCB = new QCheckBox(tr("启用邮件通知"));
    connect(notifyEmailCB, &QCheckBox::toggled, this, [=] (bool checked) {
        UserData::instance()->notifySetting.emailNotify.notifyEmailEnable = checked;
        QSettings setting;
        setting.setValue(_("email_notify/email_notify_en"), checked);
    });
    bool checked = setting.value(_("email_notify/email_notify_en"), false).value<bool>();
    notifyEmailCB->setChecked(checked);
    QVBoxLayout *vlayout = new QVBoxLayout;
    QRadioButton *easyRb = new QRadioButton(tr("简易"));
    QRadioButton *advanceRb = new QRadioButton(tr("高级"));
    QButtonGroup *bg = new QButtonGroup;
    bg->addButton(easyRb);
    bg->addButton(advanceRb);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(easyRb);
    hlayout->addWidget(advanceRb);
    hlayout->addStretch();
    vlayout->addWidget(notifyEmailCB);
    vlayout->addLayout(hlayout);
    hlayout = new QHBoxLayout;
    QLineEdit *senderLe = new QLineEdit;
    QLineEdit *authCodeLe = new QLineEdit;
    QCheckBox *keepAuthCodeCB = new QCheckBox(tr("记住授权码"));
    QLineEdit *recevierLe = new QLineEdit;
    QLineEdit *domainLe = new QLineEdit;
    recevierLe->setPlaceholderText(tr("需要@xxx后缀，多个收件人以，分隔"));
    QComboBox *domainCB = new QComboBox;
    domainCB->addItems(domailName);
    connect(domainCB, &QComboBox::currentTextChanged, this, [domainCB] (QString text) {
        UserData *ud = UserData::instance();
        QSettings setting;
        QStringList emailServer = {
            _("smtp.163.com"),
            _("smtp.126.com"),
            _("smtp.qq.com"),
            _("smtp.outlook.com"),
            _("smtp.sina.com"),
            _("smtp.gmail.com"),
            _("smtp.mail.yahoo.com"),
            _("smtp.139.com"),
            _("smtp.189.cn"),
            _("smtp.wo.cn"),
            _("smtp.88.com")
        };
        QStringList addr = ud->notifySetting.emailNotify.senderEmailAddress.split('@');
        if (!addr.isEmpty()) {
            ud->notifySetting.emailNotify.senderEmailAddress = addr[0] + text;

            setting.setValue(_("email_notify/sender_mail_ddress"),
                             ud->notifySetting.emailNotify.senderEmailAddress);
        }

        if (!ud->notifySetting.emailNotify.advanceMode) {
            int index = domainCB->currentIndex();
            Q_ASSERT(index < emailServer.size());
            ud->notifySetting.emailNotify.emailServer = emailServer[index];
            setting.setValue(_("email_notify/sender_domain"),
                             text);
        }
    });
    connect(senderLe, &QLineEdit::textChanged, this, [=] (QString text) {
        UserData *ud = UserData::instance();
        QString addr = text.trimmed();
        QString domain = domainCB->currentText();
        if (!ud->notifySetting.emailNotify.advanceMode) {
            ud->notifySetting.emailNotify.senderEmailAddress =
                addr + domain;
        } else {
            ud->notifySetting.emailNotify.senderEmailAddress =
                addr;
        }

        QSettings setting;
        setting.setValue(_("email_notify/sender_mail_ddress"),
                         text);
        if (domain != _("@yahoo.com")) {
            text = _("smtp") + domain.replace('@', '.');
        } else {
            text = _("smtp.mail.yahoo.com");
        }

        if (!ud->notifySetting.emailNotify.advanceMode) {
            setting.setValue(_("email_notify/email_server"),
                             text);
            ud->notifySetting.emailNotify.emailServer = text;
            domainLe->setText(text);
        }
    });

    connect(authCodeLe, &QLineEdit::textChanged, this, [] (QString text) {
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.authCode = text.trimmed();
        if (ud->notifySetting.emailNotify.keepAuthCode) {
            QSettings setting;
            setting.setValue(_("email_notify/auth_code"),
                             text);
        }
    });
    QString text = setting.value(_("email_notify/auth_code"),
                                 _("")).value<QString>();
    authCodeLe->setText(text);
    authCodeLe->setPlaceholderText(_("授权码"));
    authCodeLe->setEchoMode(QLineEdit::Password);
    connect(recevierLe, &QLineEdit::textChanged, this, [] (QString text) {
        UserData *ud = UserData::instance();
        QStringList address = text.split(',');
        ud->notifySetting.emailNotify.receiverEmailAddress.clear();
        for (auto &addr : address) {
            ud->notifySetting.emailNotify.receiverEmailAddress
                .push_back(addr.trimmed());
        }
        QSettings setting;
        setting.setValue(_("email_notify/receiver_mail_address"),
                         ud->notifySetting.emailNotify.receiverEmailAddress);
    });
    text = setting.value(_("email_notify/receiver_mail_address"),
                                 _("")).value<QString>();
    recevierLe->setText(text);

    QLabel *label = new QLabel(tr("发件人"));
    hlayout->addWidget(label);
    hlayout->addWidget(senderLe);
    hlayout->addWidget(domainCB);
    vlayout->addLayout(hlayout);
    label = new QLabel(tr("授权码"));
    hlayout = new QHBoxLayout;
    hlayout->addWidget(label);
    hlayout->addWidget(authCodeLe);
    vlayout->addLayout(hlayout);
    hlayout = new QHBoxLayout;
    label = new QLabel(tr("收件人"));
    hlayout->addWidget(label);
    hlayout->addWidget(recevierLe);
    vlayout->addLayout(hlayout);

    emailTestMsgLa = new QLabel;
    emailTestMsgLa->setStyleSheet(_("color: red"));
    vlayout->addWidget(emailTestMsgLa);

    connect(domainLe, &QLineEdit::textChanged, this, [] (QString text) {
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.emailServer = text;
        QSettings setting;
        setting.setValue(_("email_notify/email_server"),
                         text);
    });
    text = setting.value(_("email_notify/email_server"),
                         _("")).value<QString>();
    domainLe->setText(text);
    QLabel *smtpDomailLa = new QLabel(tr("smtp域名"));

    hlayout = new QHBoxLayout;
    hlayout->addWidget(smtpDomailLa);
    hlayout->addWidget(domainLe);
    vlayout->addLayout(hlayout);
    QLabel *smtpPortLa = new QLabel(tr("smtp端口"));
    hlayout = new QHBoxLayout;
    hlayout->addWidget(smtpPortLa);
    QCheckBox *useSsl = new QCheckBox(tr("ssl加密发送"));
    QSpinBox *portSb = new QSpinBox;
    connect(portSb, &QSpinBox::valueChanged, this, [] (int value) {
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.emailPort = value;
        QSettings setting;
        setting.setValue(_("email_notify/email_port"),
                         value);
    });
    int value = setting.value(_("email_notify/email_port"),
                         465).value<int>();
    portSb->setMinimum(1);
    portSb->setMaximum(65535);
    portSb->setValue(value);
    portSb->setMinimumWidth(60);
    portSb->setAlignment(Qt::AlignRight);
    hlayout->addWidget(portSb);
    hlayout->addStretch();
    vlayout->addLayout(hlayout);
    smtpDomailLa->hide();
    domainLe->hide();
    smtpPortLa->hide();
    portSb->hide();
    connect(advanceRb, &QPushButton::toggled, this, [=] (bool checked) {
        if (checked) {
            domainCB->hide();
            smtpDomailLa->show();
            domainLe->show();
            smtpPortLa->show();
            portSb->show();
            senderLe->setPlaceholderText(tr("需要@xxx后缀"));
        } else {
            domainCB->show();
            smtpDomailLa->hide();
            domainLe->hide();
            smtpPortLa->hide();
            portSb->hide();
            senderLe->setPlaceholderText(tr("不需要@xxx后缀"));
            portSb->setValue(useSsl->isChecked() ? 465 : 25);
        }
        QSettings setting;
        setting.setValue(_("email_notify/advance_mode"), checked);
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.advanceMode = checked;
    });
    checked = setting.value(_("email_notify/advance_mode"),
                               false).value<bool>();
    advanceRb->setChecked(checked);
    easyRb->setChecked(!checked);
    senderLe->setPlaceholderText(checked ? tr("需要@xxx后缀") : tr("不需要@xxx后缀"));

    connect(useSsl, &QCheckBox::toggled, this, [advanceRb] (bool checked) {
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.enableSsl = checked;
        QSettings setting;
        setting.setValue(_("email_notify/email_enable_ssl"),
                         checked);
        if (!advanceRb->isChecked()) {
            ud->notifySetting.emailNotify.emailPort = checked ? 465 : 25;
        }
    });
    checked = setting.value(_("email_notify/email_enable_ssl"),
                            true).value<bool>();
    useSsl->setChecked(checked);
    vlayout->addWidget(useSsl);
    connect(keepAuthCodeCB, &QCheckBox::toggled, this, [] (bool checked) {
        UserData *ud = UserData::instance();
        ud->notifySetting.emailNotify.keepAuthCode = checked;
        QSettings setting;
        setting.setValue(_("email_notify/keep_auth_code"),
                         checked);
    });
    checked = setting.value(_("email_notify/keep_auth_code"),
                            true).value<bool>();
    keepAuthCodeCB->setChecked(checked);
    vlayout->addWidget(keepAuthCodeCB);

    text = setting.value(_("email_notify/sender_mail_ddress"),
                                 _("")).value<QString>();
    senderLe->setText(text);
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QPushButton *testMail = new QPushButton(tr("测试发送"));
    connect(testMail, &QPushButton::clicked, this, &SettingDialog::sendTestEmail);
    testMail->setToolTip(tr("您稍后将收到主题为'云映运行状态通知'的邮件"));
    hlayout1->addStretch();
    hlayout1->addWidget(testMail);
    hlayout1->addStretch();
    vlayout->addLayout(hlayout1);
    vlayout->addStretch();
    hlayout = new QHBoxLayout;
    hlayout->addLayout(vlayout);
    hlayout->addStretch();

    QWidget *widget = new QWidget;
    widget->setLayout(hlayout);
    tab->addTab(widget, tr("邮件通知"));
}

void SettingDialog::setUp()
{
    QTabWidget *tabWidget = new QTabWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;

    commonSetting(tabWidget);
    grabTicketSetting(tabWidget);
    candidateSetting(tabWidget);
    notifySetting(tabWidget);

    vLayout->addWidget(tabWidget);
    dialog->setLayout(vLayout);
    //settingDialog->resize(800, 600);
    //dialog->exec();
}
