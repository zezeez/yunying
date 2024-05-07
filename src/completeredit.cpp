#include "completeedit.h"
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>
#include "userdata.h"

enum stationInfoEnum {
    ETHREEPINYINCODE = 0,
    ESTANAME,
    ESTACODE,
    EFULLSTAPINYIN,
    ESIMPLESTAPINYIN,
    ESTACURRENTCOUNT,
};

#define _ QStringLiteral

#define STATIONL1COUNT 256
#define STATIONL2COUNT 256
#define STATIONSIZE (STATIONL1COUNT * STATIONL2COUNT)
#define STATIONL1POS(x) ((x) * STATIONL1COUNT)
#define STATIONL2POS(y) (y)
#define STATIONPOS(x, y) (STATIONL1POS(x) + \
                          STATIONL2POS(y))

InputCompleter::InputCompleter(QObject *parent) :
    QCompleter(parent), m_model()
{
    setModel(&m_model);

    stationNameIndexLevel1.fill(-1, STATIONL1COUNT);
    stationNameIndexLevel2.fill(-1, STATIONL2COUNT);
    stationNameData.resize(STATIONSIZE);
    stationFullPinYinIndexLevel1.fill(-1, STATIONL1COUNT);
    stationFullPinYinIndexLevel2.fill(-1, STATIONL2COUNT);
    stationFullPinYinData.resize(STATIONSIZE);
    stationSimplePinYinIndexLevel1.fill(-1, STATIONL1COUNT);
    stationSimplePinYinIndexLevel2.fill(-1, STATIONL2COUNT);
    stationSimplePinYinData.resize(STATIONSIZE);

    isAppend = false;
}

InputCompleter &InputCompleter::operator=(const InputCompleter &other)
{
    int i;
    for (i = 0; i < other.stationNameIndexLevel1.size(); i++) {
        stationNameIndexLevel1[i] = other.stationNameIndexLevel1[i];
    }
    for (i = 0; i < other.stationNameIndexLevel2.size(); i++) {
        stationNameIndexLevel2[i] = other.stationNameIndexLevel2[i];
    }
    for (i = 0; i < other.stationNameData.size(); i++) {
        stationNameData[i] = other.stationNameData[i];
    }

    for (i = 0; i < other.stationFullPinYinIndexLevel1.size(); i++) {
        stationFullPinYinIndexLevel1[i] = other.stationFullPinYinIndexLevel1[i];
    }
    for (i = 0; i < other.stationFullPinYinIndexLevel2.size(); i++) {
        stationFullPinYinIndexLevel2[i] = other.stationFullPinYinIndexLevel2[i];
    }
    for (i = 0; i < other.stationFullPinYinData.size(); i++) {
        stationFullPinYinData[i] = other.stationFullPinYinData[i];
    }

    for (i = 0; i < other.stationSimplePinYinIndexLevel1.size(); i++) {
        stationSimplePinYinIndexLevel1[i] = other.stationSimplePinYinIndexLevel1[i];
    }
    for (i = 0; i < other.stationSimplePinYinIndexLevel2.size(); i++) {
        stationSimplePinYinIndexLevel2[i] = other.stationSimplePinYinIndexLevel2[i];
    }
    for (i = 0; i < other.stationSimplePinYinData.size(); i++) {
        stationSimplePinYinData[i] = other.stationSimplePinYinData[i];
    }

    keyWordStack = other.keyWordStack;
    m_word = other.m_word;
    isAppend = other.isAppend;

    return *this;
}

void InputCompleter::setStationData(const QByteArray &nameText)
{
    int i, j;
    int nameTextSize = nameText.size();
    int sectionIndex = 0;
    QByteArray staName, staCode, staSimplePinYin, staFullPinYin;
    UserData *ud = UserData::instance();

    i = 0;
    while (i < nameTextSize && nameText[i] != '@') {
        ++i;
    }
    if (i == nameTextSize)
        return;
    // skip '@' char
    ++i;
    for (; i < nameTextSize; i = j) {
        for (j = i + 1; j < nameTextSize && nameText[j] != '|' && nameText[j] != '@'; j++) {

        }
        if (j < nameTextSize) {
            if (j > i + 1) {
                switch (sectionIndex) {
                case ESTANAME:
                    staName = nameText.sliced(i + 1, j - (i + 1));
                    break;
                case ESTACODE:
                    staCode = nameText.sliced(i + 1, j - (i + 1));
                    break;
                case EFULLSTAPINYIN:
                    staFullPinYin = nameText.sliced(i + 1, j - (i + 1));
                    break;
                case ESIMPLESTAPINYIN:
                    staSimplePinYin = nameText.sliced(i + 1, j - (i + 1));
                    break;
                default:
                    break;
                }
            }
            sectionIndex++;
            if (nameText[j] == '@') {
                if (!staName.isEmpty() && !staCode.isEmpty() &&
                    !staFullPinYin.isEmpty() && !staSimplePinYin.isEmpty() &&
                    !staCode.isEmpty()) {
                    addStationName(staName, staFullPinYin);
                    addStationFullPinYin(staName, staFullPinYin);
                    addStationSimplePinYin(staName, staSimplePinYin, staFullPinYin);
                    ud->setStationCode(staName, staCode);
                }
                sectionIndex = 0;
                staName.clear();
                staFullPinYin.clear();
                staSimplePinYin.clear();
            }
        }
    }
    if (!staName.isEmpty() && !staCode.isEmpty() &&
        !staFullPinYin.isEmpty() && !staSimplePinYin.isEmpty()) {
        addStationName(staName, staFullPinYin);
        addStationFullPinYin(staName, staFullPinYin);
        addStationSimplePinYin(staName, staSimplePinYin, staFullPinYin);
        ud->setStationCode(staName, staCode);
    }
}

void InputCompleter::addStationName(const QByteArray &staName, const QByteArray &staFullPinYin)
{
    int idx1 = 0, idx2 = 0;
    if (staName.length() > 1) {
        idx2 = static_cast<unsigned char>(staName[1]);
    }
    idx1 = static_cast<unsigned char>(staName[0]);
    stationNameIndexLevel1[idx1] = idx1;
    stationNameIndexLevel2[idx2] = idx2;
    stationNameData[STATIONPOS(idx1, idx2)].append(
        QPair<QByteArray, QString>(staName, staName + _(" ") + staFullPinYin));
}

void InputCompleter::addStationFullPinYin(const QByteArray &staName, const QByteArray &staFullPinYin)
{
    int idx1 = 0, idx2 = 0;
    if (staFullPinYin.length() > 1) {
        idx2 = static_cast<unsigned char>(staFullPinYin[1]);
    }
    idx1 = static_cast<unsigned char>(staFullPinYin[0]);
    stationFullPinYinIndexLevel1[idx1] = idx1;
    stationFullPinYinIndexLevel2[idx2] = idx2;
    if (idx1 == 103 && idx2 == 122) {
        qDebug() << staName << staFullPinYin;
    }
    stationFullPinYinData[STATIONPOS(idx1, idx2)].append(
        QPair<QByteArray, QString>(staFullPinYin, staName + _(" ") + staFullPinYin));
}

void InputCompleter::addStationSimplePinYin(const QByteArray &staName,
                                            const QByteArray &staSimplePinYin, const QByteArray &staFullPinYin)
{
    int idx1 = 0, idx2 = 0;
    if (staSimplePinYin.length() > 1) {
        idx2 = static_cast<unsigned char>(staSimplePinYin[1]);
    }
    idx1 = static_cast<unsigned char>(staSimplePinYin[0]);
    stationSimplePinYinIndexLevel1[idx1] = idx1;
    stationSimplePinYinIndexLevel2[idx2] = idx2;
    stationSimplePinYinData[STATIONPOS(idx1, idx2)].append(
        QPair<QByteArray, QString>(staSimplePinYin, staName + _(" ") + staFullPinYin));
}

void InputCompleter::update(const QByteArray &word)
{
    // Do any filtering you like.
    QVector<QPair<QByteArray, QString>>::const_iterator it;
    QVector<QPair<QByteArray, QString>> previous, current;
    QStringList result;

    int count = word.length() - m_word.length();
    if (word.length() > 2) {
            if (count > 0) {
                isAppend = true;
                if (!keyWordStack.empty()) {
                    previous = keyWordStack.top();
                    for (it = previous.cbegin(); it != previous.cend(); it++) {
                        if (it->first.startsWith(word)) {
                            result.append(it->second);
                            current.append(*it);
                        }
                    }
                    keyWordStack.push(current);
                }
            } else {
                if (isAppend)
                    count--;
                isAppend = false;
                while (count++ < 0) {
                    if (!keyWordStack.isEmpty())
                        current = keyWordStack.pop();
                }
                for (it = current.cbegin(); it != current.cend(); it++) {
                    result.append(it->second);
                }
            }
    } else {
        if (!word.isEmpty()) {
            if (count > 0) {
                int idx11 = 0, idx21 = 0, idx31 = 0;
                unsigned char w0, w1;
                isAppend = true;
                if (word.size() > 0) {
                    w0 = word[0];
                    idx11 = stationSimplePinYinIndexLevel1[w0];
                    idx21 = stationFullPinYinIndexLevel1[w0];
                    idx31 = stationNameIndexLevel1[w0];
                }
                int idx12 = -1, idx22 = -1, idx32 = -1;
                if (word.size() > 1) {
                    w1 = word[1];
                    idx12 = stationSimplePinYinIndexLevel2[w1];
                    idx22 = stationFullPinYinIndexLevel2[w1];
                    idx32 = stationNameIndexLevel2[w1];
                }
                int idx;
                const QVector<QVector<QVector<QPair<QByteArray, QString>>>> &v = {
                    stationSimplePinYinData,
                    stationFullPinYinData,
                    stationNameData
                };
                QVector<int> idx1 = { idx11, idx21, idx31 };
                QVector<int> idx2 = { idx12, idx22, idx32 };
                int basePos = 0;
                for (int i = 0; i < v.size(); i++) {
                    if (idx1[i] != -1) {
                        if (idx2[i] == -1) {
                            basePos = STATIONL1POS(idx1[i]);
                            for (idx = 0; idx < 256; idx++) {
                                if (!v[i][basePos +
                                    STATIONL2POS(idx)].isEmpty()) {
                                    keyWordStack.push(v[i][basePos +
                                                           STATIONL2POS(idx)]);
                                    for (auto &d : v[i][basePos +
                                                        STATIONL2POS(idx)]) {
                                        result.append(d.second);
                                    }
                                }
                            }
                        } else {
                            if (!v[i][STATIONPOS(idx1[i], idx2[i])].isEmpty()) {
                                keyWordStack.push(v[i][STATIONPOS(idx1[i], idx2[i])]);
                                 for (auto &d : v[i][STATIONPOS(idx1[i], idx2[i])]) {
                                    result.append(d.second);
                                }
                            }
                        }
                    }
                }
            } else {
                if (isAppend)
                    count--;
                isAppend = false;
                while (count++ < 0) {
                    if (!keyWordStack.isEmpty())
                        current = keyWordStack.pop();
                }
                for (it = current.cbegin(); it != current.cend(); it++) {
                    result.append(it->second);
                }
            }
        }
    }

    if (!result.isEmpty()) {
        m_model.setStringList(result);
    }
    m_word = word;
    complete();
}

CompleteEdit::CompleteEdit(QWidget *parent)
    : QLineEdit(parent), c(nullptr)
{
}

CompleteEdit::~CompleteEdit()
{
}

void CompleteEdit::setCompleter(InputCompleter *completer)
{
    if (c) {
        QObject::disconnect(c, nullptr, this, nullptr);
        delete c;
    }

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    connect(completer, SIGNAL(activated(const QString &)), this, SLOT(insertCompletion(const QString &)));
}

InputCompleter *CompleteEdit::completer() const
{
    return c;
}

void CompleteEdit::insertCompletion(const QString &completion)
{
    setText(completion.split(' ').at(0));
    selectAll();
}

void CompleteEdit::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // Let the completer do default behavior
        }
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!c)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        c->popup()->hide();
        return;
    }

    c->update(text().toUtf8());
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

VarCodeLabel::VarCodeLabel(QWidget *parent) :
    QLabel(parent), area(9)
{
    QVector<mapArea>::iterator it;
    for (it = area.begin(); it != area.end(); ++it) {
        (*it).active = false;
        (*it).selected = false;
    }

    for (int i = 0; i < 9; i++) {
        picLabel[i].setParent(this);
        picLabel[i].hide();
        picLabel[i].setFixedSize(64, 64);
        picLabel[i].setPixmap(QPixmap(QStringLiteral(":/icon/images/selected.svg")));
    }
}

void VarCodeLabel::mouseMoveEvent(QMouseEvent *ev)
{
    QLabel::mouseMoveEvent(ev);
}

void VarCodeLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        QPoint point;
        //point.setX(ev->x() + 10);
        //point.setY(ev->y() - 35);
        //point = ev->pos();
        point.setX(ev->position().x() + 20);
        point.setY(ev->position().y() - 35);
        int ar = getArea(ev->position().x(), ev->position().y());
        if (ar > 0 && ar <= 8) {
            struct mapArea &ma = area[ar];
            ma.pos = point;
            if (!picLabel[ar].isVisible()) {
                picLabel[ar].move(ev->position().x() - 30, ev->position().y() - 45);
                picLabel[ar].show();
            } else {
                picLabel[ar].hide();
            }
            ma.selected = !ma.selected;
        }
        qDebug() << "(x, y) = " << "(" << point.rx() << ", " << point.ry() << ")" << Qt::endl;
    }
}

void VarCodeLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
}

void VarCodeLabel::paintEvent(QPaintEvent *ev)
{
    QLabel::paintEvent(ev);
}

int VarCodeLabel::getArea(int x, int y)
{
    int w = width();
    int h = height();
    int xstep = w / 4;
    int ystep = (h - 45) / 2;
    int i, j;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            int xmax = xstep * (j + 1);
            int xmin = xstep * j;
            int ymax = ystep * (i + 1);
            int ymin = ystep * i;
            if (x >= xmin && x < xmax && y >= ymin + 45 && y < ymax + 42) {
                return i * 4 + j + 1;
            }
        }
    }
    return 0;
}

void VarCodeLabel::clearSelected()
{
    for (int i = 0; i < area.size(); i++) {
        area[i].selected = false;
    }
    for (int i = 0; i < 9; i++) {
        picLabel[i].hide();
    }
}

QVector<mapArea> &VarCodeLabel::getPoints()
{
    return area;
}

VarCodeLabel::~VarCodeLabel()
{

}


ClickLabel::ClickLabel(QWidget *parent) :
    QLabel(parent)
{

}

ClickLabel::~ClickLabel()
{

}

void ClickLabel::mousePressEvent(QMouseEvent *ev)
{
    QLabel::mousePressEvent(ev);
    emit clicked(ev->position().x(), ev->position().y());
}

CheckedLabel::CheckedLabel(QWidget *parent) : ClickLabel(parent)
{
    checkedIndicator.setParent(this);
    //checkedIndicator.setFixedSize(64, 64);
    checkedIndicator.setPixmap(QPixmap(QStringLiteral(":/icon/images/selected.svg")));
}

CheckedLabel::~CheckedLabel()
{

}

void CheckedLabel::mousePressEvent(QMouseEvent *ev)
{
    checkedIndicator.show();
    ClickLabel::mousePressEvent(ev);
}

void CheckedLabel::setChecked(bool checked)
{
    if (checked) {
        checkedIndicator.show();
    } else {
        checkedIndicator.hide();
    }
}

void CheckedLabel::reSetCheckedPos()
{
    checkedIndicator.move(35, 0);
}

MultiAreaLabel::MultiAreaLabel(QWidget *parent, int areaCount, int areaWidth, int areaHeight) :
    QWidget(parent)
{
    count = areaCount;
    width = areaWidth;
    height = areaHeight;
    areaList.resize(areaCount);
    int row = 0, col = 0;
    for (int i = 0; i < areaCount; i++) {
        areaList[i] = new CheckedLabel;
        areaList[i]->setChecked(false);
        connect(areaList[i], &CheckedLabel::clicked, this, &MultiAreaLabel::areaClicked);
        gridlayout.addWidget(areaList[i], row, col);
        col++;
        if (col >= 4) {
            row++;
            col = 0;
        }
    }
    areaList[0]->setChecked(true);
    areaSel = 0;
}

MultiAreaLabel::~MultiAreaLabel()
{
    for (int i = 0; i < count; i++) {
        delete areaList[i];
    }
}

void MultiAreaLabel::setAreaCount(int c)
{
    count = c;
}

int MultiAreaLabel::areaCount() const
{
    return count;
}

void MultiAreaLabel::setAreaWidth(int w)
{
    width = w;
}

int MultiAreaLabel::areaWidth() const
{
    return width;
}

void MultiAreaLabel::setAreaHeight(int h)
{
    height = h;
}

int MultiAreaLabel::areaHeight() const
{
    return height;
}

void MultiAreaLabel::setAreaPixmap(int area, const QString &file)
{
    if (area >= count) {
        return;
    }

    areaList[area]->setPixmap(QPixmap(file));
    areaList[area]->reSetCheckedPos();
}

void MultiAreaLabel::setAreasPixmap(const QStringList &files)
{
    int n = qMin(count, files.size());
    for (int i = 0; i < n; i++) {
        setAreaPixmap(i, files[i]);
    }
}

void MultiAreaLabel::show()
{
    for (int i = 0; i < count; i++) {
        areaList[i]->show();
    }
}

void MultiAreaLabel::hide()
{
    for (int i = 0; i < count; i++) {
        areaList[i]->hide();
    }
}

void MultiAreaLabel::areaClicked()
{
    CheckedLabel *l = dynamic_cast<CheckedLabel *>(sender());
    for (int i = 0; i < count; i++) {
        if (areaList[i] != l) {
            areaList[i]->setChecked(false);
        } else {
            areaSel = i;
        }
    }
    emit clicked(areaSel);
}

QGridLayout *MultiAreaLabel::layout()
{
    return &gridlayout;
}

void MultiAreaLabel::setAreaSelected(int area)
{
    if (area < count) {
        for (int i = 0; i < count; i++) {
            areaList[i]->setChecked(false);
        }
        areaList[area]->setChecked(true);
    }
}

int MultiAreaLabel::areaSelected() const
{
    return areaSel;
}
