#include <QLabel>
#include <BattleManager/battledataaccessor.h>
#include <TeambuilderLibrary/theme.h>
#include "battlewindowattackbutton.h"

void AbstractAttackButton::init()
{
    QVBoxLayout *l = new QVBoxLayout(pointer());

    l->addWidget(name = new QLabel(), 0, Qt::AlignCenter);
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight | Qt::AlignVCenter);
    name->setObjectName("AttackName");
    pp->setObjectName("AttackPP");
}


QString AbstractAttackButton::power() const
{
    const auto &p = *this->p;

    if (num() == Move::Return) {
        return QString("%1").arg(std::max((p.happiness() * 2 / 5),1));
    } else if (num() == Move::Frustration) {
        return  QString("%1").arg(std::max(( (255-p.happiness()) * 2 / 5),1));
    } else if (num() == Move::HiddenPower && gen <= 5) {
        return QString("%1").arg(HiddenPowerInfo::Power(gen, p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]));
    } else {
        return (zmove && validZmove) ? MoveInfo::PowerToString(MoveInfo::ZPower(num(), gen)) : MoveInfo::PowerS(num(), gen);
    }
}

int AbstractAttackButton::num() const
{
    if (!zmove || !validZmove || MoveInfo::Category(b->num(), gen) == Move::Other) {
        return b->num();
    }
    return ItemInfo::ZCrystalMove(p->item());
}

int AbstractAttackButton::type() const
{
    return MoveInfo::Type(b->num(), gen, *p);
}

QString AbstractAttackButton::moveName() const
{
    return MoveInfo::ZName(num(), zmove && validZmove);
}

void AbstractAttackButton::updateAttack(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen, bool zmove)
{
    this->b = &b;
    this->p = &p;
    this->gen = gen;
    this->zmove = zmove;
    validZmove = false;

    if (zmove) {
        int ztype = ItemInfo::ZCrystalType(p.item());

        if (ztype != MoveInfo::Type(b.num(), gen, p)) {
            pointer()->setDisabled(true);
            return;
        } else {
            validZmove = true;
        }
    }

    updateGui();
}

void AbstractAttackButton::updateGui()
{
    name->setText(moveName());
    pp->setText(QObject::tr("PP %1/%2").arg(b->PP()).arg(b->totalPP()));
    pointer()->setToolTip(tooltip());
    pointer()->setAccessibleName(moveName());
}

QString AbstractAttackButton::tooltip() const
{
    int num = this->num();

    QString moveCategory = CategoryInfo::Name(MoveInfo::Category(num, gen));
    return QObject::tr("%1\n\nPower: %2\nAccuracy: %3\nCategory: %4\nRange: %6\n\nDescription: %5").arg(moveName(), power(),
                                                                                                MoveInfo::AccS(num, gen), moveCategory,
                                                                                                MoveInfo::Description(num, gen), MoveInfo::TargetS(num, gen));
}

OldAttackButton::OldAttackButton(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)
{
    init();

    updateAttack(b,p,gen,false);
}

void OldAttackButton::updateGui()
{
    AbstractAttackButton::updateGui();

    pointer()->setStyleSheet(QString("background: %1;").arg(Theme::TypeColor(type()).name()));
}

ImageAttackButton::ImageAttackButton(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)
    : QImageButton(Theme::path("BattleWindow/Buttons/0D.png"), Theme::path("BattleWindow/Buttons/0H.png"))
{
    init();

    updateAttack(b,p,gen,false);
}

void ImageAttackButton::updateGui()
{
    AbstractAttackButton::updateGui();

    QString model = QString("BattleWindow/Buttons/%1%2.png").arg(type());
    changePics(Theme::path(model.arg("D")), Theme::path(model.arg("H")), Theme::path(model.arg("C")));
}
