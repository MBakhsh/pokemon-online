#include "pokeboxes.h"
#include "pokebox.h"
#include "ui_pokeboxes.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/teamholder.h"
#include "theme.h"

PokeBoxes::PokeBoxes(QWidget *parent, TeamHolder *nteam) :
    TeamBuilderWidget(parent), m_team(nteam),
    ui(new Ui::PokeBoxes)
{
    ui->setupUi(this);

    ui->pokemonButtons->setTeam(team().team());
    changePoke(&team().team().poke(0));
    updatePoke();

    loadBoxes();

    connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SLOT(changeTeamPoke(int)));
    connect(ui->storeButton, SIGNAL(clicked()), SLOT(storePokemon()));
    connect(ui->deleteButton, SIGNAL(clicked()), SLOT(deletePokemon()));
    connect(ui->withdrawButton, SIGNAL(clicked()), SLOT(withdrawPokemon()));
    connect(ui->switchButton, SIGNAL(clicked()), SLOT(switchPokemon()));
}

PokeBoxes::~PokeBoxes()
{
    delete ui;
}

void PokeBoxes::showPoke(PokeTeam *poke)
{
    changePoke(poke);
    updatePoke();
}

void PokeBoxes::updateTeam()
{
    ui->pokemonButtons->setTeam(team().team());
    updatePoke();
}

void PokeBoxes::changePoke(PokeTeam *poke)
{
    this->m_poke = poke;
}

void PokeBoxes::updatePoke()
{
    ui->nickNameLabel->setText(poke().nickname());
    ui->speciesLabel->setText(PokemonInfo::Name(poke().num()));
    ui->pokemonSprite->setPixmap(poke().picture());
    ui->pokemonSprite->setFixedSize(poke().picture().size());
    ui->type1Label->setPixmap(Theme::TypePicture(PokemonInfo::Type1(poke().num(), poke().gen())));
    if(PokemonInfo::Type2(poke().num(), poke().gen()) != Type::Curse) {
        ui->type2Label->setVisible(true);
        ui->type2Label->setPixmap(Theme::TypePicture(PokemonInfo::Type2(poke().num(), poke().gen())));
    } else {
        ui->type2Label->setVisible(false);
    }
    ui->nature->setText(NatureInfo::Name(poke().nature()));
    ui->itemSprite->setPixmap(ItemInfo::Icon(poke().item()));
    ui->genderLabel->setPixmap(Theme::GenderPicture(poke().gender()));
    ui->levelLabel->setText(tr("Lv. %1").arg(poke().level()));
    QString movesInfo;
    for(int movesCount = 0; movesCount < 4; movesCount++) {
        if(movesCount == 4) {
            movesInfo.append(QString("%1").arg(MoveInfo::Name(poke().move(movesCount))));
        } else {
            movesInfo.append(QString("%1\n").arg(MoveInfo::Name(poke().move(movesCount))));
        }
    }
    ui->moves->setText(movesInfo);
}

void PokeBoxes::changeTeamPoke(int index)
{
    changePoke(&team().team().poke(index));
    updatePoke();
}

void PokeBoxes::loadBoxes()
{
    QDir directory = QDir(PokeBox::getBoxPath());

    QStringList files = directory.entryList(QStringList() << "*.box", QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

    if (files.size() == 0) {
        /* Tries to get old boxes */
        QApplication::setApplicationName("Pokeymon-Online");
        QDir dir = QDir(PokeBox::getBoxPath());
        QApplication::setApplicationName("Pokemon-Online");

        QStringList f = dir.entryList(QStringList() << "*.box", QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);

        if (f.length() != 0) {
            foreach(QString fs, f) {
                QFile x(fs);
                x.copy(directory.absoluteFilePath(fs));
            }
            files = f;
        } else {
            files << tr("Box A.box") << tr("Box B.box") << tr("Box C.box") << tr("Box D.box") << tr("Box E.box") <<
                     tr("Box F.box") << tr("Box G.box") << tr("Box H.box");
        }
    }

    foreach (QString file, files) {
        addBox(file);
    }
}

void PokeBoxes::switchBoxTeam(int box, int boxslot, int teamslot)
{
    ui->boxes->setCurrentIndex(box);
    currentBox()->changeCurrentSpot(boxslot);

    ui->pokemonButtons->setCurrentSlot(teamslot);

    try {
        currentBox()->getCurrent();
        if (team().team().poke(teamslot).num() == 0)
            withdrawPokemon();
        else
            switchPokemon();
    } catch (const QString&) {
        try {
            storePokemon();
        } catch (const QString&) {

        }
    }
}

void PokeBoxes::addBox(const QString &name)
{
    PokeBox *box = new PokeBox(boxes.size(), name);
    boxes.push_back(box);
    box->setParent(this);
    ui->boxes->addTab(box, box->getBoxName());
    connect(box, SIGNAL(switchWithTeam(int,int,int)), SLOT(switchBoxTeam(int,int,int)));
    connect(box, SIGNAL(show(PokeTeam*)), SLOT(showPoke(PokeTeam*)));
}

void PokeBoxes::storePokemon()
{
    try {
        currentBox()->addPokemonToBox(team().team().poke(currentPoke()));
        currentBox()->saveBox();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::withdrawPokemon()
{
    try {
        setCurrentTeamPoke(currentBox()->getCurrent());
        updateSpot(currentPoke());
        emit teamChanged();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::switchPokemon()
{
    try {
        PokeTeam *p = new PokeTeam(*currentBox()->getCurrent());
        currentBox()->changeCurrent(*currentPokeTeam());
        currentBox()->saveBox();
        setCurrentTeamPoke(p);

        /* Don't worry, if getCurrent doesn't throw exceptions then changeCurrent doesn't.
           Hence no memory leaks */
        delete p;

        updateSpot(currentPoke());
        teamChanged();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

void PokeBoxes::deletePokemon()
{
    try {
        currentBox()->deleteCurrent();
        currentBox()->saveBox();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box %1 - %2").arg(currentBox()->getNum()).arg(currentBox()->getBoxName()), ex);
    }
}

int PokeBoxes::currentPoke() const
{
    return std::max(ui->pokemonButtons->currentSlot(), 0);
}

PokeBox *PokeBoxes::currentBox()
{
    return boxes[ui->boxes->currentIndex()];
}

void PokeBoxes::updateSpot(int i)
{
    ui->pokemonButtons->updatePoke(i);
}

void PokeBoxes::setCurrentTeamPoke(PokeTeam *p)
{
    *currentPokeTeam() = *p;

    if (p->gen() != team().team().gen()) {
        p->setGen(team().team().gen());
        p->runCheck();
    }
}

PokeTeam *PokeBoxes::currentPokeTeam()
{
    return &team().team().poke(currentPoke());
}
